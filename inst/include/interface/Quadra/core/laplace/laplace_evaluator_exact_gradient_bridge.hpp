#pragma once

#include <Eigen/Dense>
#include <utility>
#include <vector>

#include "laplace_evaluation_result.hpp"

namespace quadra {
namespace laplace {

// Options for the evaluator bridge.
//
// This is deliberately thin for now. More policy can be added here as the
// bridge is wired into the full LaplaceEvaluator implementation.
struct LaplaceEvaluatorExactGradientBridgeOptions {
  ExactLaplaceIterationWorkspaceOptions workspace_options;
};

// Bridge object for existing LaplaceEvaluator-style workflows.
//
// The intent is to make the new exact-gradient engine usable from current
// evaluator code without requiring a large invasive rewrite.
//
// CombinedObjectiveFn:
//   model/objective f(theta, u), called with x = [theta, u]
//
// CrossDerivativeFn:
//   returns f_{u theta_j} at the current theta/uhat
//
// The bridge assumes the caller has already solved for uhat and can provide:
//   - joint objective value
//   - joint envelope gradient
//   - H_uu(theta, uhat)
//
// That matches the natural boundary of a Laplace evaluator after the random
// effects mode solve.
template <class CombinedObjectiveFn, class CrossDerivativeFn>
class LaplaceEvaluatorExactGradientBridge {
public:
  LaplaceEvaluatorExactGradientBridge(
      CombinedObjectiveFn combined_objective,
      CrossDerivativeFn cross_derivative_fn, int theta_dim, int random_dim,
      RandomHessianPattern random_hessian_pattern,
      std::vector<int> active_directions,
      LaplaceEvaluatorExactGradientBridgeOptions options =
          LaplaceEvaluatorExactGradientBridgeOptions{})
      : combined_objective_(std::move(combined_objective)),
        cross_derivative_fn_(std::move(cross_derivative_fn)),
        theta_dim_(theta_dim), random_dim_(random_dim),
        pattern_(std::move(random_hessian_pattern)),
        active_directions_(std::move(active_directions)), options_(options) {}

  LaplaceEvaluationResult
  evaluate(const Eigen::VectorXd &theta, const Eigen::VectorXd &uhat,
           const Eigen::MatrixXd &Huu, double joint_objective_value,
           const Eigen::VectorXd &joint_envelope_gradient) const {
    return evaluate_laplace_objective_and_gradient(
        combined_objective_, cross_derivative_fn_, theta_dim_, random_dim_,
        pattern_, active_directions_, theta, uhat, Huu, joint_objective_value,
        joint_envelope_gradient, options_.workspace_options);
  }

  int theta_dim() const { return theta_dim_; }

  int random_dim() const { return random_dim_; }

  const RandomHessianPattern &pattern() const { return pattern_; }

  const std::vector<int> &active_directions() const {
    return active_directions_;
  }

private:
  CombinedObjectiveFn combined_objective_;
  CrossDerivativeFn cross_derivative_fn_;
  int theta_dim_;
  int random_dim_;
  RandomHessianPattern pattern_;
  std::vector<int> active_directions_;
  LaplaceEvaluatorExactGradientBridgeOptions options_;
};

template <class CombinedObjectiveFn, class CrossDerivativeFn>
auto make_laplace_evaluator_exact_gradient_bridge(
    CombinedObjectiveFn combined_objective,
    CrossDerivativeFn cross_derivative_fn, int theta_dim, int random_dim,
    RandomHessianPattern random_hessian_pattern,
    std::vector<int> active_directions,
    LaplaceEvaluatorExactGradientBridgeOptions options =
        LaplaceEvaluatorExactGradientBridgeOptions{}) {
  return LaplaceEvaluatorExactGradientBridge<CombinedObjectiveFn,
                                             CrossDerivativeFn>(
      std::move(combined_objective), std::move(cross_derivative_fn), theta_dim,
      random_dim, std::move(random_hessian_pattern),
      std::move(active_directions), options);
}

} // namespace laplace
} // namespace quadra
