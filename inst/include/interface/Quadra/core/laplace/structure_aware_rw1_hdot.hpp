#ifndef QUADRA_LAPLACE_STRUCTURE_AWARE_RW1_HDOT_HPP
#define QUADRA_LAPLACE_STRUCTURE_AWARE_RW1_HDOT_HPP

#include <Eigen/Dense>

#include <cmath>
#include <stdexcept>
#include <utility>

#include "sparse_huu_factorization.hpp"

namespace quadra {
namespace laplace {

struct StructureAwareRw1HdotTraceResult {
  Eigen::VectorXd trace_terms;
  Eigen::VectorXd gradient;
  double objective = 0.0;
};

template <class FuThetaColumnProvider>
Eigen::VectorXd
rw1_total_u_direction(const Eigen::VectorXd &theta, const Eigen::VectorXd &uhat,
                      const int theta_index, SparseHuuFactorization &factor,
                      FuThetaColumnProvider &&f_u_theta_column) {
  (void)theta;

  if (theta_index < 0) {
    throw std::invalid_argument("rw1_total_u_direction: theta_index < 0");
  }

  return -factor.solve(f_u_theta_column(theta, uhat, theta_index));
}

template <class SelectedInverseAccessor, class FuThetaColumnProvider>
Eigen::VectorXd rw1_structure_aware_hdot_traces(
    const int m, const int n_directions, const Eigen::VectorXd &theta,
    const Eigen::VectorXd &uhat, SelectedInverseAccessor &&selected_inverse,
    SparseHuuFactorization &factor, FuThetaColumnProvider &&f_u_theta_column) {
  if (m < 0) {
    throw std::invalid_argument("rw1_structure_aware_hdot_traces: m < 0");
  }
  if (n_directions < 0) {
    throw std::invalid_argument(
        "rw1_structure_aware_hdot_traces: n_directions < 0");
  }
  if (theta.size() < 5) {
    throw std::invalid_argument(
        "rw1_structure_aware_hdot_traces: theta must have at least 5 values");
  }
  if (uhat.size() != m) {
    throw std::invalid_argument(
        "rw1_structure_aware_hdot_traces: uhat size does not match m");
  }

  const double lambda0 = std::exp(theta[2]);
  const double beta = std::exp(theta[4]);

  Eigen::VectorXd traces = Eigen::VectorXd::Zero(n_directions);

  for (int k = 0; k < n_directions; ++k) {
    const int theta_index = k % 5;

    Eigen::VectorXd theta_direction = Eigen::VectorXd::Zero(5);
    theta_direction[theta_index] = 1.0;

    const Eigen::VectorXd u_direction = rw1_total_u_direction(
        theta, uhat, theta_index, factor,
        std::forward<FuThetaColumnProvider>(f_u_theta_column));

    double trace = 0.0;

    for (int i = 0; i < m; ++i) {
      double hdot_diag = 0.0;

      // Direct log_lambda0 contribution.
      if (theta_direction[2] != 0.0) {
        hdot_diag += lambda0 * theta_direction[2];
      }

      // Total direction contribution through beta * exp(u_i).
      hdot_diag +=
          beta * std::exp(uhat[i]) * (theta_direction[4] + u_direction[i]);

      trace += selected_inverse(i, i) * hdot_diag;

      if (i > 0) {
        // Current validated HAD Hdot convention has no direct RW off-diagonal
        // term in this dense-slot comparison.
        const double hdot_subdiag = 0.0;
        trace += 2.0 * selected_inverse(i, i - 1) * hdot_subdiag;
      }
    }

    traces[k] = trace;
  }

  return traces;
}

template <class SelectedInverseAccessor, class FuThetaColumnProvider>
StructureAwareRw1HdotTraceResult rw1_structure_aware_exact_gradient(
    const int m, const int n_directions, const Eigen::VectorXd &theta,
    const Eigen::VectorXd &uhat, SelectedInverseAccessor &&selected_inverse,
    const Eigen::VectorXd &joint_gradient, const double joint_objective,
    const double logdet_huu, SparseHuuFactorization &factor,
    FuThetaColumnProvider &&f_u_theta_column) {
  StructureAwareRw1HdotTraceResult out;
  out.trace_terms = rw1_structure_aware_hdot_traces(
      m, n_directions, theta, uhat,
      std::forward<SelectedInverseAccessor>(selected_inverse), factor,
      std::forward<FuThetaColumnProvider>(f_u_theta_column));

  out.objective = joint_objective + 0.5 * logdet_huu;
  out.gradient = joint_gradient + 0.5 * out.trace_terms;
  return out;
}

} // namespace laplace
} // namespace quadra

#endif // QUADRA_LAPLACE_STRUCTURE_AWARE_RW1_HDOT_HPP
