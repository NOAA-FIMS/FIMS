#pragma once

#include <Eigen/Dense>
#include <utility>
#include <vector>

#include "laplace_evaluator_exact_gradient_bridge.hpp"

namespace quadra {
namespace laplace {

// Main evaluator-facing exact-gradient input bundle.
//
// This mirrors the information naturally available after a Laplace evaluator
// solves for the random-effect mode:
//
//   theta
//   uhat
//   H_uu(theta, uhat)
//   f(theta, uhat)
//   fixed-u/envelope gradient wrt theta
//
// The cross-derivative callback is held by the bridge because it is normally
// model/evaluator specific.
struct LaplaceExactGradientEvaluationInputs {
  Eigen::VectorXd theta;
  Eigen::VectorXd uhat;
  Eigen::MatrixXd Huu;
  double joint_objective = 0.0;
  Eigen::VectorXd joint_envelope_gradient;
};

// Canonical evaluator-facing exact-gradient function.
//
// This is the intended integration target for the main LaplaceEvaluator:
// after mode solve and Hessian construction, populate the input bundle and
// call this function to obtain objective, exact gradient, and diagnostics.
template <class Bridge>
LaplaceEvaluationResult evaluate_laplace_with_exact_gradient(
    const Bridge &bridge, const LaplaceExactGradientEvaluationInputs &inputs) {
  return bridge.evaluate(inputs.theta, inputs.uhat, inputs.Huu,
                         inputs.joint_objective,
                         inputs.joint_envelope_gradient);
}

// Convenience factory + evaluation in one call.
//
// CombinedObjectiveFn:
//   objective f(theta, u), x = [theta, u]
//
// CrossDerivativeFn:
//   callable returning f_{u theta_j}
//
// This helper is useful for existing evaluator code that does not want to own
// a persistent bridge object yet.
template <class CombinedObjectiveFn, class CrossDerivativeFn>
LaplaceEvaluationResult evaluate_laplace_with_exact_gradient(
    CombinedObjectiveFn combined_objective,
    CrossDerivativeFn cross_derivative_fn, int theta_dim, int random_dim,
    RandomHessianPattern random_hessian_pattern,
    std::vector<int> active_directions,
    const LaplaceExactGradientEvaluationInputs &inputs,
    LaplaceEvaluatorExactGradientBridgeOptions options =
        LaplaceEvaluatorExactGradientBridgeOptions{}) {
  auto bridge = make_laplace_evaluator_exact_gradient_bridge(
      std::move(combined_objective), std::move(cross_derivative_fn), theta_dim,
      random_dim, std::move(random_hessian_pattern),
      std::move(active_directions), options);

  return evaluate_laplace_with_exact_gradient(bridge, inputs);
}

} // namespace laplace
} // namespace quadra
