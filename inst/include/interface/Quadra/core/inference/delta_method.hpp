#ifndef QUADRA_DELTA_METHOD_HPP
#define QUADRA_DELTA_METHOD_HPP

#include <Eigen/Dense>

#include <cmath>
#include <limits>
#include <string>
#include <vector>

namespace quadra {

struct DeltaMethodResult {
  double estimate_m = std::numeric_limits<double>::quiet_NaN();
  double variance_m = std::numeric_limits<double>::quiet_NaN();
  double std_error_m = std::numeric_limits<double>::quiet_NaN();
  double lower95_m = std::numeric_limits<double>::quiet_NaN();
  double upper95_m = std::numeric_limits<double>::quiet_NaN();

  Eigen::VectorXd gradient_m;

  bool success_m = false;
  std::string message_m;
};

template <typename Function>
DeltaMethodResult
delta_method_scalar(Function &&f, const std::vector<double> &theta_hat,
                    const Eigen::MatrixXd &covariance, double step = 1.0e-5) {
  DeltaMethodResult result;

  const Eigen::Index n = static_cast<Eigen::Index>(theta_hat.size());

  result.gradient_m = Eigen::VectorXd::Zero(n);

  if (n == 0) {
    result.success_m = false;
    result.message_m = "No parameters supplied.";
    return result;
  }

  if (covariance.rows() != n || covariance.cols() != n) {
    result.success_m = false;
    result.message_m = "Covariance dimension does not match parameter vector.";
    return result;
  }

  if (!(step > 0.0) || !std::isfinite(step)) {
    result.success_m = false;
    result.message_m = "Finite-difference step must be positive and finite.";
    return result;
  }

  result.estimate_m = f(theta_hat);

  if (!std::isfinite(result.estimate_m)) {
    result.success_m = false;
    result.message_m = "Derived quantity is non-finite at theta_hat.";
    return result;
  }

  for (Eigen::Index i = 0; i < n; ++i) {
    std::vector<double> xp = theta_hat;
    std::vector<double> xm = theta_hat;

    xp[static_cast<size_t>(i)] += step;
    xm[static_cast<size_t>(i)] -= step;

    const double fp = f(xp);
    const double fm = f(xm);

    if (!std::isfinite(fp) || !std::isfinite(fm)) {
      result.success_m = false;
      result.message_m =
          "Derived quantity became non-finite during finite differencing.";
      return result;
    }

    result.gradient_m[i] = (fp - fm) / (2.0 * step);
  }

  const double variance =
      (result.gradient_m.transpose() * covariance * result.gradient_m)(0, 0);

  if (!std::isfinite(variance) || variance < 0.0) {
    result.success_m = false;
    result.message_m = "Delta-method variance is negative or non-finite.";
    result.variance_m = variance;
    return result;
  }

  result.variance_m = variance;
  result.std_error_m = std::sqrt(variance);
  result.lower95_m = result.estimate_m - 1.96 * result.std_error_m;
  result.upper95_m = result.estimate_m + 1.96 * result.std_error_m;

  result.success_m = true;
  result.message_m = "Delta-method scalar uncertainty estimated successfully.";

  return result;
}

} // namespace quadra

#endif
