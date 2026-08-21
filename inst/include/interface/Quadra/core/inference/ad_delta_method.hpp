#ifndef QUADRA_AD_DELTA_METHOD_HPP
#define QUADRA_AD_DELTA_METHOD_HPP

#include <Eigen/Dense>

#include "../autodiff.hpp"

#include <cmath>
#include <limits>
#include <string>
#include <vector>

namespace quadra {

struct ADDeltaMethodResult {
  double estimate_m = std::numeric_limits<double>::quiet_NaN();
  double variance_m = std::numeric_limits<double>::quiet_NaN();
  double std_error_m = std::numeric_limits<double>::quiet_NaN();
  double lower95_m = std::numeric_limits<double>::quiet_NaN();
  double upper95_m = std::numeric_limits<double>::quiet_NaN();

  Eigen::VectorXd gradient_m;

  bool success_m = false;
  std::string message_m;
};

template <typename Functor>
ADDeltaMethodResult ad_delta_method_scalar(Functor &&f,
                                           const std::vector<double> &theta_hat,
                                           const Eigen::MatrixXd &covariance) {
  ADDeltaMethodResult result;

  const Eigen::Index n = static_cast<Eigen::Index>(theta_hat.size());
  result.gradient_m = Eigen::VectorXd::Zero(n);

  if (n == 0) {
    result.message_m = "No parameters supplied.";
    return result;
  }

  if (covariance.rows() != n || covariance.cols() != n) {
    result.message_m = "Covariance dimension does not match parameter vector.";
    return result;
  }

  try {
    had::ADGraph graph;

    std::vector<quadra::AD> theta_ad;
    theta_ad.reserve(theta_hat.size());

    for (double x : theta_hat) {
      theta_ad.emplace_back(x);
    }

    quadra::AD y = f(theta_ad);

    result.estimate_m = value_of(y);

    if (!std::isfinite(result.estimate_m)) {
      result.message_m = "Derived quantity is non-finite at theta_hat.";
      return result;
    }

    had::g_ADGraph = &graph;
    had::ZeroAdjoints(graph);
    had::SetAdjoint(y, 1.0);
    had::PropagateAdjoint();

    for (Eigen::Index i = 0; i < n; ++i) {
      result.gradient_m[i] =
          had::GetAdjoint(theta_ad[static_cast<std::size_t>(i)]);
    }

    const double variance =
        (result.gradient_m.transpose() * covariance * result.gradient_m)(0, 0);

    result.variance_m = variance;

    if (!std::isfinite(variance) || variance < 0.0) {
      result.message_m = "AD delta-method variance is negative or non-finite.";
      return result;
    }

    result.std_error_m = std::sqrt(variance);
    result.lower95_m = result.estimate_m - 1.96 * result.std_error_m;
    result.upper95_m = result.estimate_m + 1.96 * result.std_error_m;

    result.success_m = true;
    result.message_m =
        "AD delta-method scalar uncertainty estimated successfully.";
    return result;
  } catch (const std::exception &e) {
    result.success_m = false;
    result.message_m = std::string("AD delta-method failed: ") + e.what();
    return result;
  } catch (...) {
    result.success_m = false;
    result.message_m = "AD delta-method failed with unknown exception.";
    return result;
  }
}

} // namespace quadra

#endif
