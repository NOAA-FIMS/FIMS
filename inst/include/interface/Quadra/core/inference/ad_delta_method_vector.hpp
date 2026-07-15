#ifndef QUADRA_AD_DELTA_METHOD_VECTOR_HPP
#define QUADRA_AD_DELTA_METHOD_VECTOR_HPP

#include <Eigen/Dense>

#include "ad_delta_method.hpp"
#include "fixed_effect_covariance.hpp"

#include <cmath>
#include <limits>
#include <string>
#include <vector>

namespace quadra {

struct ADDeltaMethodVectorResult {
  Eigen::VectorXd estimate_m;
  Eigen::MatrixXd jacobian_m;
  Eigen::MatrixXd covariance_m;
  Eigen::VectorXd std_error_m;
  Eigen::MatrixXd correlation_m;

  bool success_m = false;
  std::string message_m;
};

template <typename Functor>
ADDeltaMethodVectorResult
ad_delta_method_vector(Functor &&f, const std::vector<double> &theta_hat,
                       const Eigen::MatrixXd &theta_covariance) {
  ADDeltaMethodVectorResult result;

  const Eigen::Index n_theta = static_cast<Eigen::Index>(theta_hat.size());

  if (n_theta == 0) {
    result.message_m = "No parameters supplied.";
    return result;
  }

  if (theta_covariance.rows() != n_theta ||
      theta_covariance.cols() != n_theta) {
    result.message_m =
        "Theta covariance dimension does not match parameter vector.";
    return result;
  }

  try {
    had::ADGraph graph;
    had::g_ADGraph = &graph;

    std::vector<quadra::AD> theta_ad;
    theta_ad.reserve(theta_hat.size());

    for (double x : theta_hat) {
      theta_ad.emplace_back(x);
    }

    const std::vector<quadra::AD> y = f(theta_ad);
    const Eigen::Index n_out = static_cast<Eigen::Index>(y.size());

    if (n_out == 0) {
      result.message_m = "Derived function returned no outputs.";
      return result;
    }

    result.estimate_m = Eigen::VectorXd::Zero(n_out);
    result.jacobian_m = Eigen::MatrixXd::Zero(n_out, n_theta);

    for (Eigen::Index k = 0; k < n_out; ++k) {
      result.estimate_m[k] = value_of(y[static_cast<std::size_t>(k)]);

      if (!std::isfinite(result.estimate_m[k])) {
        result.message_m = "Derived output is non-finite at theta_hat.";
        return result;
      }

      had::ZeroAdjoints(graph);
      had::SetAdjoint(y[static_cast<std::size_t>(k)], 1.0);
      had::PropagateAdjoint();

      for (Eigen::Index j = 0; j < n_theta; ++j) {
        result.jacobian_m(k, j) =
            had::GetAdjoint(theta_ad[static_cast<std::size_t>(j)]);
      }
    }

    result.covariance_m =
        result.jacobian_m * theta_covariance * result.jacobian_m.transpose();

    result.covariance_m =
        0.5 * (result.covariance_m + result.covariance_m.transpose());

    result.std_error_m = Eigen::VectorXd::Zero(n_out);

    for (Eigen::Index k = 0; k < n_out; ++k) {
      const double variance = result.covariance_m(k, k);

      if (!std::isfinite(variance) || variance < 0.0) {
        result.message_m = "Derived covariance diagonal contains negative or "
                           "non-finite variance.";
        return result;
      }

      result.std_error_m[k] = std::sqrt(variance);
    }

    result.correlation_m = covariance_to_correlation(result.covariance_m);

    result.success_m = true;
    result.message_m =
        "AD vector delta-method uncertainty estimated successfully.";
    return result;
  } catch (const std::exception &e) {
    result.message_m =
        std::string("AD vector delta-method failed: ") + e.what();
    return result;
  } catch (...) {
    result.message_m = "AD vector delta-method failed with unknown exception.";
    return result;
  }
}

} // namespace quadra

#endif
