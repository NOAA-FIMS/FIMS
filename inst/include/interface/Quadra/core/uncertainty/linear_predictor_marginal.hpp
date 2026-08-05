#pragma once

#include "../laplace/sparse_factorization_cache.hpp"
#include "../laplace/sparse_huu_factorization.hpp"

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <cmath>
#include <limits>
#include <string>

namespace quadra {

struct LinearPredictorMarginalResult {
  Eigen::VectorXd conditional_variance_m;
  Eigen::VectorXd parameter_variance_m;
  Eigen::VectorXd marginal_variance_m;
  Eigen::VectorXd std_error_m;
  bool success_m = false;
  std::string message_m;
};

inline LinearPredictorMarginalResult linear_predictor_marginal_diagonal(
    const SparseLDLTFactorizationCache &factorization,
    const laplace::TakahashiSelectedInverse &selected_inverse,
    const Eigen::MatrixXd &du_dtheta, const Eigen::MatrixXd &theta_covariance,
    const Eigen::MatrixXd &X_theta,
    const Eigen::SparseMatrix<double, Eigen::RowMajor> &Z,
    Eigen::Index block_size = 256) {
  LinearPredictorMarginalResult out;
  const Eigen::Index n = Z.rows();
  const Eigen::Index n_random = Z.cols();
  const Eigen::Index n_fixed = du_dtheta.cols();
  if (factorization.rows() != n_random || du_dtheta.rows() != n_random ||
      X_theta.rows() != n || X_theta.cols() != n_fixed ||
      theta_covariance.rows() != n_fixed ||
      theta_covariance.cols() != n_fixed || block_size < 1) {
    out.message_m = "Non-conformable cached prediction uncertainty inputs.";
    return out;
  }
  try {
    out.conditional_variance_m.resize(n);
    Eigen::MatrixXd sensitivity = X_theta + Z * du_dtheta;
    std::vector<Eigen::Index> unsupported;
    unsupported.reserve(static_cast<size_t>(n));
    for (Eigen::Index row = 0; row < n; ++row) {
      bool supported = true;
      for (Eigen::SparseMatrix<double, Eigen::RowMajor>::InnerIterator left(
               Z, row);
           left && supported; ++left)
        for (Eigen::SparseMatrix<double, Eigen::RowMajor>::InnerIterator right(
                 Z, row);
             right; ++right)
          if (!selected_inverse.contains(left.col(), right.col())) {
            supported = false;
            break;
          }
      if (!supported) {
        unsupported.push_back(row);
        continue;
      }
      double variance = 0.0;
      for (Eigen::SparseMatrix<double, Eigen::RowMajor>::InnerIterator left(
               Z, row);
           left; ++left)
        for (Eigen::SparseMatrix<double, Eigen::RowMajor>::InnerIterator right(
                 Z, row);
             right; ++right)
          variance += left.value() * right.value() *
                      selected_inverse.value(left.col(), right.col());
      out.conditional_variance_m[row] = variance;
    }
    for (Eigen::Index first = 0;
         first < static_cast<Eigen::Index>(unsupported.size());
         first += block_size) {
      const Eigen::Index count = std::min(
          block_size, static_cast<Eigen::Index>(unsupported.size()) - first);
      Eigen::MatrixXd rhs = Eigen::MatrixXd::Zero(n_random, count);
      for (Eigen::Index local = 0; local < count; ++local) {
        const Eigen::Index row =
            unsupported[static_cast<size_t>(first + local)];
        for (Eigen::SparseMatrix<double, Eigen::RowMajor>::InnerIterator it(
                 Z, row);
             it; ++it)
          rhs(it.col(), local) = it.value();
      }
      const Eigen::MatrixXd solution = factorization.solve(rhs);
      for (Eigen::Index local = 0; local < count; ++local) {
        const Eigen::Index row =
            unsupported[static_cast<size_t>(first + local)];
        out.conditional_variance_m[row] =
            rhs.col(local).dot(solution.col(local));
      }
    }
    const Eigen::MatrixXd propagated = sensitivity * theta_covariance;
    out.parameter_variance_m =
        (propagated.array() * sensitivity.array()).rowwise().sum();
    out.marginal_variance_m =
        out.conditional_variance_m + out.parameter_variance_m;
    out.std_error_m.resize(n);
    for (Eigen::Index i = 0; i < n; ++i) {
      const double variance = out.marginal_variance_m[i];
      out.std_error_m[i] = variance >= 0.0 && std::isfinite(variance)
                               ? std::sqrt(variance)
                               : std::numeric_limits<double>::quiet_NaN();
    }
    out.success_m = out.conditional_variance_m.allFinite() &&
                    out.parameter_variance_m.allFinite() &&
                    out.std_error_m.allFinite();
    out.message_m = out.success_m
                        ? "Selected-inverse prediction uncertainty computed."
                        : "Prediction uncertainty contains non-finite values.";
  } catch (const std::exception &e) {
    out.message_m =
        std::string("Cached prediction uncertainty failed: ") + e.what();
  }
  return out;
}

// Marginal uncertainty for linear functionals
// eta = X_theta theta + Z u_hat(theta).
// Sparse solves provide diag(Z H_uu^-1 Z'), while the profiled sensitivity
// X_theta + Z du/dtheta propagates fixed-parameter uncertainty.
inline LinearPredictorMarginalResult linear_predictor_marginal_diagonal(
    const Eigen::SparseMatrix<double> &H_uu, const Eigen::MatrixXd &du_dtheta,
    const Eigen::MatrixXd &theta_covariance, const Eigen::MatrixXd &X_theta,
    const Eigen::SparseMatrix<double, Eigen::RowMajor> &Z) {
  LinearPredictorMarginalResult out;
  const Eigen::Index n = Z.rows();
  const Eigen::Index n_random = H_uu.rows();
  const Eigen::Index n_fixed = du_dtheta.cols();
  if (H_uu.cols() != n_random || Z.cols() != n_random ||
      du_dtheta.rows() != n_random || X_theta.rows() != n ||
      X_theta.cols() != n_fixed || theta_covariance.rows() != n_fixed ||
      theta_covariance.cols() != n_fixed) {
    out.message_m = "Non-conformable prediction uncertainty inputs.";
    return out;
  }

  try {
    laplace::SparseHuuFactorization factorization(H_uu);
    out.conditional_variance_m.resize(n);
    Eigen::MatrixXd sensitivity = X_theta;
    Eigen::VectorXd rhs = Eigen::VectorXd::Zero(n_random);
    for (Eigen::Index i = 0; i < n; ++i) {
      for (Eigen::SparseMatrix<double, Eigen::RowMajor>::InnerIterator it(Z, i);
           it; ++it)
        rhs[it.col()] = it.value();
      out.conditional_variance_m[i] = rhs.dot(factorization.solve(rhs));
      sensitivity.row(i).noalias() += Z.row(i) * du_dtheta;
      for (Eigen::SparseMatrix<double, Eigen::RowMajor>::InnerIterator it(Z, i);
           it; ++it)
        rhs[it.col()] = 0.0;
    }
    const Eigen::MatrixXd propagated = sensitivity * theta_covariance;
    out.parameter_variance_m =
        (propagated.array() * sensitivity.array()).rowwise().sum();
    out.marginal_variance_m =
        out.conditional_variance_m + out.parameter_variance_m;
    out.std_error_m.resize(n);
    for (Eigen::Index i = 0; i < n; ++i) {
      const double variance = out.marginal_variance_m[i];
      out.std_error_m[i] = variance >= 0.0 && std::isfinite(variance)
                               ? std::sqrt(variance)
                               : std::numeric_limits<double>::quiet_NaN();
    }
    out.success_m = out.conditional_variance_m.allFinite() &&
                    out.parameter_variance_m.allFinite() &&
                    out.std_error_m.allFinite();
    out.message_m = out.success_m
                        ? "Linear-predictor marginal uncertainty computed."
                        : "Prediction uncertainty contains non-finite values.";
  } catch (const std::exception &e) {
    out.message_m =
        std::string("Linear-predictor marginal uncertainty failed: ") +
        e.what();
  }
  return out;
}

} // namespace quadra
