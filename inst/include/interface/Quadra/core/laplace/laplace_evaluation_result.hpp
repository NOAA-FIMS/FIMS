#pragma once

#include <Eigen/Dense>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

#include "exact_laplace_iteration_workspace.hpp"

namespace quadra {
namespace laplace {

struct LaplaceEvaluationResult {
  double objective = 0.0;
  double joint_objective = 0.0;
  double logdet_huu = 0.0;

  Eigen::VectorXd gradient;
  Eigen::VectorXd theta;
  Eigen::VectorXd uhat;
  Eigen::VectorXd joint_envelope_gradient;
  Eigen::VectorXd logdet_gradient_contribution;

  int theta_dim = 0;
  int random_dim = 0;
  int active_direction_count = 0;
};

// Compute log det from an LDLT factorization.
//
// Assumes H is positive definite. For indefinite matrices, logdet is not
// meaningful for the Laplace approximation without additional handling.
inline double logdet_from_ldlt(const Eigen::LDLT<Eigen::MatrixXd> &ldlt) {
  if (ldlt.info() != Eigen::Success) {
    throw std::runtime_error("LDLT factorization failed.");
  }

  const auto D = ldlt.vectorD();

  for (int i = 0; i < D.size(); ++i) {
    if (!(D[i] > 0.0) || !std::isfinite(D[i])) {
      throw std::runtime_error(
          "H_uu factorization has nonpositive or nonfinite diagonal.");
    }
  }

  return D.array().log().sum();
}

// Optimizer-facing exact Laplace objective + gradient evaluation.
//
// Inputs:
//   combined_objective:
//       callable objective f(theta, u)
//
//   joint_objective_value:
//       f(theta, uhat)
//
//   joint_envelope_gradient:
//       gradient wrt theta of f(theta, uhat), relying on the envelope theorem
//       so fixed-u partials are sufficient at the mode
//
//   cross_derivative_fn:
//       callable returning f_{u theta_j} for a requested theta index
//
//   Huu:
//       random-effect Hessian H_uu(theta, uhat)
//
//   pattern:
//       sparsity pattern for H_uu/Hdot extraction
//
// Output:
//   LaplaceEvaluationResult with:
//       objective = joint + 0.5 logdet(H_uu)
//       gradient  = joint envelope gradient + exact logdet gradient
template <class CombinedObjectiveFn, class CrossDerivativeFn>
LaplaceEvaluationResult evaluate_laplace_objective_and_gradient(
    CombinedObjectiveFn combined_objective,
    CrossDerivativeFn cross_derivative_fn, int theta_dim, int random_dim,
    RandomHessianPattern random_hessian_pattern,
    std::vector<int> active_directions, const Eigen::VectorXd &theta,
    const Eigen::VectorXd &uhat, const Eigen::MatrixXd &Huu,
    double joint_objective_value,
    const Eigen::VectorXd &joint_envelope_gradient,
    ExactLaplaceIterationWorkspaceOptions options =
        ExactLaplaceIterationWorkspaceOptions{}) {
  if (theta.size() != theta_dim) {
    throw std::invalid_argument("theta has wrong length.");
  }
  if (uhat.size() != random_dim) {
    throw std::invalid_argument("uhat has wrong length.");
  }
  if (Huu.rows() != random_dim || Huu.cols() != random_dim) {
    throw std::invalid_argument("Huu has wrong dimensions.");
  }
  if (joint_envelope_gradient.size() != theta_dim) {
    throw std::invalid_argument("joint_envelope_gradient has wrong length.");
  }

  Eigen::LDLT<Eigen::MatrixXd> ldlt(Huu);
  const double logdet = logdet_from_ldlt(ldlt);

  auto workspace = make_exact_laplace_iteration_workspace(
      std::move(combined_objective), std::move(cross_derivative_fn), theta_dim,
      random_dim, std::move(random_hessian_pattern), active_directions, theta,
      uhat, Huu, options);

  const Eigen::VectorXd gradient = workspace.gradient(joint_envelope_gradient);

  LaplaceEvaluationResult out;
  out.objective = joint_objective_value + 0.5 * logdet;
  out.joint_objective = joint_objective_value;
  out.logdet_huu = logdet;
  out.gradient = gradient;
  out.theta = theta;
  out.uhat = uhat;
  out.joint_envelope_gradient = joint_envelope_gradient;
  out.logdet_gradient_contribution = workspace.logdet_gradient_contribution();
  out.theta_dim = theta_dim;
  out.random_dim = random_dim;
  out.active_direction_count =
      static_cast<int>(workspace.active_directions().size());

  return out;
}

} // namespace laplace
} // namespace quadra
