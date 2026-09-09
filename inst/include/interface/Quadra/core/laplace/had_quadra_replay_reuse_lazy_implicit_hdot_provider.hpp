#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "../autodiff/laplace_graph_plan.hpp"
#include "had_quadra_replay_reuse_sparse_hdot_provider.hpp"
#include "reverse_hessian_trace_contraction.hpp"
#include "sparse_trace_contraction.hpp"

namespace quadra {
namespace laplace {

// Seed total derivative direction lazily:
//
//   theta_dot = e_theta_index
//   u_dot     = u_direction
//
// where u_direction is usually:
//   du*/dtheta_j = - H_uu^{-1} f_{u theta_j}
inline void seed_had_quadra_lazy_implicit_direction(
    std::vector<had::AReal> &x, had::ADGraph &graph, int theta_dim,
    int random_dim, int theta_index, const Eigen::VectorXd &u_direction) {
  if (theta_index < 0 || theta_index >= theta_dim) {
    throw std::out_of_range("theta_index out of range.");
  }
  if (u_direction.size() != random_dim) {
    throw std::invalid_argument("u_direction has wrong length.");
  }

  const int n = theta_dim + random_dim;
  if (static_cast<int>(x.size()) != n) {
    throw std::invalid_argument("x has wrong length.");
  }

  for (int k = 0; k < theta_dim; ++k) {
    const double d = (k == theta_index) ? 1.0 : 0.0;
    x[static_cast<size_t>(k)].dot = d;
    had::VertexDot(graph, x[static_cast<size_t>(k)].varId) = d;
  }

  for (int r = 0; r < random_dim; ++r) {
    const int idx = theta_dim + r;
    const double d = u_direction[r];
    x[static_cast<size_t>(idx)].dot = d;
    had::VertexDot(graph, x[static_cast<size_t>(idx)].varId) = d;
  }
}

// Replay-reuse exact total-Hdot provider with lazy u-direction solves.
//
// UDirectionProvider must be callable as:
//
//   Eigen::VectorXd operator()(int theta_index) const;
//
// and return du*/dtheta_j.
template <class CombinedObjectiveFn, class UDirectionProvider>
class HadQuadraReplayReuseLazyImplicitHdotProvider {
public:
  HadQuadraReplayReuseLazyImplicitHdotProvider(
      CombinedObjectiveFn combined_objective,
      UDirectionProvider u_direction_provider, int theta_dim, int random_dim,
      RandomHessianPattern pattern, std::vector<int> active_directions,
      double drop_tol = 0.0, std::size_t expected_vertex_count = 0)
      : combined_objective_(std::move(combined_objective)),
        u_direction_provider_(std::move(u_direction_provider)),
        theta_dim_(theta_dim), random_dim_(random_dim),
        pattern_(std::move(pattern)),
        active_directions_(std::move(active_directions)), drop_tol_(drop_tol),
        expected_vertex_count_(expected_vertex_count) {
    if (theta_dim_ <= 0) {
      throw std::invalid_argument("theta_dim must be positive.");
    }
    if (random_dim_ <= 0) {
      throw std::invalid_argument("random_dim must be positive.");
    }
    if (drop_tol_ < 0.0 || !std::isfinite(drop_tol_)) {
      throw std::invalid_argument("drop_tol must be nonnegative and finite.");
    }

    std::sort(active_directions_.begin(), active_directions_.end());
    active_directions_.erase(
        std::unique(active_directions_.begin(), active_directions_.end()),
        active_directions_.end());

    for (int idx : active_directions_) {
      if (idx < 0 || idx >= theta_dim_) {
        throw std::out_of_range("active direction out of range.");
      }
    }

    for (const auto &entry : pattern_) {
      if (entry.first < 0 || entry.first >= random_dim_ || entry.second < 0 ||
          entry.second >= random_dim_) {
        throw std::out_of_range("random Hessian pattern entry out of range.");
      }
    }
  }

  std::vector<Eigen::SparseMatrix<double>>
  compute_all_sparse(const Eigen::VectorXd &theta,
                     const Eigen::VectorXd &uhat) const {
    std::vector<Eigen::SparseMatrix<double>> out(
        static_cast<size_t>(theta_dim_));
    for_each_sparse(theta, uhat,
                    [&out](int direction, Eigen::SparseMatrix<double> hdot) {
                      out[static_cast<size_t>(direction)] = std::move(hdot);
                    });
    return out;
  }

