#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

#include "auto_active_direction_hdot_provider.hpp"
#include "had_quadra_replay_reuse_sparse_hdot_provider.hpp"
#include "had_quadra_sparse_exact_hdot_provider.hpp"
#include "sparse_trace_contraction.hpp"

namespace quadra {
namespace laplace {

// Production-style exact Laplace gradient context.
//
// Combines:
//   - active Hdot direction discovery,
//   - replay-reuse Hdot extraction,
//   - cached sparse trace contraction.
//
// CombinedObjectiveFn must be callable as:
//
//   template <class AD>
//   AD operator()(const std::vector<AD>& x) const;
//
// where x = [theta, u].
//
// HessianUUFn supplied to gradient(...) must return H_uu(theta, uhat).
template <class CombinedObjectiveFn>
class AutoReplayCachedLaplaceGradientContext {
public:
  AutoReplayCachedLaplaceGradientContext(CombinedObjectiveFn combined_objective,
                                         int theta_dim, int random_dim,
                                         RandomHessianPattern pattern,
                                         const Eigen::VectorXd &discovery_theta,
                                         const Eigen::VectorXd &discovery_uhat,
                                         double discovery_tol = 0.0,
                                         double drop_tol = 0.0)
      : combined_objective_(std::move(combined_objective)),
        theta_dim_(theta_dim), random_dim_(random_dim),
        pattern_(std::move(pattern)), discovery_tol_(discovery_tol),
        drop_tol_(drop_tol) {
    if (theta_dim_ <= 0) {
      throw std::invalid_argument("theta_dim must be positive.");
    }
    if (random_dim_ <= 0) {
      throw std::invalid_argument("random_dim must be positive.");
    }
    if (discovery_theta.size() != theta_dim_) {
      throw std::invalid_argument("discovery_theta has wrong length.");
    }
    if (discovery_uhat.size() != random_dim_) {
      throw std::invalid_argument("discovery_uhat has wrong length.");
    }

    discover_active_directions(discovery_theta, discovery_uhat);
  }

  template <class HessianUUFn>
  Eigen::VectorXd gradient(const Eigen::VectorXd &grad_joint_envelope,
                           const HessianUUFn &hessian_uu_at_fixed_uhat,
                           const Eigen::VectorXd &theta,
                           const Eigen::VectorXd &uhat) const {
    if (grad_joint_envelope.size() != theta_dim_) {
      throw std::invalid_argument("grad_joint_envelope has wrong length.");
    }
    if (theta.size() != theta_dim_) {
      throw std::invalid_argument("theta has wrong length.");
    }
    if (uhat.size() != random_dim_) {
      throw std::invalid_argument("uhat has wrong length.");
    }

    auto provider = make_had_quadra_replay_reuse_sparse_hdot_provider(
        combined_objective_, theta_dim_, random_dim_, pattern_,
        discovery_.active_directions, drop_tol_);

    return full_exact_laplace_gradient_replay_reuse_cached_trace(
        grad_joint_envelope, hessian_uu_at_fixed_uhat, provider, theta, uhat);
  }

  const ActiveDirectionDiscoveryResult &discovery() const { return discovery_; }

  const std::vector<int> &active_directions() const {
    return discovery_.active_directions;
  }

private:
  CombinedObjectiveFn combined_objective_;
  int theta_dim_;
  int random_dim_;
  RandomHessianPattern pattern_;
  double discovery_tol_;
  double drop_tol_;
  ActiveDirectionDiscoveryResult discovery_;

  void discover_active_directions(const Eigen::VectorXd &theta,
                                  const Eigen::VectorXd &uhat) {
    auto base = make_had_quadra_sparse_exact_hdot_provider(
        combined_objective_, theta_dim_, random_dim_, pattern_, drop_tol_);

    auto auto_active = make_auto_active_direction_hdot_provider(
        base, theta, uhat, theta_dim_, random_dim_, discovery_tol_);

    discovery_ = auto_active.discovery();
  }
};

template <class CombinedObjectiveFn>
auto make_auto_replay_cached_laplace_gradient_context(
    CombinedObjectiveFn combined_objective, int theta_dim, int random_dim,
    RandomHessianPattern pattern, const Eigen::VectorXd &discovery_theta,
    const Eigen::VectorXd &discovery_uhat, double discovery_tol = 0.0,
    double drop_tol = 0.0) {
  return AutoReplayCachedLaplaceGradientContext<CombinedObjectiveFn>(
      std::move(combined_objective), theta_dim, random_dim, std::move(pattern),
      discovery_theta, discovery_uhat, discovery_tol, drop_tol);
}

} // namespace laplace
} // namespace quadra
