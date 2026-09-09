#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <cmath>
#include <functional>
#include <stdexcept>
#include <string>

namespace quadra {
namespace laplace {

struct FullExactLaplaceGradientFDOptions {
  // Central-difference step for dH_uu / dtheta_j.
  double step = 1.0e-5;

  // If true, use relative scaling:
  //   h_j = step * max(1, abs(theta_j))
  bool relative_step = true;

  // Numerical guard used when checking matrix dimensions.
  bool check_dimensions = true;
};

namespace detail {

inline double theta_step(double theta_j,
                         const FullExactLaplaceGradientFDOptions &options) {
  if (!(options.step > 0.0) || !std::isfinite(options.step)) {
    throw std::invalid_argument(
        "FullExactLaplaceGradientFDOptions::step must be positive and finite.");
  }

  if (!options.relative_step) {
    return options.step;
  }

  return options.step * std::max(1.0, std::abs(theta_j));
}

inline void check_square_same_size(const Eigen::MatrixXd &A,
                                   const Eigen::MatrixXd &B,
                                   const std::string &name_a,
                                   const std::string &name_b) {
  if (A.rows() != A.cols()) {
    throw std::invalid_argument(name_a + " must be square.");
  }
  if (B.rows() != B.cols()) {
    throw std::invalid_argument(name_b + " must be square.");
  }
  if (A.rows() != B.rows()) {
    throw std::invalid_argument(name_a + " and " + name_b +
                                " must have the same dimensions.");
  }
}

} // namespace detail

// Computes
//
//   trace(H_uu^{-1} Hdot)
//
// using a dense solve.
//
// This is deliberately simple and validation-oriented. For production speed,
// replace this with sparse-factorization reuse and sparse trace estimators/
// exact sparse contractions where appropriate.
inline double trace_hinv_hdot_dense(const Eigen::MatrixXd &H_uu,
                                    const Eigen::MatrixXd &Hdot) {
  detail::check_square_same_size(H_uu, Hdot, "H_uu", "Hdot");

  Eigen::LDLT<Eigen::MatrixXd> ldlt(H_uu);
  if (ldlt.info() != Eigen::Success) {
    throw std::runtime_error("LDLT factorization failed for H_uu.");
  }

  const Eigen::MatrixXd X = ldlt.solve(Hdot);
  if (ldlt.info() != Eigen::Success) {
    throw std::runtime_error("LDLT solve failed for H_uu^{-1} Hdot.");
  }

  return X.trace();
}

// Finite-difference approximation to:
//
//   d/dtheta_j log |H_uu(theta, uhat)|
//     = trace(H_uu^{-1} dH_uu/dtheta_j)
//
// IMPORTANT:
// This differentiates H_uu(theta, uhat) at fixed uhat. That is the
// validation target for the Laplace log-determinant envelope contribution.
// Do not perturb uhat inside hessian_uu_at_theta_fixed_uhat.
//
// HessianUUFn must be callable as:
//
//   Eigen::MatrixXd hessian_uu_at_theta_fixed_uhat(const Eigen::VectorXd&
//   theta)
//
// where the implementation closes over the already-optimized uhat.
template <class HessianUUFn>
Eigen::VectorXd
logdet_trace_gradient_fd(const HessianUUFn &hessian_uu_at_theta_fixed_uhat,
                         const Eigen::VectorXd &theta,
                         const FullExactLaplaceGradientFDOptions &options =
                             FullExactLaplaceGradientFDOptions{}) {
  const int p = static_cast<int>(theta.size());

  const Eigen::MatrixXd H0 = hessian_uu_at_theta_fixed_uhat(theta);
  if (options.check_dimensions && H0.rows() != H0.cols()) {
    throw std::invalid_argument("H_uu(theta) must be square.");
  }

  Eigen::VectorXd out(p);
  out.setZero();

  for (int j = 0; j < p; ++j) {
    const double h = detail::theta_step(theta[j], options);

    Eigen::VectorXd theta_plus = theta;
    Eigen::VectorXd theta_minus = theta;
    theta_plus[j] += h;
    theta_minus[j] -= h;

    const Eigen::MatrixXd H_plus = hessian_uu_at_theta_fixed_uhat(theta_plus);
    const Eigen::MatrixXd H_minus = hessian_uu_at_theta_fixed_uhat(theta_minus);

    if (options.check_dimensions) {
      detail::check_square_same_size(H0, H_plus, "H_uu(theta)",
                                     "H_uu(theta_plus)");
      detail::check_square_same_size(H0, H_minus, "H_uu(theta)",
                                     "H_uu(theta_minus)");
    }

    const Eigen::MatrixXd Hdot = (H_plus - H_minus) / (2.0 * h);

    out[j] = trace_hinv_hdot_dense(H0, Hdot);
  }

  return out;
}

// Adds the Laplace log-determinant contribution to an already-computed
// envelope gradient of the joint objective:
//
//   grad_full = grad_joint_envelope
//             + 0.5 * trace(H_uu^{-1} dH_uu/dtheta)
//
// The caller owns computation of grad_joint_envelope. For an optimized uhat,
// this is usually just the fixed-effect block of the joint objective gradient
// evaluated at (theta, uhat).
template <class HessianUUFn>
Eigen::VectorXd full_exact_laplace_gradient_fd(
    const Eigen::VectorXd &grad_joint_envelope,
    const HessianUUFn &hessian_uu_at_theta_fixed_uhat,
    const Eigen::VectorXd &theta,
    const FullExactLaplaceGradientFDOptions &options =
        FullExactLaplaceGradientFDOptions{}) {
  if (grad_joint_envelope.size() != theta.size()) {
    throw std::invalid_argument(
        "grad_joint_envelope and theta must have the same length.");
  }

  Eigen::VectorXd grad = grad_joint_envelope;
  grad += 0.5 * logdet_trace_gradient_fd(hessian_uu_at_theta_fixed_uhat, theta,
                                         options);
  return grad;
}

} // namespace laplace
} // namespace quadra
