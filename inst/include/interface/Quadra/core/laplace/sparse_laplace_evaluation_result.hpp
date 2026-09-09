#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <utility>
#include <vector>

#include "laplace_evaluation_result.hpp"
#include "sparse_exact_laplace_iteration_workspace.hpp"

namespace quadra {
namespace laplace {

struct SparseLaplaceExactGradientEvaluationInputs {
  Eigen::VectorXd theta;
  Eigen::VectorXd uhat;
  Eigen::SparseMatrix<double> Huu;
  double joint_objective = 0.0;
  Eigen::VectorXd joint_envelope_gradient;
};

template <class CombinedObjectiveFn, class CrossDerivativeFn>
LaplaceEvaluationResult evaluate_sparse_laplace_with_exact_gradient(
    CombinedObjectiveFn combined_objective,
    CrossDerivativeFn cross_derivative_fn, int theta_dim, int random_dim,
    RandomHessianPattern random_hessian_pattern,
    std::vector<int> active_directions,
    const SparseLaplaceExactGradientEvaluationInputs &inputs,
    SparseExactLaplaceIterationWorkspaceOptions options =
        SparseExactLaplaceIterationWorkspaceOptions{}) {
  auto workspace = make_sparse_exact_laplace_iteration_workspace(
      std::move(combined_objective), std::move(cross_derivative_fn), theta_dim,
      random_dim, std::move(random_hessian_pattern),
      std::move(active_directions), inputs.theta, inputs.uhat, inputs.Huu,
      options);

  const Eigen::VectorXd gradient =
      workspace.gradient(inputs.joint_envelope_gradient);

  LaplaceEvaluationResult out;
  out.objective = inputs.joint_objective + 0.5 * workspace.logdet_huu();
  out.joint_objective = inputs.joint_objective;
  out.logdet_huu = workspace.logdet_huu();
  out.gradient = gradient;
  out.theta = inputs.theta;
  out.uhat = inputs.uhat;
  out.joint_envelope_gradient = inputs.joint_envelope_gradient;
  out.logdet_gradient_contribution = workspace.logdet_gradient_contribution();
  out.theta_dim = theta_dim;
  out.random_dim = random_dim;
  out.active_direction_count =
      static_cast<int>(workspace.active_directions().size());

  return out;
}

} // namespace laplace
} // namespace quadra
