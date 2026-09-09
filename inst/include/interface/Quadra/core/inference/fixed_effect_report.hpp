#ifndef QUADRA_FIXED_EFFECT_REPORT_HPP
#define QUADRA_FIXED_EFFECT_REPORT_HPP

#include "fixed_effect_covariance.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace quadra {

struct FixedEffectSummaryRow {
  std::string name_m;

  double estimate_m = std::numeric_limits<double>::quiet_NaN();
  double std_error_m = std::numeric_limits<double>::quiet_NaN();
  double z_value_m = std::numeric_limits<double>::quiet_NaN();
  double lower95_m = std::numeric_limits<double>::quiet_NaN();
  double upper95_m = std::numeric_limits<double>::quiet_NaN();
};

struct FixedEffectReport {
  std::vector<FixedEffectSummaryRow> rows_m;
  bool success_m = false;
  std::string message_m;
};

inline FixedEffectReport build_fixed_effect_report(
    const std::vector<std::string> &names, const std::vector<double> &estimates,
    const FixedEffectCovarianceResult &covariance_result) {
  FixedEffectReport report;

  if (!covariance_result.success_m) {
    report.success_m = false;
    report.message_m =
        "Cannot build fixed-effect report: covariance estimation failed: " +
        covariance_result.message_m;
    return report;
  }

  if (names.size() != estimates.size()) {
    report.success_m = false;
    report.message_m =
        "Fixed-effect names and estimates have different lengths.";
    return report;
  }

  const auto n = static_cast<Eigen::Index>(estimates.size());

  if (covariance_result.covariance_m.rows() != n ||
      covariance_result.covariance_m.cols() != n) {
    report.success_m = false;
    report.message_m = "Covariance matrix dimension does not match estimates.";
    return report;
  }

  report.rows_m.reserve(estimates.size());

  for (Eigen::Index i = 0; i < n; ++i) {
    FixedEffectSummaryRow row;
    row.name_m = names[static_cast<size_t>(i)];
    row.estimate_m = estimates[static_cast<size_t>(i)];

    const double variance = covariance_result.covariance_m(i, i);

    if (variance > 0.0 && std::isfinite(variance)) {
      row.std_error_m = std::sqrt(variance);
      row.z_value_m = row.estimate_m / row.std_error_m;
      row.lower95_m = row.estimate_m - 1.96 * row.std_error_m;
      row.upper95_m = row.estimate_m + 1.96 * row.std_error_m;
    }

    report.rows_m.push_back(row);
  }

  report.success_m = true;
  report.message_m = "Fixed-effect report built successfully.";
  return report;
}

inline void print_fixed_effect_report(const FixedEffectReport &report,
                                      std::ostream &os = std::cout) {
  if (!report.success_m) {
    os << "Fixed-effect report unavailable: " << report.message_m << "\n";
    return;
  }

  os << "\nFixed-effect inference report\n";
  os << std::setw(24) << "parameter" << std::setw(16) << "estimate"
     << std::setw(16) << "std.error" << std::setw(16) << "z" << std::setw(16)
     << "lower95" << std::setw(16) << "upper95" << "\n";

  for (const auto &row : report.rows_m) {
    os << std::setw(24) << row.name_m << std::setw(16) << row.estimate_m
       << std::setw(16) << row.std_error_m << std::setw(16) << row.z_value_m
       << std::setw(16) << row.lower95_m << std::setw(16) << row.upper95_m
       << "\n";
  }
}

} // namespace quadra

#endif
