#ifndef QUADRA_LAPLACE_PROFILED_DELTA_METHOD_VECTOR_HPP
#define QUADRA_LAPLACE_PROFILED_DELTA_METHOD_VECTOR_HPP

#include "../inference/fixed_effect_covariance.hpp"

#include <Eigen/Dense>

#include <cmath>
#include <limits>
#include <string>

namespace quadra {

struct LaplaceProfiledDeltaMethodVectorResult {
  Eigen::VectorXd estimate_m;
  Eigen::MatrixXd jacobian_m;
  Eigen::MatrixXd covariance_m;
  Eigen::MatrixXd correlation_m;
  Eigen::VectorXd std_error_m;
  Eigen::VectorXd cv_m;

  bool success_m = false;
  std::string message_m;
};

inline LaplaceProfiledDeltaMethodVectorResult
compute_laplace_profiled_delta_method_vector(
    const Eigen::VectorXd &estimates, const Eigen::MatrixXd &g_theta,
    const Eigen::MatrixXd &g_u, const Eigen::MatrixXd &du_dtheta,
    const Eigen::MatrixXd &theta_covariance) {
  LaplaceProfiledDeltaMethodVectorResult result;
  result.estimate_m = estimates;

  const Eigen::Index n_derived = estimates.size();

  if (g_theta.rows() != n_derived) {
    result.message_m = "g_theta row count must match estimates.";
    return result;
  }

  if (g_u.rows() != n_derived) {
    result.message_m = "g_u row count must match estimates.";
    return result;
  }

  if (g_u.cols() != du_dtheta.rows()) {
    result.message_m = "g_u column count must match du_dtheta row count.";
    return result;
  }

  if (g_theta.cols() != du_dtheta.cols()) {
    result.message_m =
        "g_theta column count must match du_dtheta column count.";
    return result;
  }

  if (theta_covariance.rows() != g_theta.cols() ||
      theta_covariance.cols() != g_theta.cols()) {
    result.message_m =
        "theta covariance dimension does not match profiled Jacobian.";
    return result;
  }

  result.jacobian_m = g_theta + g_u * du_dtheta;

  if (!result.jacobian_m.allFinite()) {
    result.message_m = "profiled Jacobian contains non-finite values.";
    return result;
  }

  result.covariance_m =
      result.jacobian_m * theta_covariance * result.jacobian_m.transpose();

  result.covariance_m =
      0.5 * (result.covariance_m + result.covariance_m.transpose());

  if (!result.covariance_m.allFinite()) {
    result.message_m =
        "profiled derived covariance contains non-finite values.";
    return result;
  }

  result.std_error_m = Eigen::VectorXd::Zero(n_derived);
  result.cv_m = Eigen::VectorXd::Zero(n_derived);

  for (Eigen::Index i = 0; i < n_derived; ++i) {
    const double variance = result.covariance_m(i, i);

    if (!std::isfinite(variance) || variance < 0.0) {
      result.message_m = "profiled derived covariance diagonal contains "
                         "negative or non-finite variance.";
      return result;
    }

    result.std_error_m[i] = std::sqrt(variance);

    result.cv_m[i] = std::abs(estimates[i]) > 1.0e-12
                         ? result.std_error_m[i] / std::abs(estimates[i])
                         : std::numeric_limits<double>::quiet_NaN();
  }

  result.correlation_m = covariance_to_correlation(result.covariance_m);

  result.success_m = true;
  result.message_m =
      "Laplace profiled vector delta method computed successfully.";

  return result;
}

} // namespace quadra

#endif