  // Memory-first traversal: replay one direction, hand its sparse Hdot to
  // the caller, and release it before continuing.
  template <class Consumer>
  void for_each_sparse(const Eigen::VectorXd &theta,
                       const Eigen::VectorXd &uhat, Consumer consumer) const {
    ensure_replayed(theta, uhat);

    for (int j : active_directions_) {

      const Eigen::VectorXd u_direction = u_direction_provider_(j);
      if (u_direction.size() != random_dim_) {
        throw std::runtime_error("u_direction_provider returned wrong length.");
      }

      reset_had_quadra_directional_reverse_state(*graph_);
      seed_had_quadra_lazy_implicit_direction(x_, *graph_, theta_dim_,
                                              random_dim_, j, u_direction);
      retangent_had_quadra_graph(*graph_);

      had::SetAdjoint(y_, had::Real(1.0));
      PropagateRandomHessianDirectionalRestricted(*graph_, graph_plan_);

      consumer(j, extract_sparse_hdot(x_));
    }

    had::g_ADGraph = nullptr;
  }

  template <class SelectedInverseAccessor>
  ReverseHessianTraceResult compute_trace_contraction(
      const Eigen::VectorXd &theta, const Eigen::VectorXd &uhat,
      SelectedInverseAccessor selected_inverse,
      std::size_t memory_cap_bytes = 64U * 1024U * 1024U) const {
    ensure_replayed(theta, uhat);
    std::vector<had::AReal> fixed(
        x_.begin(), x_.begin() + static_cast<std::ptrdiff_t>(theta_dim_));
    std::vector<had::AReal> random(
        x_.begin() + static_cast<std::ptrdiff_t>(theta_dim_), x_.end());
    ReverseHessianTraceResult result = reverse_hessian_trace_contraction(
        *graph_, graph_plan_, fixed, random, pattern_,
        std::move(selected_inverse), memory_cap_bytes);
    had::g_ADGraph = nullptr;
    return result;
  }

  const std::vector<int> &active_directions() const {
    return active_directions_;
  }

  void set_u_direction_provider(UDirectionProvider provider) {
    u_direction_provider_ = std::move(provider);
  }

  int tape_build_count() const { return tape_build_count_; }

  bool replay_matches(const Eigen::VectorXd &theta, const Eigen::VectorXd &uhat,
                      double relative_tolerance = 1e-11) const {
    ensure_replayed(theta, uhat);
    std::vector<double> values;
    values.reserve(static_cast<std::size_t>(theta_dim_ + random_dim_));
    for (int j = 0; j < theta_dim_; ++j)
      values.push_back(theta[j]);
    for (int i = 0; i < random_dim_; ++i)
      values.push_back(uhat[i]);
    const double direct = combined_objective_(values);
    const double replayed = y_.val;
    const double scale = 1.0 + std::max(std::abs(direct), std::abs(replayed));
    return std::abs(direct - replayed) <= relative_tolerance * scale;
  }

  had::ADGraphMemoryStatistics graph_memory_statistics() const {
    return graph_ ? had::MeasureADGraphMemory(*graph_)
                  : had::ADGraphMemoryStatistics{};
  }

  std::shared_ptr<const had::SharedHessianTopology> FreezeHessianTopology(
      std::shared_ptr<const had::SharedHessianTopology> candidate = nullptr) {
    return graph_ ? graph_->FreezeHessianTopology(std::move(candidate))
                  : std::move(candidate);
  }

private:
  CombinedObjectiveFn combined_objective_;
  UDirectionProvider u_direction_provider_;
  int theta_dim_;
  int random_dim_;
  RandomHessianPattern pattern_;
  std::vector<int> active_directions_;
  double drop_tol_;
  std::size_t expected_vertex_count_;
  mutable std::unique_ptr<had::ADGraph> graph_;
  mutable std::vector<had::AReal> x_;
  mutable had::AReal y_;
  mutable int tape_build_count_ = 0;
  mutable LaplaceGraphPlan graph_plan_;

