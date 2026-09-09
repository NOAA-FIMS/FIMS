#pragma once

#include <Eigen/Dense>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <limits>
#include <numeric>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace quadra {

struct LaplaceEffectiveSparsityRow {
  double curvature_retained = 0.0;
  std::string label;
  std::size_t entries_required = 0;
  double entry_share = 0.0;
  double compression_vs_structural = 0.0;
};

struct LaplaceEffectiveBandwidthRow {
  double curvature_retained = 0.0;
  std::string label;
  std::size_t bandwidth = 0;
  std::size_t entry_count_if_banded = 0;
  double entry_share_if_banded = 0.0;
};

struct LaplaceStructureReport {
  std::size_t random_effects = 0;
  std::size_t total_entries = 0;
  std::size_t structural_nonzeros = 0;
  double structural_density = 0.0;
  double nonzero_tolerance = 1.0e-8;
  double max_abs_entry = 0.0;

  bool eigen_success = false;
  bool positive_definite = false;
  double min_eigenvalue = std::numeric_limits<double>::quiet_NaN();
  double max_eigenvalue = std::numeric_limits<double>::quiet_NaN();
  double condition_number_abs = std::numeric_limits<double>::quiet_NaN();

  std::vector<LaplaceEffectiveSparsityRow> effective_sparsity;
  std::vector<LaplaceEffectiveBandwidthRow> effective_bandwidth;
};

inline std::vector<std::pair<double, std::string>>
default_laplace_structure_targets() {
  return {{0.90, "90%"}, {0.95, "95%"},    {0.97, "97%"},    {0.98, "98%"},
          {0.99, "99%"}, {0.995, "99.5%"}, {0.999, "99.9%"}, {1.00, "100%"}};
}

inline std::size_t banded_square_entry_count(std::size_t n,
                                             std::size_t bandwidth) {
  std::size_t out = 0;
  for (std::size_t i = 0; i < n; ++i) {
    for (std::size_t j = 0; j < n; ++j) {
      const std::size_t d = (i > j) ? (i - j) : (j - i);
      if (d <= bandwidth) {
        ++out;
      }
    }
  }
  return out;
}

inline LaplaceStructureReport summarize_laplace_hessian_structure(
    const Eigen::MatrixXd &H, double nonzero_tol = 1.0e-8,
    const std::vector<std::pair<double, std::string>> &targets =
        default_laplace_structure_targets()) {
  LaplaceStructureReport report;
  report.random_effects = static_cast<std::size_t>(H.rows());
  report.total_entries = static_cast<std::size_t>(H.rows() * H.cols());
  report.nonzero_tolerance = nonzero_tol;

  if (H.rows() == 0 || H.cols() == 0) {
    return report;
  }

  if (H.rows() != H.cols()) {
    throw std::runtime_error(
        "summarize_laplace_hessian_structure requires a square matrix");
  }

  std::vector<double> abs_values;
  abs_values.reserve(report.total_entries);

  double total_abs = 0.0;
  for (Eigen::Index i = 0; i < H.rows(); ++i) {
    for (Eigen::Index j = 0; j < H.cols(); ++j) {
      const double av = std::abs(H(i, j));
      report.max_abs_entry = std::max(report.max_abs_entry, av);
      if (av > nonzero_tol) {
        ++report.structural_nonzeros;
        abs_values.push_back(av);
        total_abs += av;
      }
    }
  }

  report.structural_density =
      report.total_entries > 0
          ? static_cast<double>(report.structural_nonzeros) /
                static_cast<double>(report.total_entries)
          : 0.0;

  std::sort(abs_values.begin(), abs_values.end(), std::greater<double>());

  auto entries_for_share = [&](double target) -> std::size_t {
    if (target >= 1.0) {
      return report.structural_nonzeros;
    }
    double running = 0.0;
    for (std::size_t i = 0; i < abs_values.size(); ++i) {
      running += abs_values[i];
      if (total_abs > 0.0 && running / total_abs >= target) {
        return i + 1;
      }
    }
    return abs_values.size();
  };

  for (const auto &target : targets) {
    const std::size_t entries = entries_for_share(target.first);
    LaplaceEffectiveSparsityRow row;
    row.curvature_retained = target.first;
    row.label = target.second;
    row.entries_required = entries;
    row.entry_share = report.total_entries > 0
                          ? static_cast<double>(entries) /
                                static_cast<double>(report.total_entries)
                          : 0.0;
    row.compression_vs_structural =
        entries > 0 ? static_cast<double>(report.structural_nonzeros) /
                          static_cast<double>(entries)
                    : 0.0;
    report.effective_sparsity.push_back(row);
  }

  std::vector<double> band_abs(report.random_effects, 0.0);
  double upper_abs_total = 0.0;

  for (Eigen::Index i = 0; i < H.rows(); ++i) {
    for (Eigen::Index j = i; j < H.cols(); ++j) {
      const std::size_t d = static_cast<std::size_t>(j - i);
      const double av = std::abs(H(i, j));
      band_abs[d] += av;
      upper_abs_total += av;
    }
  }

  auto bandwidth_for_share = [&](double target) -> std::size_t {
    if (target >= 1.0) {
      return report.random_effects > 0 ? report.random_effects - 1 : 0;
    }
    double running = 0.0;
    for (std::size_t d = 0; d < band_abs.size(); ++d) {
      running += band_abs[d];
      if (upper_abs_total > 0.0 && running / upper_abs_total >= target) {
        return d;
      }
    }
    return report.random_effects > 0 ? report.random_effects - 1 : 0;
  };

  for (const auto &target : targets) {
    const std::size_t bw = bandwidth_for_share(target.first);
    LaplaceEffectiveBandwidthRow row;
    row.curvature_retained = target.first;
    row.label = target.second;
    row.bandwidth = bw;
    row.entry_count_if_banded =
        banded_square_entry_count(report.random_effects, bw);
    row.entry_share_if_banded =
        report.total_entries > 0
            ? static_cast<double>(row.entry_count_if_banded) /
                  static_cast<double>(report.total_entries)
            : 0.0;
    report.effective_bandwidth.push_back(row);
  }

  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eig(H);
  report.eigen_success = eig.info() == Eigen::Success;
  if (report.eigen_success && eig.eigenvalues().size() > 0) {
    report.min_eigenvalue = eig.eigenvalues().minCoeff();
    report.max_eigenvalue = eig.eigenvalues().maxCoeff();
    report.positive_definite = report.min_eigenvalue > 0.0;
    report.condition_number_abs =
        std::abs(report.max_eigenvalue) /
        std::max(std::abs(report.min_eigenvalue), 1.0e-300);
  }

  return report;
}

