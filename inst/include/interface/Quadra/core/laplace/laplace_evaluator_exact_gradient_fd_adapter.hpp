#pragma once

#include <Eigen/Dense>
#include <functional>
#include <stdexcept>
#include <utility>

#include "full_exact_laplace_gradient_fd.hpp"

namespace quadra {
namespace laplace {

// A small adapter for wiring the validated FD trace term into whatever
// LaplaceEvaluator interface is currently active in Quadra.
//
// This intentionally uses callables instead of depending directly on a specific
// LaplaceEvaluator class shape. That keeps this file stable while the evaluator
// internals are still moving.
//
// Required callables:
//
//   JointEnvelopeGradientFn:
//     Eigen::VectorXd operator()(const Eigen::VectorXd& theta,
//                                const Eigen::VectorXd& uhat) const;
//
//   HessianUUAtFixedUhatFn:
//     Eigen::MatrixXd operator()(const Eigen::VectorXd& theta,
//                                const Eigen::VectorXd& uhat) const;
//
// The Hessian callable MUST NOT re-optimize uhat. It must evaluate H_uu at the
// supplied theta and the supplied, already-optimized uhat.
template <class JointEnvelopeGradientFn, class HessianUUAtFixedUhatFn>
class LaplaceEvaluatorExactGradientFDAdapter {
public:
  LaplaceEvaluatorExactGradientFDAdapter(
      JointEnvelopeGradientFn joint_envelope_gradient,
      HessianUUAtFixedUhatFn hessian_uu_at_fixed_uhat,
      FullExactLaplaceGradientFDOptions options =
          FullExactLaplaceGradientFDOptions{})
      : joint_envelope_gradient_(std::move(joint_envelope_gradient)),
        hessian_uu_at_fixed_uhat_(std::move(hessian_uu_at_fixed_uhat)),
        options_(options) {}

  Eigen::VectorXd operator()(const Eigen::VectorXd &theta,
                             const Eigen::VectorXd &uhat) const {
    const Eigen::VectorXd grad_joint = joint_envelope_gradient_(theta, uhat);

    if (grad_joint.size() != theta.size()) {
      throw std::invalid_argument(
          "joint envelope gradient length must match theta length.");
    }

    auto hessian_theta_only = [&](const Eigen::VectorXd &theta_perturbed) {
      return hessian_uu_at_fixed_uhat_(theta_perturbed, uhat);
    };

    return full_exact_laplace_gradient_fd(grad_joint, hessian_theta_only, theta,
                                          options_);
  }

  const FullExactLaplaceGradientFDOptions &options() const { return options_; }

private:
  JointEnvelopeGradientFn joint_envelope_gradient_;
  HessianUUAtFixedUhatFn hessian_uu_at_fixed_uhat_;
  FullExactLaplaceGradientFDOptions options_;
};

template <class JointEnvelopeGradientFn, class HessianUUAtFixedUhatFn>
auto make_laplace_evaluator_exact_gradient_fd_adapter(
    JointEnvelopeGradientFn joint_envelope_gradient,
    HessianUUAtFixedUhatFn hessian_uu_at_fixed_uhat,
    FullExactLaplaceGradientFDOptions options =
        FullExactLaplaceGradientFDOptions{}) {
  return LaplaceEvaluatorExactGradientFDAdapter<JointEnvelopeGradientFn,
                                                HessianUUAtFixedUhatFn>(
      std::move(joint_envelope_gradient), std::move(hessian_uu_at_fixed_uhat),
      options);
}

} // namespace laplace
} // namespace quadra