  void ensure_replayed(const Eigen::VectorXd &theta,
                       const Eigen::VectorXd &uhat) const {
    if (theta.size() != theta_dim_) {
      throw std::invalid_argument("theta has wrong length.");
    }
    if (uhat.size() != random_dim_) {
      throw std::invalid_argument("uhat has wrong length.");
    }

    if (!graph_) {
      graph_ = std::make_unique<had::ADGraph>();
      had::g_ADGraph = graph_.get();
      if (expected_vertex_count_ > 0)
        graph_->ReserveRecordedVertices(expected_vertex_count_);
      x_.reserve(static_cast<size_t>(theta_dim_ + random_dim_));
      for (int j = 0; j < theta_dim_; ++j)
        x_.emplace_back(theta[j]);
      for (int i = 0; i < random_dim_; ++i)
        x_.emplace_back(uhat[i]);
      y_ = combined_objective_(x_);
      std::vector<had::VertexId> fixed_ids, random_ids;
      fixed_ids.reserve(static_cast<size_t>(theta_dim_));
      random_ids.reserve(static_cast<size_t>(random_dim_));
      for (int j = 0; j < theta_dim_; ++j)
        fixed_ids.push_back(x_[static_cast<size_t>(j)].varId);
      for (int i = 0; i < random_dim_; ++i)
        random_ids.push_back(x_[static_cast<size_t>(theta_dim_ + i)].varId);
      graph_plan_.Build(*graph_, fixed_ids, random_ids, y_.varId);
      ++tape_build_count_;
    } else {
      had::g_ADGraph = graph_.get();
      for (int j = 0; j < theta_dim_; ++j)
        had::SetValue(x_[static_cast<size_t>(j)], theta[j]);
      for (int i = 0; i < random_dim_; ++i)
        had::SetValue(x_[static_cast<size_t>(theta_dim_ + i)], uhat[i]);
      graph_->Forward();
      y_.val = graph_->vertices[y_.varId].primal;
    }
  }

  bool is_active(int theta_index) const {
    return std::binary_search(active_directions_.begin(),
                              active_directions_.end(), theta_index);
  }

  Eigen::SparseMatrix<double>
  extract_sparse_hdot(const std::vector<had::AReal> &x) const {
    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(pattern_.size() * 2);

    for (const auto &entry : pattern_) {
      const int a = entry.first;
      const int b = entry.second;

      const int ia = theta_dim_ + a;
      const int ib = theta_dim_ + b;

      double value = had::GetAdjointDot(x[static_cast<size_t>(ia)],
                                        x[static_cast<size_t>(ib)]);

      if (std::abs(value) <= drop_tol_) {
        continue;
      }

      triplets.emplace_back(a, b, value);
      if (a != b) {
        triplets.emplace_back(b, a, value);
      }
    }

    Eigen::SparseMatrix<double> Hdot(random_dim_, random_dim_);
    Hdot.setFromTriplets(triplets.begin(), triplets.end());
    Hdot.makeCompressed();
    return Hdot;
  }
};

template <class CombinedObjectiveFn, class UDirectionProvider>
auto make_had_quadra_replay_reuse_lazy_implicit_hdot_provider(
    CombinedObjectiveFn combined_objective,
    UDirectionProvider u_direction_provider, int theta_dim, int random_dim,
    RandomHessianPattern pattern, std::vector<int> active_directions,
    double drop_tol = 0.0, std::size_t expected_vertex_count = 0) {
  return HadQuadraReplayReuseLazyImplicitHdotProvider<CombinedObjectiveFn,
                                                      UDirectionProvider>(
      std::move(combined_objective), std::move(u_direction_provider), theta_dim,
      random_dim, std::move(pattern), std::move(active_directions), drop_tol,
      expected_vertex_count);
}

// Exact Laplace gradient using lazy total implicit Hdot and cached sparse
// trace.
template <class HessianUUFn, class LazyImplicitProvider>
Eigen::VectorXd full_exact_laplace_gradient_lazy_implicit_cached_trace(
    const Eigen::VectorXd &grad_joint_envelope,
    const HessianUUFn &hessian_uu_at_mode, const LazyImplicitProvider &provider,
    const Eigen::VectorXd &theta, const Eigen::VectorXd &uhat) {
  if (grad_joint_envelope.size() != theta.size()) {
    throw std::invalid_argument(
        "grad_joint_envelope and theta must have same length.");
  }

  const Eigen::MatrixXd H = hessian_uu_at_mode(theta, uhat);

  Eigen::LDLT<Eigen::MatrixXd> ldlt(H);
  if (ldlt.info() != Eigen::Success) {
    throw std::runtime_error("LDLT factorization failed.");
  }

  const auto Hdots = provider.compute_all_sparse(theta, uhat);

  if (static_cast<int>(Hdots.size()) != theta.size()) {
    throw std::runtime_error(
        "provider returned wrong number of Hdot matrices.");
  }

  const auto needed_columns = needed_columns_from_sparse_matrices(Hdots);
  SelectedInverseColumnTraceCache trace_cache(ldlt, static_cast<int>(H.rows()),
                                              needed_columns);

  Eigen::VectorXd grad = grad_joint_envelope;

  for (int j = 0; j < theta.size(); ++j) {
    const auto &Hdot = Hdots[static_cast<size_t>(j)];
    if (Hdot.nonZeros() == 0) {
      continue;
    }

    const double trace = trace_cache.trace(Hdot);
    grad[j] += 0.5 * trace;
  }

  return grad;
}

} // namespace laplace
} // namespace quadra
