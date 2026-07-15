#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

#include "had_quadra_replay_reuse_sparse_hdot_provider.hpp"
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
    graph.vertices[x[static_cast<size_t>(k)].varId].dot = d;
  }

  for (int r = 0; r < random_dim; ++r) {
    const int idx = theta_dim + r;
    const double d = u_direction[r];
    x[static_cast<size_t>(idx)].dot = d;
    graph.vertices[x[static_cast<size_t>(idx)].varId].dot = d;
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
      double drop_tol = 0.0)
      : combined_objective_(std::move(combined_objective)),
        u_direction_provider_(std::move(u_direction_provider)),
        theta_dim_(theta_dim), random_dim_(random_dim),
        pattern_(std::move(pattern)),
        active_directions_(std::move(active_directions)), drop_tol_(drop_tol) {
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
    if (theta.size() != theta_dim_) {
      throw std::invalid_argument("theta has wrong length.");
    }
    if (uhat.size() != random_dim_) {
      throw std::invalid_argument("uhat has wrong length.");
    }

    had::ADGraph graph;
    had::g_ADGraph = &graph;

    std::vector<had::AReal> x;
    x.reserve(static_cast<size_t>(theta_dim_ + random_dim_));

    for (int j = 0; j < theta_dim_; ++j) {
      x.emplace_back(theta[j]);
      x.back().dot = 0.0;
      graph.vertices[x.back().varId].dot = 0.0;
    }

    for (int i = 0; i < random_dim_; ++i) {
      x.emplace_back(uhat[i]);
      x.back().dot = 0.0;
      graph.vertices[x.back().varId].dot = 0.0;
    }

    had::AReal y = combined_objective_(x);

    std::vector<Eigen::SparseMatrix<double>> out;
    out.resize(static_cast<size_t>(theta_dim_));

    for (int j = 0; j < theta_dim_; ++j) {
      if (!is_active(j)) {
        out[static_cast<size_t>(j)].resize(random_dim_, random_dim_);
        out[static_cast<size_t>(j)].makeCompressed();
        continue;
      }

      const Eigen::VectorXd u_direction = u_direction_provider_(j);
      if (u_direction.size() != random_dim_) {
        throw std::runtime_error("u_direction_provider returned wrong length.");
      }

      reset_had_quadra_directional_reverse_state(graph);
      seed_had_quadra_lazy_implicit_direction(x, graph, theta_dim_, random_dim_,
                                              j, u_direction);
      retangent_had_quadra_graph(graph);

      had::SetAdjoint(y, had::Real(1.0));
      had::PropagateAdjointDirectional();

      out[static_cast<size_t>(j)] = extract_sparse_hdot(x);
    }

    had::g_ADGraph = nullptr;
    return out;
  }

  const std::vector<int> &active_directions() const {
    return active_directions_;
  }

private:
  CombinedObjectiveFn combined_objective_;
  UDirectionProvider u_direction_provider_;
  int theta_dim_;
  int random_dim_;
  RandomHessianPattern pattern_;
  std::vector<int> active_directions_;
  double drop_tol_;

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
    double drop_tol = 0.0) {
  return HadQuadraReplayReuseLazyImplicitHdotProvider<CombinedObjectiveFn,
                                                      UDirectionProvider>(
      std::move(combined_objective), std::move(u_direction_provider), theta_dim,
      random_dim, std::move(pattern), std::move(active_directions), drop_tol);
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
