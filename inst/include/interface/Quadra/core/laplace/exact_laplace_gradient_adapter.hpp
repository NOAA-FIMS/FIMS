#pragma once

#include <Eigen/Dense>
#include <stdexcept>
#include <utility>

#include "exact_laplace_gradient_engine.hpp"

namespace quadra {
namespace laplace {

// Explicit input bundle for exact Laplace gradient evaluation.
//
// This is intentionally small and model-agnostic. A higher-level Model adapter
// can populate this from a solve/evaluation result.
struct ExactLaplaceGradientInputs {
  Eigen::VectorXd theta;
  Eigen::VectorXd uhat;
  Eigen::VectorXd joint_envelope_gradient;
};

// Adapter around ExactLaplaceGradientEngine.
//
// This class is the bridge point for integrating the optimized exact-gradient
// engine into broader Laplace workflows. It expects callers to supply:
//   - theta
//   - optimized/random-effect mode uhat
//   - envelope joint gradient wrt theta at fixed uhat
//   - a callable H_uu(theta, uhat)
//
// The engine supplies the exact logdet gradient contribution:
//
//   0.5 * trace(H_uu^{-1} dH_uu/dtheta_j)
//
// using sparse exact directional Hdot, replay reuse, active-direction
// discovery, and cached sparse trace contraction.
template <class CombinedObjectiveFn, class HessianUUFn>
class ExactLaplaceGradientAdapter {
public:
  ExactLaplaceGradientAdapter(CombinedObjectiveFn combined_objective,
                              HessianUUFn hessian_uu_at_fixed_uhat,
                              int theta_dim, int random_dim,
                              RandomHessianPattern random_hessian_pattern,
                              const Eigen::VectorXd &discovery_theta,
                              const Eigen::VectorXd &discovery_uhat,
                              ExactLaplaceGradientEngineOptions options =
                                  ExactLaplaceGradientEngineOptions{})
      : hessian_uu_at_fixed_uhat_(std::move(hessian_uu_at_fixed_uhat)),
        engine_(make_exact_laplace_gradient_engine(
            std::move(combined_objective), theta_dim, random_dim,
            std::move(random_hessian_pattern), discovery_theta, discovery_uhat,
            options)) {}

  Eigen::VectorXd gradient(const ExactLaplaceGradientInputs &inputs) const {
    validate_inputs(inputs);

    return engine_.gradient(inputs.joint_envelope_gradient,
                            hessian_uu_at_fixed_uhat_, inputs.theta,
                            inputs.uhat);
  }

  const std::vector<int> &active_directions() const {
    return engine_.active_directions();
  }

  const ActiveDirectionDiscoveryResult &discovery() const {
    return engine_.discovery();
  }

  int theta_dim() const { return engine_.theta_dim(); }

  int random_dim() const { return engine_.random_dim(); }

private:
  HessianUUFn hessian_uu_at_fixed_uhat_;
  ExactLaplaceGradientEngine<CombinedObjectiveFn> engine_;

  void validate_inputs(const ExactLaplaceGradientInputs &inputs) const {
    if (inputs.theta.size() != theta_dim()) {
      throw std::invalid_argument("theta has wrong length.");
    }
    if (inputs.uhat.size() != random_dim()) {
      throw std::invalid_argument("uhat has wrong length.");
    }
    if (inputs.joint_envelope_gradient.size() != theta_dim()) {
      throw std::invalid_argument("joint_envelope_gradient has wrong length.");
    }
  }
};

template <class CombinedObjectiveFn, class HessianUUFn>
auto make_exact_laplace_gradient_adapter(
    CombinedObjectiveFn combined_objective,
    HessianUUFn hessian_uu_at_fixed_uhat, int theta_dim, int random_dim,
    RandomHessianPattern random_hessian_pattern,
    const Eigen::VectorXd &discovery_theta,
    const Eigen::VectorXd &discovery_uhat,
    ExactLaplaceGradientEngineOptions options =
        ExactLaplaceGradientEngineOptions{}) {
  return ExactLaplaceGradientAdapter<CombinedObjectiveFn, HessianUUFn>(
      std::move(combined_objective), std::move(hessian_uu_at_fixed_uhat),
      theta_dim, random_dim, std::move(random_hessian_pattern), discovery_theta,
      discovery_uhat, options);
}

} // namespace laplace
} // namespace quadra