inline void
write_laplace_structure_report_text(const LaplaceStructureReport &report,
                                    std::ostream &out) {
  out << std::setprecision(15);
  out << "Laplace Structure Report\n";
  out << "========================\n\n";
  out << "Random effects:              " << report.random_effects << "\n";

  if (report.random_effects == 0) {
    out << "No random-effect Hessian available.\n";
    return;
  }

  out << "Matrix size:                 " << report.random_effects << " x "
      << report.random_effects << "\n";
  out << "Total entries:               " << report.total_entries << "\n";
  out << "Structural nonzeros:         " << report.structural_nonzeros << " / "
      << report.total_entries << " (" << 100.0 * report.structural_density
      << "%)\n";
  out << "Nonzero tolerance:           " << report.nonzero_tolerance << "\n";
  out << "Max |H_ij|:                  " << report.max_abs_entry << "\n";
  out << "Positive definite:           "
      << (report.positive_definite ? "yes" : "no") << "\n";
  out << "Min eigenvalue:              " << report.min_eigenvalue << "\n";
  out << "Max eigenvalue:              " << report.max_eigenvalue << "\n";
  out << "Condition number:            " << report.condition_number_abs
      << "\n\n";

  out << "Effective sparsity\n";
  out << "------------------\n";
  out << "curvature_retained,entries_required,entry_share,"
         "compression_vs_structural\n";
  for (const auto &row : report.effective_sparsity) {
    out << row.label << "," << row.entries_required << "," << row.entry_share
        << "," << row.compression_vs_structural << "\n";
  }

  out << "\nEffective bandwidth\n";
  out << "-------------------\n";
  out << "curvature_retained,bandwidth,entry_count_if_banded,"
         "entry_share_if_banded\n";
  for (const auto &row : report.effective_bandwidth) {
    out << row.label << "," << row.bandwidth << "," << row.entry_count_if_banded
        << "," << row.entry_share_if_banded << "\n";
  }

  out << "\nInterpretation\n";
  out << "--------------\n";
  out << "This report measures numerical curvature concentration, not only "
         "symbolic sparsity.\n";
  out << "A dense structural Hessian can still be effectively sparse if most "
         "curvature is carried by relatively few entries or bands.\n";
}

inline void
write_laplace_structure_report_text(const LaplaceStructureReport &report,
                                    const std::string &path) {
  std::ofstream out(path);
  write_laplace_structure_report_text(report, out);
}

inline void
write_laplace_structure_report_csv(const LaplaceStructureReport &report,
                                   std::ostream &out) {
  out << std::setprecision(15);
  out << "metric,target,value,extra\n";
  out << "random_effects,," << report.random_effects << ",\n";
  out << "total_entries,," << report.total_entries << ",\n";
  out << "structural_nonzeros,," << report.structural_nonzeros << ",\n";
  out << "structural_density,," << report.structural_density << ",\n";
  out << "positive_definite,," << (report.positive_definite ? "yes" : "no")
      << ",\n";
  out << "min_eigenvalue,," << report.min_eigenvalue << ",\n";
  out << "max_eigenvalue,," << report.max_eigenvalue << ",\n";
  out << "condition_number,," << report.condition_number_abs << ",\n";

  for (const auto &row : report.effective_sparsity) {
    out << "effective_sparsity_entries," << row.label << ","
        << row.entries_required
        << ",compression_vs_structural=" << row.compression_vs_structural
        << "\n";
  }

  for (const auto &row : report.effective_bandwidth) {
    out << "effective_bandwidth," << row.label << "," << row.bandwidth << ",\n";
  }
}

inline void
write_laplace_structure_report_csv(const LaplaceStructureReport &report,
                                   const std::string &path) {
  std::ofstream out(path);
  write_laplace_structure_report_csv(report, out);
}

} // namespace quadra
