#ifndef QUADRA_LAPLACE_PROFILED_DERIVED_GRADIENT_HPP
#define QUADRA_LAPLACE_PROFILED_DERIVED_GRADIENT_HPP

#include <Eigen/Dense>

#include <string>

namespace quadra {

struct LaplaceProfiledDerivedGradientResult {
  Eigen::VectorXd gradient_m;

  bool success_m = false;
  std::string message_m;
};

inline LaplaceProfiledDerivedGradientResult
compute_laplace_profiled_derived_gradient(const Eigen::VectorXd &g_theta,
                                          const Eigen::VectorXd &g_u,
                                          const Eigen::MatrixXd &du_dtheta) {
  LaplaceProfiledDerivedGradientResult result;

  if (g_u.size() != du_dtheta.rows()) {
    result.message_m = "g_u size must match du_dtheta row count.";
    return result;
  }

  if (g_theta.size() != du_dtheta.cols()) {
    result.message_m = "g_theta size must match du_dtheta column count.";
    return result;
  }

  result.gradient_m = g_theta + (g_u.transpose() * du_dtheta).transpose();

  if (!result.gradient_m.allFinite()) {
    result.message_m = "profiled derived gradient contains non-finite values.";
    return result;
  }

  result.success_m = true;
  result.message_m = "profiled derived gradient computed successfully.";
  return result;
}

inline double delta_variance_from_gradient(const Eigen::VectorXd &gradient,
                                           const Eigen::MatrixXd &covariance) {
  return (gradient.transpose() * covariance * gradient)(0, 0);
}

} // namespace quadra

#endif
