#pragma once

#include <chrono>
#include <set>
#include <stdexcept>
#include <vector>

#include "../autodiff/laplace_graph_plan.hpp"
#include "../laplace.hpp"
#include "random_effect_objective.hpp"
#include "sparse_factorization_cache.hpp"
#include "sparse_huu_factorization.hpp"

namespace quadra {

struct RandomEffectHessianResult {
  double objective_value_m = 0.0;
  double gradient_norm_m = 0.0;

  std::vector<double> fixed_m;
  std::vector<double> random_m;
  std::vector<double> full_m;
  std::vector<double> gradient_random_m;
  std::vector<double> gradient_fixed_m;
  Eigen::MatrixXd mixed_hessian_m;

  Eigen::SparseMatrix<double> hessian_random_m;

  std::vector<ReportValue> reports_m;
};

struct FixedGradientMixedHessianResult {
  Eigen::VectorXd fixed_gradient_m;
  Eigen::MatrixXd mixed_hessian_m;
  double replay_ms = 0.0;
  double propagate_ms = 0.0;
  double extract_ms = 0.0;
};

inline std::vector<int>
random_indices_as_ints(const ParameterPartition &partition) {
  std::vector<int> out;
  out.reserve(partition.random_indices_m.size());

  for (size_t idx : partition.random_indices_m) {
    out.push_back(static_cast<int>(idx));
  }

  return out;
}

template <class Model> class RandomEffectHessianWorkspace {
public:
  RandomEffectHessianWorkspace(Model &model, const std::vector<double> &fixed,
                               const std::vector<double> &random,
                               const ParameterPartition &partition,
                               bool collect_reports = true)
      : model_(model), partition_(partition),
        collect_reports_(collect_reports) {
    const auto parameter_start = std::chrono::steady_clock::now();
    had::g_ADGraph = &tape_.graph;
    ModelReportContext ctx;
    model_.initialize(ctx);
    fixed_ad_ = to_ad(fixed);
    random_ad_ = to_ad(random);
    full_ad_ = merge_parameters(fixed_ad_, random_ad_, partition_);
    const auto parameter_end = std::chrono::steady_clock::now();
    const auto record_start = parameter_end;
    objective_ = model_.template evaluate<AD>(full_ad_, ctx);
    const auto record_end = std::chrono::steady_clock::now();
    const auto plan_start = record_end;
    std::vector<had::VertexId> fixed_vertices, random_vertices;
    for (const auto &parameter : fixed_ad_)
      fixed_vertices.push_back(parameter.varId);
    for (const auto &parameter : random_ad_)
      random_vertices.push_back(parameter.varId);
    graph_plan_.Build(tape_.graph, fixed_vertices, random_vertices,
                      objective_.varId);
    had::VertexId parameter_vertex_count = 0;
    for (had::VertexId id : fixed_vertices)
      parameter_vertex_count = std::max(parameter_vertex_count, id + 1);
    for (had::VertexId id : random_vertices)
      parameter_vertex_count = std::max(parameter_vertex_count, id + 1);
    fixed_vertex_to_index_.assign(parameter_vertex_count, -1);
    random_vertex_to_index_.assign(parameter_vertex_count, -1);
    for (size_t i = 0; i < fixed_ad_.size(); ++i)
      fixed_vertex_to_index_[fixed_ad_[i].varId] = static_cast<int>(i);
    for (size_t i = 0; i < random_ad_.size(); ++i)
      random_vertex_to_index_[random_ad_[i].varId] = static_cast<int>(i);
    random_idx_ = random_indices_as_ints(partition_);
    const auto plan_end = std::chrono::steady_clock::now();
    construction_parameter_ms_ = std::chrono::duration<double, std::milli>(
                                     parameter_end - parameter_start)
                                     .count();
    construction_record_ms_ =
        std::chrono::duration<double, std::milli>(record_end - record_start)
            .count();
    construction_plan_ms_ =
        std::chrono::duration<double, std::milli>(plan_end - plan_start)
            .count();
  }

  double construction_parameter_ms() const {
    return construction_parameter_ms_;
  }

  double construction_record_ms() const { return construction_record_ms_; }

  double construction_plan_ms() const { return construction_plan_ms_; }

  RandomEffectHessianResult Evaluate(const std::vector<double> &fixed,
                                     const std::vector<double> &random,
                                     double drop_tol = 0.0) {
    if (fixed.size() != fixed_ad_.size() || random.size() != random_ad_.size())
      throw std::invalid_argument(
          "RandomEffectHessianWorkspace parameter length mismatch");
    had::g_ADGraph = &tape_.graph;
    for (size_t i = 0; i < fixed.size(); ++i)
      set_value(fixed_ad_[i], fixed[i]);
    for (size_t i = 0; i < random.size(); ++i)
      set_value(random_ad_[i], random[i]);
    tape_.graph.Forward(graph_plan_.laplace_forward_order());
    const auto objective_vertex = graph_plan_.objective();
    if (!std::isfinite(tape_.graph.vertices[objective_vertex].primal)) {
      had::VertexId first = objective_vertex;
      for (auto id : graph_plan_.laplace_reverse_order())
        if (!std::isfinite(tape_.graph.vertices[id].primal) && id < first)
          first = id;
      const auto &vertex = tape_.graph.vertices[first];
      throw std::runtime_error(
          "RandomEffectHessianWorkspace replay produced a non-finite "
          "objective ancestor at vertex " + std::to_string(first) +
          " (opcode " + std::to_string(static_cast<int>(vertex.op)) +
          ", left " + std::to_string(vertex.left) + ", right " +
          std::to_string(vertex.right) + ")");
    }
    last_hessian_push_count_ = 0;
    PropagateRandomHessianRestricted(tape_.graph, graph_plan_,
                                     &last_hessian_push_count_);
    Eigen::VectorXd g = extract_gradient(random_ad_);
    if (pattern_.empty()) {
      auto &cache = laplace_pattern_cache();
      const std::size_t key =
          laplace_pattern_cache_key(tape_.graph, full_ad_, random_idx_);
      auto found = cache.find(key);
      if (found != cache.end()) {
        pattern_ = found->second;
      } else {
        pattern_ = discover_pattern_from_graph(full_ad_, random_idx_);
        // Numeric sparsity discovery can miss structural entries at special
        // parameter values (for example, AR1 cross-time blocks at rho = 0).
        // Replay once at a small deterministic perturbation and retain the
        // union. This stays model-independent and does not change the values
        // returned for the caller's evaluation point. A completely dense
        // pattern cannot acquire additional entries, so avoid the redundant
        // replay and Hessian sweep in that case.
        const std::size_t dense_size = random.size() * random.size();
        if (pattern_.size() < dense_size) {
          for (size_t i = 0; i < fixed.size(); ++i)
            set_value(fixed_ad_[i],
                      fixed[i] + 0.01 * static_cast<double>(1 + i % 7));
          for (size_t i = 0; i < random.size(); ++i)
            set_value(random_ad_[i],
                      random[i] + 0.001 * static_cast<double>(1 + i % 5));
          tape_.graph.Forward(graph_plan_.laplace_forward_order());
          PropagateRandomHessianRestricted(tape_.graph, graph_plan_);
          const SparseHessianPattern probe = discover_pattern_from_graph(
              full_ad_, random_idx_, true, true, 1e-12, false);
          std::set<std::pair<int, int>> union_pattern(pattern_.begin(),
                                                      pattern_.end());
          union_pattern.insert(probe.begin(), probe.end());
          pattern_.assign(union_pattern.begin(), union_pattern.end());
          for (size_t i = 0; i < fixed.size(); ++i)
            set_value(fixed_ad_[i], fixed[i]);
          for (size_t i = 0; i < random.size(); ++i)
            set_value(random_ad_[i], random[i]);
          tape_.graph.Forward(graph_plan_.laplace_forward_order());
          PropagateRandomHessianRestricted(tape_.graph, graph_plan_);
        }
        cache.emplace(key, pattern_);
      }
    }

    Eigen::SparseMatrix<double> H(static_cast<int>(random.size()),
                                  static_cast<int>(random.size()));
    if (drop_tol == 0.0) {
      if (hessian_pattern_template_.rows() == 0) {
        std::vector<Eigen::Triplet<double>> entries;
        entries.reserve(pattern_.size());
        for (const auto &ij : pattern_)
          entries.emplace_back(ij.first, ij.second, 0.0);
        hessian_pattern_template_.resize(static_cast<int>(random.size()),
                                         static_cast<int>(random.size()));
        hessian_pattern_template_.setFromTriplets(entries.begin(),
                                                  entries.end());
        hessian_pattern_template_.makeCompressed();
      }
      H = hessian_pattern_template_;
      for (int outer = 0; outer < H.outerSize(); ++outer)
        for (Eigen::SparseMatrix<double>::InnerIterator it(H, outer); it; ++it)
          it.valueRef() = hessian_value(
              full_ad_[random_idx_[static_cast<size_t>(it.row())]],
              full_ad_[random_idx_[static_cast<size_t>(it.col())]]);
    } else {
      std::vector<Eigen::Triplet<double>> entries;
      entries.reserve(pattern_.size());
      for (const auto &ij : pattern_) {
        const double value = hessian_value(
            full_ad_[random_idx_[static_cast<size_t>(ij.first)]],
            full_ad_[random_idx_[static_cast<size_t>(ij.second)]]);
        if (std::abs(value) > drop_tol)
          entries.emplace_back(ij.first, ij.second, value);
      }
      H.setFromTriplets(entries.begin(), entries.end());
    }

    RandomEffectHessianResult result;
    result.objective_value_m = value_of(objective_);
    result.fixed_m = fixed;
    result.random_m = random;
    result.full_m = merge_parameters(fixed, random, partition_);
    if (collect_reports_) {
      ModelReportContext report_ctx;
      model_.initialize(report_ctx);
      (void)evaluate_fixed_random<Model, double>(model_, fixed, random,
                                                 partition_, report_ctx);
      result.reports_m = report_ctx.reports().values();
    }
    result.hessian_random_m = std::move(H);
    result.gradient_random_m.resize(static_cast<size_t>(g.size()));
    for (int i = 0; i < g.size(); ++i)
      result.gradient_random_m[static_cast<size_t>(i)] = g[i];
    result.gradient_norm_m =
        random_effect_gradient_norm(result.gradient_random_m);
    return result;
  }

  FixedGradientMixedHessianResult
  EvaluateFixedGradientMixedHessian(const std::vector<double> &fixed,
                                    const std::vector<double> &random) {
    if (fixed.size() != fixed_ad_.size() || random.size() != random_ad_.size())
      throw std::invalid_argument(
          "RandomEffectHessianWorkspace parameter length mismatch");
    had::g_ADGraph = &tape_.graph;
    for (size_t i = 0; i < fixed.size(); ++i)
      set_value(fixed_ad_[i], fixed[i]);
    for (size_t i = 0; i < random.size(); ++i)
      set_value(random_ad_[i], random[i]);
    const auto replay_start = std::chrono::steady_clock::now();
    tape_.graph.Forward(graph_plan_.laplace_forward_order());
    const auto replay_end = std::chrono::steady_clock::now();
    PropagateMixedHessianRestricted(tape_.graph, graph_plan_);
    const auto propagate_end = std::chrono::steady_clock::now();

    FixedGradientMixedHessianResult result;
    result.fixed_gradient_m.resize(static_cast<Eigen::Index>(fixed_ad_.size()));
    result.mixed_hessian_m.resize(static_cast<Eigen::Index>(random_ad_.size()),
                                  static_cast<Eigen::Index>(fixed_ad_.size()));
    result.mixed_hessian_m.setZero();
    for (size_t j = 0; j < fixed_ad_.size(); ++j) {
      result.fixed_gradient_m[static_cast<Eigen::Index>(j)] =
          had::GetAdjoint(fixed_ad_[j]);
    }
    const auto extract_mixed_tree = [this, &result](had::VertexId outer) {
      const int outer_fixed = fixed_vertex_to_index_[outer];
      const int outer_random = random_vertex_to_index_[outer];
      tape_.graph.ForEachSoEdge(
          outer, [this, &result, outer_fixed, outer_random](had::VertexId inner,
                                                            had::Real value) {
            const int inner_fixed = inner < fixed_vertex_to_index_.size()
                                        ? fixed_vertex_to_index_[inner]
                                        : -1;
            const int inner_random = inner < random_vertex_to_index_.size()
                                         ? random_vertex_to_index_[inner]
                                         : -1;
            if (outer_random >= 0 && inner_fixed >= 0)
              result.mixed_hessian_m(outer_random, inner_fixed) = value;
            else if (outer_fixed >= 0 && inner_random >= 0)
              result.mixed_hessian_m(inner_random, outer_fixed) = value;
          });
    };
    for (const auto &parameter : fixed_ad_)
      extract_mixed_tree(parameter.varId);
    for (const auto &parameter : random_ad_)
      extract_mixed_tree(parameter.varId);
    const auto extract_end = std::chrono::steady_clock::now();
    result.replay_ms =
        std::chrono::duration<double, std::milli>(replay_end - replay_start)
            .count();
    result.propagate_ms =
        std::chrono::duration<double, std::milli>(propagate_end - replay_end)
            .count();
    result.extract_ms =
        std::chrono::duration<double, std::milli>(extract_end - propagate_end)
            .count();
    return result;
  }

  Eigen::VectorXd
  solve_newton_system(const Eigen::SparseMatrix<double> &hessian,
                      const Eigen::VectorXd &rhs) {
    try {
      if (!newton_factorization_.analyzed())
        newton_factorization_.analyze_pattern(hessian);
      newton_factorization_.factorize(hessian);
    } catch (const std::exception &) {
      // Structural zeros can change the numeric sparse representation at
      // special parameter values. Recompute safely in that uncommon case.
      newton_factorization_.compute(hessian);
    }
    return newton_factorization_.solve(rhs);
  }

  double
  factorize_terminal_hessian(const Eigen::SparseMatrix<double> &hessian) {
    try {
      if (!newton_factorization_.analyzed())
        newton_factorization_.analyze_pattern(hessian);
      newton_factorization_.factorize(hessian);
    } catch (const std::exception &) {
      newton_factorization_.compute(hessian);
    }
    return newton_factorization_.logdet();
  }

  Eigen::VectorXd terminal_solve(const Eigen::VectorXd &rhs) const {
    return newton_factorization_.solve(rhs);
  }

  Eigen::MatrixXd terminal_solve(const Eigen::MatrixXd &rhs) const {
    return newton_factorization_.solve(rhs);
  }

  laplace::TakahashiSelectedInverse terminal_selected_inverse() const {
    return laplace::TakahashiSelectedInverse(
        newton_factorization_.matrixL(), newton_factorization_.vectorD(),
        newton_factorization_.permutationP());
  }

  const SparseLDLTFactorizationCache &terminal_factorization() const {
    return newton_factorization_;
  }

  std::size_t last_hessian_push_count() const {
    return last_hessian_push_count_;
  }

  had::ADGraphMemoryStatistics graph_memory_statistics() const {
    return had::MeasureADGraphMemory(tape_.graph);
  }

  std::size_t recorded_vertex_count() const {
    return tape_.graph.vertices.size();
  }

  std::shared_ptr<const had::SharedHessianTopology> FreezeHessianTopology(
      std::shared_ptr<const had::SharedHessianTopology> candidate = nullptr) {
    return tape_.graph.FreezeHessianTopology(std::move(candidate));
  }

private:
  double hessian_value(const AD &a, const AD &b) const {
    if (a.varId == b.varId)
      return tape_.graph.selfSoEdges[a.varId];
    return GetOffDiagonalHessian(tape_.graph, a.varId, b.varId);
  }

  Model &model_;
  ParameterPartition partition_;
  TapeContext tape_;
  std::vector<AD> fixed_ad_, random_ad_, full_ad_;
  std::vector<int> fixed_vertex_to_index_, random_vertex_to_index_;
  AD objective_;
  LaplaceGraphPlan graph_plan_;
  std::vector<int> random_idx_;
  double construction_parameter_ms_ = 0.0;
  double construction_record_ms_ = 0.0;
  double construction_plan_ms_ = 0.0;
  SparseHessianPattern pattern_;
  Eigen::SparseMatrix<double> hessian_pattern_template_;
  SparseLDLTFactorizationCache newton_factorization_;
  bool collect_reports_ = true;
  std::size_t last_hessian_push_count_ = 0;
};

// Evaluate f(theta, u), gradient wrt u, and sparse Hessian wrt u.
//
// This is the bridge between the newer model/partition layer and the older
// sparse Laplace kernels in core/laplace.hpp. The active AD variables are the
// random effects. Fixed effects are injected as AD constants so the AD graph
// still represents the full model evaluation while gradients/Hessian are
// extracted only for u.
template <class Model>
inline RandomEffectHessianResult
evaluate_random_effect_hessian(Model &model, const std::vector<double> &fixed,
                               const std::vector<double> &random,
                               const ParameterPartition &partition,
                               double drop_tol = 0.0) {
  if (random.size() != partition.random_indices_m.size()) {
    throw std::invalid_argument(
        "evaluate_random_effect_hessian: random vector has incorrect length");
  }

  if (fixed.size() != partition.fixed_indices_m.size()) {
    throw std::invalid_argument(
        "evaluate_random_effect_hessian: fixed vector has incorrect length");
  }

  TapeContext tape;
  ADScope scope(tape.graph);

  ModelReportContext ctx;
  model.initialize(ctx);

  std::vector<AD> fixed_ad;
  fixed_ad.reserve(fixed.size());

  for (double theta_i : fixed) {
    fixed_ad.push_back(AD(theta_i));
  }

  std::vector<AD> random_ad = to_ad(random);
  std::vector<AD> full_ad = merge_parameters(fixed_ad, random_ad, partition);

  AD objective = model.template evaluate<AD>(full_ad, ctx);

  std::vector<had::VertexId> fixed_vertices;
  std::vector<had::VertexId> random_vertices;
  fixed_vertices.reserve(fixed_ad.size());
  random_vertices.reserve(random_ad.size());
  for (const auto &parameter : fixed_ad)
    fixed_vertices.push_back(parameter.varId);
  for (const auto &parameter : random_ad)
    random_vertices.push_back(parameter.varId);
  LaplaceGraphPlan graph_plan;
  graph_plan.Build(tape.graph, fixed_vertices, random_vertices,
                   objective.varId);
  PropagateRandomHessianRestricted(tape.graph, graph_plan);

  Eigen::VectorXd g = extract_gradient(random_ad);
  Eigen::VectorXd g_fixed = extract_gradient(fixed_ad);

  const std::vector<int> random_idx = random_indices_as_ints(partition);

  const auto &pattern = get_pattern(scope, full_ad, random_idx);

  Eigen::SparseMatrix<double> H =
      extract_sparse_hessian(scope, full_ad, random_idx, pattern, drop_tol);

  RandomEffectHessianResult result;
  result.objective_value_m = value_of(objective);
  result.fixed_m = fixed;
  result.random_m = random;
  result.full_m = merge_parameters(fixed, random, partition);
  result.reports_m = ctx.reports().values();
  result.hessian_random_m = H;

  result.gradient_random_m.resize(static_cast<size_t>(g.size()));
  result.gradient_fixed_m.assign(g_fixed.data(),
                                 g_fixed.data() + g_fixed.size());
  result.mixed_hessian_m =
      Eigen::MatrixXd::Zero(random_ad.size(), fixed_ad.size());
  for (int i = 0; i < result.mixed_hessian_m.rows(); ++i) {
    for (int j = 0; j < result.mixed_hessian_m.cols(); ++j) {
      result.mixed_hessian_m(i, j) =
          had::GetAdjoint(random_ad[static_cast<std::size_t>(i)],
                          fixed_ad[static_cast<std::size_t>(j)]);
    }
  }

  for (int i = 0; i < g.size(); ++i) {
    result.gradient_random_m[static_cast<size_t>(i)] = g[i];
  }

  result.gradient_norm_m =
      random_effect_gradient_norm(result.gradient_random_m);

  return result;
}

template <class Model>
inline RandomEffectHessianResult
evaluate_random_effect_hessian(Model &model, const std::vector<double> &fixed,
                               const std::vector<double> &random,
                               const ParameterSet &parameters,
                               double drop_tol = 0.0) {
  return evaluate_random_effect_hessian(
      model, fixed, random, partition_parameters(parameters), drop_tol);
}

inline Eigen::MatrixXd
dense_random_hessian(const RandomEffectHessianResult &result) {
  return Eigen::MatrixXd(result.hessian_random_m);
}

} // namespace quadra
