#ifndef QUADRA_LAPLACE_DERIVED_GRADIENT_HPP
#define QUADRA_LAPLACE_DERIVED_GRADIENT_HPP

#include <Eigen/Dense>

#include <string>

namespace quadra {

struct LaplaceDerivedGradientResult {
  Eigen::VectorXd gradient_m;

  bool success_m = false;
  std::string message_m;
};

inline LaplaceDerivedGradientResult
compute_laplace_derived_gradient(const Eigen::VectorXd &g_theta,
                                 const Eigen::VectorXd &g_u,
                                 const Eigen::MatrixXd &du_dtheta) {
  LaplaceDerivedGradientResult result;

  if (g_u.size() != du_dtheta.rows()) {
    result.message_m = "g_u size must match du_dtheta row count.";
    return result;
  }

  if (g_theta.size() != du_dtheta.cols()) {
    result.message_m = "g_theta size must match du_dtheta column count.";
    return result;
  }

  result.gradient_m = g_theta + g_u.transpose() * du_dtheta;

  if (!result.gradient_m.allFinite()) {
    result.message_m = "Laplace derived gradient contains non-finite values.";
    return result;
  }

  result.success_m = true;
  result.message_m = "Laplace derived gradient computed successfully.";

  return result;
}

} // namespace quadra

#endif
