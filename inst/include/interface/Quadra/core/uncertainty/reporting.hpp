#pragma once

#include "selected_inverse_diagonal.hpp"

#include <Eigen/Cholesky>
#include <Eigen/Core>
#include <Eigen/Eigenvalues>
#include <Eigen/SparseCore>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <limits>
#include <numeric>
#include <random>
#include <string>
#include <vector>

namespace quadra {
namespace uncertainty {

// QUADRA_UNCERTAINTY_REPORTING_CORE_V1

inline double quantile_sorted(const std::vector<double> &sorted, double p) {
  if (sorted.empty())
    return std::numeric_limits<double>::quiet_NaN();
  if (sorted.size() == 1)
    return sorted.front();

  const double x = p * static_cast<double>(sorted.size() - 1);
  const std::size_t lo = static_cast<std::size_t>(std::floor(x));
  const std::size_t hi = std::min<std::size_t>(lo + 1, sorted.size() - 1);
  const double w = x - static_cast<double>(lo);
  return (1.0 - w) * sorted[lo] + w * sorted[hi];
}

inline Eigen::MatrixXd
covariance_to_correlation_matrix(const Eigen::MatrixXd &covariance) {
  const Eigen::Index n = covariance.rows();
  Eigen::MatrixXd correlation = Eigen::MatrixXd::Constant(
      n, covariance.cols(), std::numeric_limits<double>::quiet_NaN());

  if (covariance.rows() != covariance.cols()) {
    return correlation;
  }

  for (Eigen::Index i = 0; i < n; ++i) {
    for (Eigen::Index j = 0; j < n; ++j) {
      const double vii = covariance(i, i);
      const double vjj = covariance(j, j);
      const double vij = covariance(i, j);

      if (std::isfinite(vii) && std::isfinite(vjj) && std::isfinite(vij) &&
          vii > 0.0 && vjj > 0.0) {
        double c = vij / std::sqrt(vii * vjj);
        if (c > 1.0 && c < 1.0 + 1.0e-10)
          c = 1.0;
        if (c < -1.0 && c > -1.0 - 1.0e-10)
          c = -1.0;
        correlation(i, j) = c;
      }
    }
  }

  return correlation;
}

inline Eigen::MatrixXd
lognormal_delta_covariance(const Eigen::VectorXd &log_estimates,
                           const Eigen::MatrixXd &log_covariance) {
  const Eigen::Index n = log_estimates.size();

  Eigen::MatrixXd out =
      Eigen::MatrixXd::Constant(n, n, std::numeric_limits<double>::quiet_NaN());

  if (log_covariance.rows() != n || log_covariance.cols() != n) {
    return out;
  }

  for (Eigen::Index i = 0; i < n; ++i) {
    const double xi = std::exp(log_estimates[i]);
    for (Eigen::Index j = 0; j < n; ++j) {
      const double xj = std::exp(log_estimates[j]);
      out(i, j) = xi * xj * log_covariance(i, j);
    }
  }

  return out;
}

struct CovarianceDiagnostics {
  int n = 0;
  bool finite_all = false;
  bool positive_diagonal = false;
  bool valid_covariance = false;
  bool ldlt_success = false;
  double max_abs_asymmetry = std::numeric_limits<double>::quiet_NaN();
  double min_variance = std::numeric_limits<double>::quiet_NaN();
  double max_variance = std::numeric_limits<double>::quiet_NaN();
  double min_eigenvalue = std::numeric_limits<double>::quiet_NaN();
  double max_eigenvalue = std::numeric_limits<double>::quiet_NaN();
};

inline CovarianceDiagnostics
diagnose_covariance_matrix(const Eigen::MatrixXd &covariance,
                           double symmetry_tol = 1.0e-8,
                           double psd_tol = 1.0e-8) {
  CovarianceDiagnostics d;
  d.n = static_cast<int>(covariance.rows());

  if (covariance.rows() != covariance.cols()) {
    return d;
  }

  const Eigen::Index n = covariance.rows();

  d.finite_all = true;
  d.positive_diagonal = true;

  double min_diag = std::numeric_limits<double>::infinity();
  double max_diag = -std::numeric_limits<double>::infinity();

  for (Eigen::Index i = 0; i < n; ++i) {
    const double v = covariance(i, i);
    if (!std::isfinite(v))
      d.finite_all = false;
    if (!(v > 0.0))
      d.positive_diagonal = false;

    if (std::isfinite(v)) {
      min_diag = std::min(min_diag, v);
      max_diag = std::max(max_diag, v);
    }

    for (Eigen::Index j = 0; j < n; ++j) {
      if (!std::isfinite(covariance(i, j)))
        d.finite_all = false;
    }
  }

  if (n > 0) {
    d.max_abs_asymmetry =
        (covariance - covariance.transpose()).cwiseAbs().maxCoeff();
    d.min_variance = min_diag;
    d.max_variance = max_diag;
  }

  if (n > 0 && d.finite_all) {
    Eigen::LDLT<Eigen::MatrixXd> ldlt(covariance);
    d.ldlt_success = (ldlt.info() == Eigen::Success &&
                      (ldlt.vectorD().array() > -psd_tol).all());

    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eig(
        0.5 * (covariance + covariance.transpose()));
    if (eig.info() == Eigen::Success) {
      d.min_eigenvalue = eig.eigenvalues().minCoeff();
      d.max_eigenvalue = eig.eigenvalues().maxCoeff();
    }
  }

  d.valid_covariance = d.finite_all && d.positive_diagonal &&
                       std::isfinite(d.max_abs_asymmetry) &&
                       d.max_abs_asymmetry < symmetry_tol && d.ldlt_success &&
                       std::isfinite(d.min_eigenvalue) &&
                       d.min_eigenvalue > -psd_tol;

  return d;
}

struct CorrelationDecayRow {
  int lag = 0;
  int count = 0;
  double mean_correlation = std::numeric_limits<double>::quiet_NaN();
  double min_correlation = std::numeric_limits<double>::quiet_NaN();
  double max_correlation = std::numeric_limits<double>::quiet_NaN();
};

inline std::vector<CorrelationDecayRow>
correlation_decay_summary(const Eigen::MatrixXd &correlation) {
  std::vector<CorrelationDecayRow> rows;

  if (correlation.rows() != correlation.cols()) {
    return rows;
  }

  const Eigen::Index n = correlation.rows();
  rows.reserve(static_cast<std::size_t>(n));

  for (Eigen::Index lag = 0; lag < n; ++lag) {
    CorrelationDecayRow row;
    row.lag = static_cast<int>(lag);

    double sum = 0.0;
    double min_corr = std::numeric_limits<double>::infinity();
    double max_corr = -std::numeric_limits<double>::infinity();

    for (Eigen::Index i = 0; i + lag < n; ++i) {
      const double c = correlation(i, i + lag);
      if (std::isfinite(c)) {
        sum += c;
        min_corr = std::min(min_corr, c);
        max_corr = std::max(max_corr, c);
        ++row.count;
      }
    }

    if (row.count > 0) {
      row.mean_correlation = sum / static_cast<double>(row.count);
      row.min_correlation = min_corr;
      row.max_correlation = max_corr;
    }

    rows.push_back(row);
  }

  return rows;
}

inline void write_dense_matrix_csv(const std::string &path,
                                   const Eigen::MatrixXd &matrix,
                                   const std::vector<std::string> &labels) {
  std::ofstream out(path);
  out << "label";
  for (const auto &label : labels) {
    out << "," << label;
  }
  out << "\n";

  for (Eigen::Index i = 0; i < matrix.rows(); ++i) {
    const std::string row_label = static_cast<std::size_t>(i) < labels.size()
                                      ? labels[static_cast<std::size_t>(i)]
                                      : std::to_string(i);
    out << row_label;
    for (Eigen::Index j = 0; j < matrix.cols(); ++j) {
      out << "," << matrix(i, j);
    }
    out << "\n";
  }
}

inline void write_covariance_diagnostics_csv(const std::string &path,
                                             const CovarianceDiagnostics &d) {
  std::ofstream out(path);
  out << "metric,value,note\n";
  out << "n," << d.n << ",matrix dimension\n";
  out << "finite_all," << (d.finite_all ? "yes" : "no")
      << ",all entries finite\n";
  out << "positive_diagonal," << (d.positive_diagonal ? "yes" : "no")
      << ",all diagonal variances positive\n";
  out << "valid_covariance," << (d.valid_covariance ? "yes" : "no")
      << ",finite positive-diagonal symmetric positive-semidefinite check\n";
  out << "ldlt_success," << (d.ldlt_success ? "yes" : "no")
      << ",dense LDLT check\n";
  out << "max_abs_asymmetry," << d.max_abs_asymmetry
      << ",maximum absolute covariance asymmetry\n";
  out << "min_variance," << d.min_variance << ",minimum diagonal variance\n";
  out << "max_variance," << d.max_variance << ",maximum diagonal variance\n";
  out << "min_eigenvalue," << d.min_eigenvalue
      << ",self-adjoint eigenvalue diagnostic\n";
  out << "max_eigenvalue," << d.max_eigenvalue
      << ",self-adjoint eigenvalue diagnostic\n";
}

inline void
write_correlation_decay_csv(const std::string &path,
                            const std::vector<CorrelationDecayRow> &rows) {
  std::ofstream out(path);
  out << "lag,count,mean_correlation,min_correlation,max_correlation\n";
  for (const auto &r : rows) {
    out << r.lag << "," << r.count << "," << r.mean_correlation << ","
        << r.min_correlation << "," << r.max_correlation << "\n";
  }
}

struct ProjectionEnvelopeRow {
  std::string scenario;
  int year = 0;
  std::string quantity;
  double estimate = std::numeric_limits<double>::quiet_NaN();
  double mean = std::numeric_limits<double>::quiet_NaN();
  double median = std::numeric_limits<double>::quiet_NaN();
  double lwr_95 = std::numeric_limits<double>::quiet_NaN();
  double upr_95 = std::numeric_limits<double>::quiet_NaN();
  double se = std::numeric_limits<double>::quiet_NaN();
  int n_samples = 0;
  std::string note;
};

inline ProjectionEnvelopeRow
summarize_samples(const std::string &scenario, int year,
                  const std::string &quantity, double estimate,
                  std::vector<double> samples, const std::string &note) {
  ProjectionEnvelopeRow row;
  row.scenario = scenario;
  row.year = year;
  row.quantity = quantity;
  row.estimate = estimate;
  row.n_samples = static_cast<int>(samples.size());
  row.note = note;

  samples.erase(std::remove_if(samples.begin(), samples.end(),
                               [](double x) { return !std::isfinite(x); }),
                samples.end());

  if (samples.empty()) {
    row.n_samples = 0;
    return row;
  }

  const double sum = std::accumulate(samples.begin(), samples.end(), 0.0);
  row.mean = sum / static_cast<double>(samples.size());

  if (samples.size() > 1) {
    double ss = 0.0;
    for (double x : samples) {
      const double d = x - row.mean;
      ss += d * d;
    }
    row.se = std::sqrt(ss / static_cast<double>(samples.size() - 1));
  } else {
    row.se = 0.0;
  }

  std::sort(samples.begin(), samples.end());
  row.median = quantile_sorted(samples, 0.50);
  row.lwr_95 = quantile_sorted(samples, 0.025);
  row.upr_95 = quantile_sorted(samples, 0.975);
  row.n_samples = static_cast<int>(samples.size());

  return row;
}

inline void
write_projection_envelopes_csv(const std::string &path,
                               const std::vector<ProjectionEnvelopeRow> &rows) {
  std::ofstream out(path);
  out << "scenario,year,quantity,estimate,mean,median,lwr_95,upr_95,se,"
      << "n_samples,note\n";

  for (const auto &r : rows) {
    out << r.scenario << "," << r.year << "," << r.quantity << "," << r.estimate
        << "," << r.mean << "," << r.median << "," << r.lwr_95 << ","
        << r.upr_95 << "," << r.se << "," << r.n_samples << "," << r.note
        << "\n";
  }
}

} // namespace uncertainty
} // namespace quadra
