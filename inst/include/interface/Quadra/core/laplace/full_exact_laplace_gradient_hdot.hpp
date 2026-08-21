#pragma once

#include <Eigen/Dense>
#include <stdexcept>
#include <utility>

#include "full_exact_laplace_gradient_fd.hpp"

namespace quadra {
namespace laplace {

struct FullExactLaplaceGradientHdotOptions {
  bool check_dimensions = true;
};

// HdotProvider must be callable as:
//
//   Eigen::MatrixXd hdot_provider(
//       const Eigen::VectorXd& theta,
//       const Eigen::VectorXd& uhat,
//       int theta_index);
//
// and return:
//
//   d H_uu(theta, uhat_fixed) / d theta[theta_index]
//
// evaluated at fixed uhat.
//
// This is the production-facing seam. The provider can be:
//   - analytic, for tests;
//   - finite-difference, for validation;
//   - AD directional third-order;
//   - edge-pushed sparse Hdot;
//   - cached pattern-aware Hdot.
//
// The outer gradient semantics do not change.
template <class HessianUUFn, class HdotProvider>
Eigen::VectorXd
logdet_trace_gradient_hdot(const HessianUUFn &hessian_uu_at_fixed_uhat,
                           const HdotProvider &hdot_provider,
                           const Eigen::VectorXd &theta,
                           const Eigen::VectorXd &uhat,
                           const FullExactLaplaceGradientHdotOptions &options =
                               FullExactLaplaceGradientHdotOptions{}) {
  const int p = static_cast<int>(theta.size());

  const Eigen::MatrixXd H_uu = hessian_uu_at_fixed_uhat(theta, uhat);

  if (options.check_dimensions && H_uu.rows() != H_uu.cols()) {
    throw std::invalid_argument("H_uu must be square.");
  }

  Eigen::VectorXd trace_grad(p);
  trace_grad.setZero();

  for (int j = 0; j < p; ++j) {
    const Eigen::MatrixXd Hdot_j = hdot_provider(theta, uhat, j);

    if (options.check_dimensions) {
      detail::check_square_same_size(H_uu, Hdot_j, "H_uu", "Hdot_j");
    }

    trace_grad[j] = trace_hinv_hdot_dense(H_uu, Hdot_j);
  }

  return trace_grad;
}

template <class HessianUUFn, class HdotProvider>
Eigen::VectorXd full_exact_laplace_gradient_hdot(
    const Eigen::VectorXd &grad_joint_envelope,
    const HessianUUFn &hessian_uu_at_fixed_uhat,
    const HdotProvider &hdot_provider, const Eigen::VectorXd &theta,
    const Eigen::VectorXd &uhat,
    const FullExactLaplaceGradientHdotOptions &options =
        FullExactLaplaceGradientHdotOptions{}) {
  if (grad_joint_envelope.size() != theta.size()) {
    throw std::invalid_argument(
        "grad_joint_envelope and theta must have the same length.");
  }

  Eigen::VectorXd grad = grad_joint_envelope;
  grad += 0.5 * logdet_trace_gradient_hdot(hessian_uu_at_fixed_uhat,
                                           hdot_provider, theta, uhat, options);
  return grad;
}

template <class JointEnvelopeGradientFn, class HessianUUFn, class HdotProvider>
class LaplaceEvaluatorExactGradientHdotAdapter {
public:
  LaplaceEvaluatorExactGradientHdotAdapter(
      JointEnvelopeGradientFn joint_envelope_gradient,
      HessianUUFn hessian_uu_at_fixed_uhat, HdotProvider hdot_provider,
      FullExactLaplaceGradientHdotOptions options =
          FullExactLaplaceGradientHdotOptions{})
      : joint_envelope_gradient_(std::move(joint_envelope_gradient)),
        hessian_uu_at_fixed_uhat_(std::move(hessian_uu_at_fixed_uhat)),
        hdot_provider_(std::move(hdot_provider)), options_(options) {}

  Eigen::VectorXd operator()(const Eigen::VectorXd &theta,
                             const Eigen::VectorXd &uhat) const {
    const Eigen::VectorXd grad_joint = joint_envelope_gradient_(theta, uhat);

    return full_exact_laplace_gradient_hdot(
        grad_joint, hessian_uu_at_fixed_uhat_, hdot_provider_, theta, uhat,
        options_);
  }

private:
  JointEnvelopeGradientFn joint_envelope_gradient_;
  HessianUUFn hessian_uu_at_fixed_uhat_;
  HdotProvider hdot_provider_;
  FullExactLaplaceGradientHdotOptions options_;
};

template <class JointEnvelopeGradientFn, class HessianUUFn, class HdotProvider>
auto make_laplace_evaluator_exact_gradient_hdot_adapter(
    JointEnvelopeGradientFn joint_envelope_gradient,
    HessianUUFn hessian_uu_at_fixed_uhat, HdotProvider hdot_provider,
    FullExactLaplaceGradientHdotOptions options =
        FullExactLaplaceGradientHdotOptions{}) {
  return LaplaceEvaluatorExactGradientHdotAdapter<JointEnvelopeGradientFn,
                                                  HessianUUFn, HdotProvider>(
      std::move(joint_envelope_gradient), std::move(hessian_uu_at_fixed_uhat),
      std::move(hdot_provider), options);
}

} // namespace laplace
} // namespace quadra
