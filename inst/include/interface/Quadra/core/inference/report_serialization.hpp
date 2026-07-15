#ifndef QUADRA_REPORT_SERIALIZATION_HPP
#define QUADRA_REPORT_SERIALIZATION_HPP

#include "../laplace/laplace_profiled_derived_report.hpp"

#include <Eigen/Dense>

#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <string>

namespace quadra {

inline void write_matrix_csv(const std::string &path,
                             const Eigen::MatrixXd &matrix) {
  std::ofstream out(path);

  if (!out) {
    throw std::runtime_error("write_matrix_csv: failed to open output file: " +
                             path);
  }

  out << std::setprecision(17);

  for (Eigen::Index i = 0; i < matrix.rows(); ++i) {
    for (Eigen::Index j = 0; j < matrix.cols(); ++j) {
      if (j > 0) {
        out << ",";
      }

      out << matrix(i, j);
    }

    out << "\n";
  }
}

inline void
write_profiled_derived_report_csv(const std::string &path,
                                  const LaplaceProfiledDerivedReport &report) {
  if (!report.success_m) {
    throw std::runtime_error(
        "write_profiled_derived_report_csv: report is not successful: " +
        report.message_m);
  }

  const auto &delta = report.delta_m;

  if (static_cast<Eigen::Index>(report.names_m.size()) !=
      delta.estimate_m.size()) {
    throw std::runtime_error("write_profiled_derived_report_csv: names and "
                             "estimates size mismatch.");
  }

  std::ofstream out(path);

  if (!out) {
    throw std::runtime_error(
        "write_profiled_derived_report_csv: failed to open output file: " +
        path);
  }

  out << std::setprecision(17);

  out << "quantity,estimate,std_error,cv\n";

  for (std::size_t i = 0; i < report.names_m.size(); ++i) {
    const Eigen::Index k = static_cast<Eigen::Index>(i);

    out << report.names_m[i] << "," << delta.estimate_m[k] << ","
        << delta.std_error_m[k] << "," << delta.cv_m[k] << "\n";
  }
}

} // namespace quadra

#endif
