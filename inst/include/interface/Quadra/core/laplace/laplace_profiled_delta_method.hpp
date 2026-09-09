#ifndef QUADRA_LAPLACE_PROFILED_DELTA_METHOD_HPP
#define QUADRA_LAPLACE_PROFILED_DELTA_METHOD_HPP

#include "laplace_profiled_derived_gradient.hpp"

#include <Eigen/Dense>

#include <cmath>
#include <limits>
#include <string>

namespace quadra {

struct LaplaceProfiledDeltaMethodResult {
  double estimate_m = std::numeric_limits<double>::quiet_NaN();
  double variance_m = std::numeric_limits<double>::quiet_NaN();
  double std_error_m = std::numeric_limits<double>::quiet_NaN();
  double cv_m = std::numeric_limits<double>::quiet_NaN();

  Eigen::VectorXd gradient_m;

  bool success_m = false;
  std::string message_m;
};

inline LaplaceProfiledDeltaMethodResult compute_laplace_profiled_delta_method(
    double estimate, const Eigen::VectorXd &g_theta, const Eigen::VectorXd &g_u,
    const Eigen::MatrixXd &du_dtheta, const Eigen::MatrixXd &theta_covariance) {
  LaplaceProfiledDeltaMethodResult result;
  result.estimate_m = estimate;

  const auto profiled_gradient =
      compute_laplace_profiled_derived_gradient(g_theta, g_u, du_dtheta);

  if (!profiled_gradient.success_m) {
    result.message_m =
        "Profiled gradient failed: " + profiled_gradient.message_m;
    return result;
  }

  if (theta_covariance.rows() != profiled_gradient.gradient_m.size() ||
      theta_covariance.cols() != profiled_gradient.gradient_m.size()) {
    result.message_m =
        "Theta covariance dimension does not match profiled gradient.";
    return result;
  }

  result.gradient_m = profiled_gradient.gradient_m;

  result.variance_m =
      delta_variance_from_gradient(result.gradient_m, theta_covariance);

  if (!std::isfinite(result.variance_m) || result.variance_m < 0.0) {
    result.message_m =
        "Profiled delta-method variance is negative or non-finite.";
    return result;
  }

  result.std_error_m = std::sqrt(result.variance_m);

  result.cv_m = std::abs(result.estimate_m) > 1.0e-12
                    ? result.std_error_m / std::abs(result.estimate_m)
                    : std::numeric_limits<double>::quiet_NaN();

  result.success_m = true;
  result.message_m = "Laplace profiled delta method computed successfully.";

  return result;
}

} // namespace quadra

#endif
