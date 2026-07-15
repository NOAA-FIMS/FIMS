#pragma once

#include <stdexcept>
#include <vector>

#include "../laplace.hpp"
#include "../autodiff/laplace_graph_plan.hpp"
#include "random_effect_objective.hpp"

namespace quadra {

struct RandomEffectHessianResult {
  double objective_value_m = 0.0;
  double gradient_norm_m = 0.0;

  std::vector<double> fixed_m;
  std::vector<double> random_m;
  std::vector<double> full_m;
  std::vector<double> gradient_random_m;

  Eigen::SparseMatrix<double> hessian_random_m;

  std::vector<ReportValue> reports_m;
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
