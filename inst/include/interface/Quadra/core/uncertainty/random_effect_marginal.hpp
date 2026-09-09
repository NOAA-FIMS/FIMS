#pragma once

#include "../laplace/sparse_huu_factorization.hpp"

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

namespace quadra {

struct RandomEffectMarginalResult {
  Eigen::VectorXd conditional_variance_m;
  Eigen::VectorXd parameter_variance_m;
  Eigen::VectorXd marginal_variance_m;
  Eigen::VectorXd std_error_m;
  bool success_m = false;
  std::string message_m;
};

inline RandomEffectMarginalResult random_effect_marginal_diagonal(
    const laplace::TakahashiSelectedInverse &selected_inverse,
    const Eigen::MatrixXd &du_dtheta, const Eigen::MatrixXd &theta_covariance) {
  RandomEffectMarginalResult out;
  const Eigen::Index n_random = du_dtheta.rows();
  const Eigen::Index n_fixed = du_dtheta.cols();
  if (theta_covariance.rows() != n_fixed ||
      theta_covariance.cols() != n_fixed) {
    out.message_m =
        "Non-conformable implicit derivative and covariance inputs.";
    return out;
  }
  try {
    out.conditional_variance_m.resize(n_random);
    for (Eigen::Index i = 0; i < n_random; ++i)
      out.conditional_variance_m[i] =
          selected_inverse.value(static_cast<int>(i), static_cast<int>(i));
    const Eigen::MatrixXd propagated = du_dtheta * theta_covariance;
    out.parameter_variance_m =
        (propagated.array() * du_dtheta.array()).rowwise().sum();
    out.marginal_variance_m =
        out.conditional_variance_m + out.parameter_variance_m;
    out.std_error_m.resize(n_random);
    for (Eigen::Index i = 0; i < n_random; ++i) {
      const double variance = out.marginal_variance_m[i];
      out.std_error_m[i] = variance >= 0.0 && std::isfinite(variance)
                               ? std::sqrt(std::max(0.0, variance))
                               : std::numeric_limits<double>::quiet_NaN();
    }
    out.success_m = out.conditional_variance_m.allFinite() &&
                    out.parameter_variance_m.allFinite() &&
                    out.std_error_m.allFinite();
    out.message_m =
        out.success_m ? "Random-effect marginal uncertainty computed."
                      : "Random-effect uncertainty contains non-finite values.";
  } catch (const std::exception &e) {
    out.message_m =
        std::string("Random-effect marginal uncertainty failed: ") + e.what();
  }
  return out;
}

// First-order marginal uncertainty for optimized random effects:
//
// Var(u_hat) ~= H_uu^-1 +
//   (du_hat / dtheta) Var(theta) (du_hat / dtheta)'.
//
// Only the diagonal is materialized. Conditional diagonal entries are obtained
// from the sparse Takahashi selected inverse, and fixed-parameter propagation
// uses row-wise contractions without constructing a dense random-effect
// covariance matrix.
inline RandomEffectMarginalResult
random_effect_marginal_diagonal(const Eigen::SparseMatrix<double> &H_uu,
                                const Eigen::MatrixXd &du_dtheta,
                                const Eigen::MatrixXd &theta_covariance) {
  RandomEffectMarginalResult out;
  const Eigen::Index n_random = H_uu.rows();
  const Eigen::Index n_fixed = du_dtheta.cols();

  if (H_uu.cols() != n_random || du_dtheta.rows() != n_random ||
      theta_covariance.rows() != n_fixed ||
      theta_covariance.cols() != n_fixed) {
    out.message_m =
        "Non-conformable Hessian, implicit derivative, and covariance inputs.";
    return out;
  }

  try {
    laplace::SparseHuuFactorization factorization(H_uu);
    const auto selected_inverse = factorization.selected_inverse();
    out.conditional_variance_m.resize(n_random);
    for (Eigen::Index i = 0; i < n_random; ++i)
      out.conditional_variance_m[i] =
          selected_inverse.value(static_cast<int>(i), static_cast<int>(i));

    const Eigen::MatrixXd propagated = du_dtheta * theta_covariance;
    out.parameter_variance_m =
        (propagated.array() * du_dtheta.array()).rowwise().sum();
    out.marginal_variance_m =
        out.conditional_variance_m + out.parameter_variance_m;
    out.std_error_m.resize(n_random);
    for (Eigen::Index i = 0; i < n_random; ++i) {
      const double variance = out.marginal_variance_m[i];
      out.std_error_m[i] = variance >= 0.0 && std::isfinite(variance)
                               ? std::sqrt(std::max(0.0, variance))
                               : std::numeric_limits<double>::quiet_NaN();
    }
    out.success_m = out.conditional_variance_m.allFinite() &&
                    out.parameter_variance_m.allFinite() &&
                    out.std_error_m.allFinite();
    out.message_m =
        out.success_m ? "Random-effect marginal uncertainty computed."
                      : "Random-effect uncertainty contains non-finite values.";
  } catch (const std::exception &e) {
    out.message_m =
        std::string("Random-effect marginal uncertainty failed: ") + e.what();
  }
  return out;
}

} // namespace quadra
