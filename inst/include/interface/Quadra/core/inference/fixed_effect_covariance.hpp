#ifndef QUADRA_FIXED_EFFECT_COVARIANCE_HPP
#define QUADRA_FIXED_EFFECT_COVARIANCE_HPP

#include <Eigen/Dense>

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <vector>

namespace quadra {

struct FixedEffectCovarianceResult {
  Eigen::MatrixXd hessian_m;
  Eigen::MatrixXd covariance_m;
  Eigen::MatrixXd correlation_m;
  Eigen::VectorXd eigenvalues_m;
  Eigen::VectorXd steps_m;

  bool success_m = false;
  bool positive_definite_m = false;
  double condition_number_m = std::numeric_limits<double>::infinity();
  double eigenvalue_threshold_m = std::numeric_limits<double>::quiet_NaN();
  std::string message_m;
};

inline bool matrix_all_finite(const Eigen::MatrixXd &x) {
  for (Eigen::Index i = 0; i < x.rows(); ++i) {
    for (Eigen::Index j = 0; j < x.cols(); ++j) {
      if (!std::isfinite(x(i, j))) {
        return false;
      }
    }
  }
  return true;
}

inline Eigen::MatrixXd
covariance_to_correlation(const Eigen::MatrixXd &covariance) {
  const Eigen::Index n = covariance.rows();
  Eigen::MatrixXd correlation = Eigen::MatrixXd::Zero(n, n);

  for (Eigen::Index i = 0; i < n; ++i) {
    const double vii = covariance(i, i);

    for (Eigen::Index j = 0; j < n; ++j) {
      const double vjj = covariance(j, j);

      if (vii > 0.0 && vjj > 0.0) {
        correlation(i, j) = covariance(i, j) / std::sqrt(vii * vjj);
      } else {
        correlation(i, j) = std::numeric_limits<double>::quiet_NaN();
      }
    }
  }

  return correlation;
}

template <typename Gradient>
FixedEffectCovarianceResult estimate_fixed_effect_covariance_from_gradient(
    Gradient &gradient, const std::vector<double> &theta_hat,
    double relative_step = std::cbrt(std::numeric_limits<double>::epsilon()),
    double eigen_tolerance =
        std::sqrt(std::numeric_limits<double>::epsilon())) {
  FixedEffectCovarianceResult result;
  const Eigen::Index n = static_cast<Eigen::Index>(theta_hat.size());
  result.hessian_m = Eigen::MatrixXd::Zero(n, n);
  result.covariance_m = Eigen::MatrixXd::Zero(n, n);
  result.correlation_m = Eigen::MatrixXd::Zero(n, n);
  result.steps_m = Eigen::VectorXd::Zero(n);

  if (n == 0 || !(relative_step > 0.0) || !std::isfinite(relative_step)) {
    result.message_m = "Invalid fixed effects or finite-difference step.";
    return result;
  }

  for (Eigen::Index j = 0; j < n; ++j) {
    double step = relative_step * std::max(1.0, std::abs(theta_hat[j]));
    std::vector<double> plus = theta_hat;
    std::vector<double> minus = theta_hat;
    plus[static_cast<size_t>(j)] += step;
    minus[static_cast<size_t>(j)] -= step;
    std::vector<double> gradient_plus = gradient(plus);
    std::vector<double> gradient_minus = gradient(minus);
    if (gradient_plus.size() != static_cast<size_t>(n) ||
        gradient_minus.size() != static_cast<size_t>(n)) {
      result.message_m = "Gradient dimension does not match fixed effects.";
      return result;
    }
    for (Eigen::Index i = 0; i < n; ++i) {
      if (!std::isfinite(gradient_plus[static_cast<size_t>(i)]) ||
          !std::isfinite(gradient_minus[static_cast<size_t>(i)])) {
        result.message_m =
            "Gradient became non-finite while estimating the Hessian.";
        return result;
      }
      result.hessian_m(i, j) = (gradient_plus[static_cast<size_t>(i)] -
                                gradient_minus[static_cast<size_t>(i)]) /
                               (2.0 * step);
    }
    result.steps_m[j] = step;
  }

  result.hessian_m = 0.5 * (result.hessian_m + result.hessian_m.transpose());
  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigen(result.hessian_m);
  if (eigen.info() != Eigen::Success) {
    result.message_m = "Eigendecomposition of fixed Hessian failed.";
    return result;
  }
  result.eigenvalues_m = eigen.eigenvalues();
  const double scale =
      std::max(1.0, result.eigenvalues_m.cwiseAbs().maxCoeff());
  result.eigenvalue_threshold_m = eigen_tolerance * scale;
  result.positive_definite_m =
      result.eigenvalues_m.minCoeff() > result.eigenvalue_threshold_m;
  if (!result.positive_definite_m) {
    result.message_m = "Fixed-effect Hessian is not positive definite.";
    return result;
  }
  result.condition_number_m =
      result.eigenvalues_m.maxCoeff() / result.eigenvalues_m.minCoeff();

  Eigen::LLT<Eigen::MatrixXd> llt(result.hessian_m);
  if (llt.info() != Eigen::Success) {
    result.message_m = "Cholesky factorization of fixed Hessian failed.";
    return result;
  }
  result.covariance_m = llt.solve(Eigen::MatrixXd::Identity(n, n));
  result.covariance_m =
      0.5 * (result.covariance_m + result.covariance_m.transpose());
  if (!matrix_all_finite(result.covariance_m)) {
    result.message_m = "Estimated covariance contains non-finite values.";
    return result;
  }
  result.correlation_m = covariance_to_correlation(result.covariance_m);
  result.success_m = true;
  result.message_m = "Fixed-effect covariance estimated from exact gradients.";
  return result;
}

template <typename Objective>
FixedEffectCovarianceResult
estimate_fixed_effect_covariance(Objective &objective,
                                 const std::vector<double> &theta_hat,
                                 double step = 1.0e-4) {
  FixedEffectCovarianceResult result;

  const Eigen::Index n = static_cast<Eigen::Index>(theta_hat.size());

  result.hessian_m = Eigen::MatrixXd::Zero(n, n);
  result.covariance_m = Eigen::MatrixXd::Zero(n, n);
  result.correlation_m = Eigen::MatrixXd::Zero(n, n);

  if (n == 0) {
    result.success_m = false;
    result.message_m = "No fixed effects supplied.";
    return result;
  }

  if (!(step > 0.0) || !std::isfinite(step)) {
    result.success_m = false;
    result.message_m = "Finite-difference step must be positive and finite.";
    return result;
  }

  auto eval = [&](const std::vector<double> &x) -> double {
    const double fx = objective(x);
    return fx;
  };

  const double f0 = eval(theta_hat);
  if (!std::isfinite(f0)) {
    result.success_m = false;
    result.message_m = "Objective is non-finite at theta_hat.";
    return result;
  }

  for (Eigen::Index i = 0; i < n; ++i) {
    std::vector<double> xp = theta_hat;
    std::vector<double> xm = theta_hat;

    xp[static_cast<size_t>(i)] += step;
    xm[static_cast<size_t>(i)] -= step;

    const double fp = eval(xp);
    const double fm = eval(xm);

    if (!std::isfinite(fp) || !std::isfinite(fm)) {
      result.success_m = false;
      result.message_m = "Objective became non-finite while estimating "
                         "diagonal Hessian entries.";
      return result;
    }

    result.hessian_m(i, i) = (fp - 2.0 * f0 + fm) / (step * step);
  }

  for (Eigen::Index i = 0; i < n; ++i) {
    for (Eigen::Index j = i + 1; j < n; ++j) {
      std::vector<double> xpp = theta_hat;
      std::vector<double> xpm = theta_hat;
      std::vector<double> xmp = theta_hat;
      std::vector<double> xmm = theta_hat;

      xpp[static_cast<size_t>(i)] += step;
      xpp[static_cast<size_t>(j)] += step;

      xpm[static_cast<size_t>(i)] += step;
      xpm[static_cast<size_t>(j)] -= step;

      xmp[static_cast<size_t>(i)] -= step;
      xmp[static_cast<size_t>(j)] += step;

      xmm[static_cast<size_t>(i)] -= step;
      xmm[static_cast<size_t>(j)] -= step;

      const double fpp = eval(xpp);
      const double fpm = eval(xpm);
      const double fmp = eval(xmp);
      const double fmm = eval(xmm);

      if (!std::isfinite(fpp) || !std::isfinite(fpm) || !std::isfinite(fmp) ||
          !std::isfinite(fmm)) {
        result.success_m = false;
        result.message_m = "Objective became non-finite while estimating "
                           "off-diagonal Hessian entries.";
        return result;
      }

      const double hij = (fpp - fpm - fmp + fmm) / (4.0 * step * step);
      result.hessian_m(i, j) = hij;
      result.hessian_m(j, i) = hij;
    }
  }

  result.hessian_m = 0.5 * (result.hessian_m + result.hessian_m.transpose());

  if (!matrix_all_finite(result.hessian_m)) {
    result.success_m = false;
    result.message_m = "Estimated Hessian contains non-finite values.";
    return result;
  }

  Eigen::LDLT<Eigen::MatrixXd> ldlt(result.hessian_m);

  if (ldlt.info() != Eigen::Success) {
    result.success_m = false;
    result.message_m = "LDLT factorization of fixed-effect Hessian failed.";
    return result;
  }

  if (!ldlt.isPositive()) {
    result.success_m = false;
    result.message_m = "Fixed-effect Hessian is not positive definite.";
    return result;
  }

  result.covariance_m = ldlt.solve(Eigen::MatrixXd::Identity(n, n));

  if (!matrix_all_finite(result.covariance_m)) {
    result.success_m = false;
    result.message_m = "Estimated covariance contains non-finite values.";
    return result;
  }

  result.covariance_m =
      0.5 * (result.covariance_m + result.covariance_m.transpose());
  result.correlation_m = covariance_to_correlation(result.covariance_m);

  result.success_m = true;
  result.message_m = "Fixed-effect covariance estimated successfully.";

  return result;
}

} // namespace quadra

#endif
