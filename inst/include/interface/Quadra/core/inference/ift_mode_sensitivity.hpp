#ifndef QUADRA_IFT_MODE_SENSITIVITY_HPP
#define QUADRA_IFT_MODE_SENSITIVITY_HPP

#include <Eigen/Dense>

#include <cmath>
#include <limits>
#include <string>

namespace quadra {

struct IFTModeSensitivityResult {
  Eigen::MatrixXd du_dtheta_m;

  bool success_m = false;
  std::string message_m;
};

inline IFTModeSensitivityResult
solve_dense_ift_mode_sensitivity(const Eigen::MatrixXd &H_uu,
                                 const Eigen::MatrixXd &H_u_theta) {
  IFTModeSensitivityResult result;

  if (H_uu.rows() != H_uu.cols()) {
    result.message_m = "H_uu must be square.";
    return result;
  }

  if (H_u_theta.rows() != H_uu.rows()) {
    result.message_m = "H_u_theta row count must match H_uu.";
    return result;
  }

  Eigen::LDLT<Eigen::MatrixXd> ldlt(H_uu);

  if (ldlt.info() != Eigen::Success) {
    result.message_m = "LDLT factorization of H_uu failed.";
    return result;
  }

  if (!ldlt.isPositive()) {
    result.message_m = "H_uu is not positive definite.";
    return result;
  }

  result.du_dtheta_m = -ldlt.solve(H_u_theta);

  if (!result.du_dtheta_m.allFinite()) {
    result.message_m = "IFT sensitivity contains non-finite values.";
    return result;
  }

  result.success_m = true;
  result.message_m = "Dense IFT mode sensitivity solved successfully.";
  return result;
}

} // namespace quadra

#endif
