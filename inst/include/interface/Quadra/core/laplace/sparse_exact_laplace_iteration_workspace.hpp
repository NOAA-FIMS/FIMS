#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

#include "had_quadra_replay_reuse_lazy_implicit_hdot_provider.hpp"
#include "sparse_huu_factorization.hpp"

namespace quadra {
namespace laplace {

struct SparseExactLaplaceIterationWorkspaceOptions {
  double hdot_drop_tol = 0.0;
};

// Sparse per-iteration exact Laplace workspace.
//
// This is the sparse-H_uu analog of ExactLaplaceIterationWorkspace.
// It owns:
//   - sparse H_uu
//   - sparse SimplicialLDLT factorization
//   - lazy implicit direction solves
//   - cached Hdot matrices
//   - cached logdet gradient contribution
template <class CombinedObjectiveFn, class CrossDerivativeFn>
class SparseExactLaplaceIterationWorkspace {
public:
  SparseExactLaplaceIterationWorkspace(
      CombinedObjectiveFn combined_objective,
      CrossDerivativeFn cross_derivative_fn, int theta_dim, int random_dim,
      RandomHessianPattern random_hessian_pattern,
      std::vector<int> active_directions, const Eigen::VectorXd &theta,
      const Eigen::VectorXd &uhat, const Eigen::SparseMatrix<double> &Huu,
      SparseExactLaplaceIterationWorkspaceOptions options =
          SparseExactLaplaceIterationWorkspaceOptions{})
      : combined_objective_(std::move(combined_objective)),
        cross_derivative_fn_(std::move(cross_derivative_fn)),
        theta_dim_(theta_dim), random_dim_(random_dim),
        pattern_(std::move(random_hessian_pattern)),
        active_directions_(std::move(active_directions)), theta_(theta),
        uhat_(uhat), Huu_(Huu), factor_(Huu), options_(options) {
    validate();

    std::sort(active_directions_.begin(), active_directions_.end());
    active_directions_.erase(
        std::unique(active_directions_.begin(), active_directions_.end()),
        active_directions_.end());
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

    return -factor_.solve(rhs);
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

    cached_logdet_gradient_ = Eigen::VectorXd::Zero(theta_dim_);

    for (int j = 0; j < theta_dim_; ++j) {
      const auto &Hdot = cached_Hdots_[static_cast<size_t>(j)];
      if (Hdot.nonZeros() == 0) {
        continue;
      }

      cached_logdet_gradient_[j] = 0.5 * factor_.trace_inverse_times(Hdot);
    }

    trace_terms_prepared_ = true;
  }

  Eigen::VectorXd
  gradient(const Eigen::VectorXd &joint_envelope_gradient) const {
    if (joint_envelope_gradient.size() != theta_dim_) {
      throw std::invalid_argument("joint_envelope_gradient has wrong length.");
    }

    prepare_trace_terms();
    return joint_envelope_gradient + cached_logdet_gradient_;
  }

  const Eigen::VectorXd &logdet_gradient_contribution() const {
    prepare_trace_terms();
    return cached_logdet_gradient_;
  }

  double logdet_huu() const { return factor_.logdet(); }

  const std::vector<int> &active_directions() const {
    return active_directions_;
  }

  const Eigen::SparseMatrix<double> &Huu() const { return Huu_; }

private:
  CombinedObjectiveFn combined_objective_;
  CrossDerivativeFn cross_derivative_fn_;
  int theta_dim_;
  int random_dim_;
  RandomHessianPattern pattern_;
  std::vector<int> active_directions_;
  Eigen::VectorXd theta_;
  Eigen::VectorXd uhat_;
  Eigen::SparseMatrix<double> Huu_;
  SparseHuuFactorization factor_;
  SparseExactLaplaceIterationWorkspaceOptions options_;

  mutable bool trace_terms_prepared_ = false;
  mutable std::vector<Eigen::SparseMatrix<double>> cached_Hdots_;
  mutable Eigen::VectorXd cached_logdet_gradient_;

  void validate() const {
    if (theta_dim_ <= 0 || random_dim_ <= 0) {
      throw std::invalid_argument("dimensions must be positive.");
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
auto make_sparse_exact_laplace_iteration_workspace(
    CombinedObjectiveFn combined_objective,
    CrossDerivativeFn cross_derivative_fn, int theta_dim, int random_dim,
    RandomHessianPattern random_hessian_pattern,
    std::vector<int> active_directions, const Eigen::VectorXd &theta,
    const Eigen::VectorXd &uhat, const Eigen::SparseMatrix<double> &Huu,
    SparseExactLaplaceIterationWorkspaceOptions options =
        SparseExactLaplaceIterationWorkspaceOptions{}) {
  return SparseExactLaplaceIterationWorkspace<CombinedObjectiveFn,
                                              CrossDerivativeFn>(
      std::move(combined_objective), std::move(cross_derivative_fn), theta_dim,
      random_dim, std::move(random_hessian_pattern),
      std::move(active_directions), theta, uhat, Huu, options);
}

} // namespace laplace
} // namespace quadra
