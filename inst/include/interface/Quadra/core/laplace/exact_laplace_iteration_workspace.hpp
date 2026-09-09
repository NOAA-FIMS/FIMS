#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

#include "had_quadra_replay_reuse_lazy_implicit_hdot_provider.hpp"
#include "implicit_direction_solve_provider.hpp"
#include "sparse_trace_contraction.hpp"

namespace quadra {
namespace laplace {

struct ExactLaplaceIterationWorkspaceOptions {
  double hdot_drop_tol = 0.0;
};

// Per-iteration workspace for exact Laplace gradients.
//
// This object is intentionally tied to one optimizer evaluation point:
//   theta, uhat, H_uu(theta, uhat)
//
// It owns reusable state that should not be rebuilt for every directional
// derivative:
//   - H_uu factorization
//   - active direction set
//   - lazy implicit direction solves
//
// CrossDerivativeFn must be callable as:
//   Eigen::VectorXd operator()(int theta_index) const;
//
// and return f_{u theta_j} at fixed u.
//
// CombinedObjectiveFn must be callable as:
//   template <class AD>
//   AD operator()(const std::vector<AD>& x) const;
template <class CombinedObjectiveFn, class CrossDerivativeFn>
class ExactLaplaceIterationWorkspace {
public:
  ExactLaplaceIterationWorkspace(CombinedObjectiveFn combined_objective,
                                 CrossDerivativeFn cross_derivative_fn,
                                 int theta_dim, int random_dim,
                                 RandomHessianPattern random_hessian_pattern,
                                 std::vector<int> active_directions,
                                 const Eigen::VectorXd &theta,
                                 const Eigen::VectorXd &uhat,
                                 const Eigen::MatrixXd &Huu,
                                 ExactLaplaceIterationWorkspaceOptions options =
                                     ExactLaplaceIterationWorkspaceOptions{})
      : combined_objective_(std::move(combined_objective)),
        cross_derivative_fn_(std::move(cross_derivative_fn)),
        theta_dim_(theta_dim), random_dim_(random_dim),
        pattern_(std::move(random_hessian_pattern)),
        active_directions_(std::move(active_directions)), theta_(theta),
        uhat_(uhat), Huu_(Huu), ldlt_(Huu), options_(options) {
    validate();

    if (ldlt_.info() != Eigen::Success) {
      throw std::runtime_error(
          "ExactLaplaceIterationWorkspace: LDLT factorization failed.");
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
  }

  Eigen::VectorXd u_direction(int theta_index) const {
    if (theta_index < 0 || theta_index >= theta_dim_) {
      throw std::out_of_range("theta_index out of range.");
    }

    Eigen::VectorXd rhs = cross_derivative_fn_(theta_index);
    if (rhs.size() != random_dim_) {
      throw std::runtime_error(
          "cross derivative callback returned wrong length.");
    }

    return -ldlt_.solve(rhs);
  }

  std::vector<Eigen::SparseMatrix<double>> compute_total_hdot_all() const {
    auto direction_provider = [this](int theta_index) -> Eigen::VectorXd {
      return this->u_direction(theta_index);
    };

    auto hdot_provider =
        make_had_quadra_replay_reuse_lazy_implicit_hdot_provider(
            combined_objective_, direction_provider, theta_dim_, random_dim_,
            pattern_, active_directions_, options_.hdot_drop_tol);

    return hdot_provider.compute_all_sparse(theta_, uhat_);
  }

  void prepare_trace_terms() const {
    if (trace_terms_prepared_) {
      return;
    }

    cached_Hdots_ = compute_total_hdot_all();

    if (static_cast<int>(cached_Hdots_.size()) != theta_dim_) {
      throw std::runtime_error(
          "compute_total_hdot_all returned wrong number of matrices.");
    }

    const auto needed_columns =
        needed_columns_from_sparse_matrices(cached_Hdots_);

    SelectedInverseColumnTraceCache trace_cache(ldlt_, random_dim_,
                                                needed_columns);

    cached_logdet_gradient_ = Eigen::VectorXd::Zero(theta_dim_);

    for (int j = 0; j < theta_dim_; ++j) {
      const auto &Hdot = cached_Hdots_[static_cast<size_t>(j)];
      if (Hdot.nonZeros() == 0) {
        continue;
      }

      cached_logdet_gradient_[j] = 0.5 * trace_cache.trace(Hdot);
    }

    trace_terms_prepared_ = true;
  }

  const Eigen::VectorXd &logdet_gradient_contribution() const {
    prepare_trace_terms();
    return cached_logdet_gradient_;
  }

  const std::vector<Eigen::SparseMatrix<double>> &cached_Hdots() const {
    prepare_trace_terms();
    return cached_Hdots_;
  }

  bool trace_terms_prepared() const { return trace_terms_prepared_; }

  Eigen::VectorXd
  gradient(const Eigen::VectorXd &joint_envelope_gradient) const {
    if (joint_envelope_gradient.size() != theta_dim_) {
      throw std::invalid_argument("joint_envelope_gradient has wrong length.");
    }

    prepare_trace_terms();
    return joint_envelope_gradient + cached_logdet_gradient_;
  }

  const Eigen::VectorXd &theta() const { return theta_; }

  const Eigen::VectorXd &uhat() const { return uhat_; }

  const Eigen::MatrixXd &Huu() const { return Huu_; }

  const std::vector<int> &active_directions() const {
    return active_directions_;
  }

  int theta_dim() const { return theta_dim_; }

  int random_dim() const { return random_dim_; }

private:
  CombinedObjectiveFn combined_objective_;
  CrossDerivativeFn cross_derivative_fn_;
  int theta_dim_;
  int random_dim_;
  RandomHessianPattern pattern_;
  std::vector<int> active_directions_;
  Eigen::VectorXd theta_;
  Eigen::VectorXd uhat_;
  Eigen::MatrixXd Huu_;
  Eigen::LDLT<Eigen::MatrixXd> ldlt_;
  mutable bool trace_terms_prepared_ = false;
  mutable std::vector<Eigen::SparseMatrix<double>> cached_Hdots_;
  mutable Eigen::VectorXd cached_logdet_gradient_;
  ExactLaplaceIterationWorkspaceOptions options_;

  void validate() const {
    if (theta_dim_ <= 0) {
      throw std::invalid_argument("theta_dim must be positive.");
    }
    if (random_dim_ <= 0) {
      throw std::invalid_argument("random_dim must be positive.");
    }
    if (theta_.size() != theta_dim_) {
      throw std::invalid_argument("theta has wrong length.");
    }
    if (uhat_.size() != random_dim_) {
      throw std::invalid_argument("uhat has wrong length.");
    }
    if (Huu_.rows() != random_dim_ || Huu_.cols() != random_dim_) {
      throw std::invalid_argument("Huu has wrong dimensions.");
    }
    for (const auto &entry : pattern_) {
      if (entry.first < 0 || entry.first >= random_dim_ || entry.second < 0 ||
          entry.second >= random_dim_) {
        throw std::out_of_range("random Hessian pattern entry out of range.");
      }
    }
  }
};

template <class CombinedObjectiveFn, class CrossDerivativeFn>
auto make_exact_laplace_iteration_workspace(
    CombinedObjectiveFn combined_objective,
    CrossDerivativeFn cross_derivative_fn, int theta_dim, int random_dim,
    RandomHessianPattern random_hessian_pattern,
    std::vector<int> active_directions, const Eigen::VectorXd &theta,
    const Eigen::VectorXd &uhat, const Eigen::MatrixXd &Huu,
    ExactLaplaceIterationWorkspaceOptions options =
        ExactLaplaceIterationWorkspaceOptions{}) {
  return ExactLaplaceIterationWorkspace<CombinedObjectiveFn, CrossDerivativeFn>(
      std::move(combined_objective), std::move(cross_derivative_fn), theta_dim,
      random_dim, std::move(random_hessian_pattern),
      std::move(active_directions), theta, uhat, Huu, options);
}

} // namespace laplace
} // namespace quadra
