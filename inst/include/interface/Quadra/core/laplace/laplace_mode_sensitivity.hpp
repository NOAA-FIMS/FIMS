#ifndef QUADRA_LAPLACE_MODE_SENSITIVITY_HPP
#define QUADRA_LAPLACE_MODE_SENSITIVITY_HPP

#include "../inference/ift_mode_sensitivity.hpp"

#include <Eigen/Dense>

#include <string>

namespace quadra {

struct LaplaceModeSensitivityResult {
  Eigen::MatrixXd du_dtheta_m;

  bool success_m = false;
  std::string message_m;
};

inline LaplaceModeSensitivityResult
solve_dense_laplace_mode_sensitivity(const Eigen::MatrixXd &H_uu,
                                     const Eigen::MatrixXd &H_u_theta) {
  LaplaceModeSensitivityResult result;

  const auto ift = solve_dense_ift_mode_sensitivity(H_uu, H_u_theta);

  if (!ift.success_m) {
    result.success_m = false;
    result.message_m = "Laplace mode sensitivity failed: " + ift.message_m;
    return result;
  }

  result.du_dtheta_m = ift.du_dtheta_m;
  result.success_m = true;
  result.message_m = "Dense Laplace mode sensitivity solved successfully.";

  return result;
}

} // namespace quadra

#endif
