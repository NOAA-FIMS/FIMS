#pragma once

#include <Eigen/Core>
#include <Eigen/SparseCholesky>
#include <Eigen/SparseCore>

#include <cmath>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace quadra {
namespace uncertainty {

struct SelectedInverseDiagonalResult {
  std::vector<double> variance;
  std::vector<double> standard_error;
  bool success = false;
  std::string message;
};

// Level-1 conditional random-effect covariance utility.
//
// For a positive-definite random-effect Hessian H_uu, the conditional
// covariance is approximately inv(H_uu). This conservative implementation
// extracts diag(inv(H_uu)) by solving H_uu x_i = e_i and reading x_i[i].
//
// This is deliberately simple and validation-friendly. Later, this can be
// replaced with a true selected-inverse algorithm for very large models.
inline SelectedInverseDiagonalResult selected_inverse_diagonal_from_spd_hessian(
    const Eigen::SparseMatrix<double> &hessian, double min_variance = 0.0) {
  SelectedInverseDiagonalResult out;

  const int n = static_cast<int>(hessian.rows());
  if (hessian.rows() != hessian.cols()) {
    out.message = "Hessian is not square";
    return out;
  }

  out.variance.assign(static_cast<std::size_t>(n),
                      std::numeric_limits<double>::quiet_NaN());
  out.standard_error.assign(static_cast<std::size_t>(n),
                            std::numeric_limits<double>::quiet_NaN());

  if (n == 0) {
    out.success = true;
    out.message = "empty Hessian";
    return out;
  }

  Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> ldlt;
  ldlt.analyzePattern(hessian);
  ldlt.factorize(hessian);

  if (ldlt.info() != Eigen::Success) {
    out.message = "SimplicialLDLT factorization failed";
    return out;
  }

  Eigen::VectorXd rhs = Eigen::VectorXd::Zero(n);
  for (int i = 0; i < n; ++i) {
    rhs.setZero();
    rhs[i] = 1.0;

    const Eigen::VectorXd sol = ldlt.solve(rhs);
    if (ldlt.info() != Eigen::Success) {
      out.message = "SimplicialLDLT solve failed";
      return out;
    }

    double v = sol[i];

    if (std::isfinite(v) && v < 0.0 && std::abs(v) <= 1.0e-12) {
      v = 0.0;
    }

    if (std::isfinite(v) && v < min_variance) {
      v = min_variance;
    }

    out.variance[static_cast<std::size_t>(i)] = v;
    out.standard_error[static_cast<std::size_t>(i)] =
        (std::isfinite(v) && v >= 0.0)
            ? std::sqrt(v)
            : std::numeric_limits<double>::quiet_NaN();
  }

  out.success = true;
  out.message = "ok";
  return out;
}

struct SelectedInverseEntry {
  int row = -1;
  int col = -1;
  double covariance = std::numeric_limits<double>::quiet_NaN();
};

struct SelectedInverseEntriesResult {
  std::vector<SelectedInverseEntry> entries;
  bool success = false;
  std::string message;
};

// Return requested entries of inv(H) for SPD H.
//
// This simple Level-1 implementation groups requested entries by column,
// solves H x_col = e_col once per unique requested column, and reads row
// entries from the solution. It is enough for derived quantities like:
//   Cov(log_B[t], log_B[0]).
inline SelectedInverseEntriesResult selected_inverse_entries_from_spd_hessian(
    const Eigen::SparseMatrix<double> &hessian,
    const std::vector<std::pair<int, int>> &requested_pairs) {
  SelectedInverseEntriesResult out;
  out.entries.reserve(requested_pairs.size());

  const int n = static_cast<int>(hessian.rows());
  if (hessian.rows() != hessian.cols()) {
    out.message = "Hessian is not square";
    return out;
  }

  for (const auto &rc : requested_pairs) {
    if (rc.first < 0 || rc.second < 0 || rc.first >= n || rc.second >= n) {
      out.message = "Requested inverse entry is out of range";
      return out;
    }
  }

  if (requested_pairs.empty()) {
    out.success = true;
    out.message = "empty request";
    return out;
  }

  Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> ldlt;
  ldlt.analyzePattern(hessian);
  ldlt.factorize(hessian);

  if (ldlt.info() != Eigen::Success) {
    out.message = "SimplicialLDLT factorization failed";
    return out;
  }

  out.entries.resize(requested_pairs.size());
  for (std::size_t k = 0; k < requested_pairs.size(); ++k) {
    out.entries[k].row = requested_pairs[k].first;
    out.entries[k].col = requested_pairs[k].second;
  }

  Eigen::VectorXd rhs = Eigen::VectorXd::Zero(n);
  int cached_col = -1;
  Eigen::VectorXd sol;

  for (std::size_t k = 0; k < requested_pairs.size(); ++k) {
    const int row = requested_pairs[k].first;
    const int col = requested_pairs[k].second;

    if (col != cached_col) {
      rhs.setZero();
      rhs[col] = 1.0;
      sol = ldlt.solve(rhs);
      if (ldlt.info() != Eigen::Success) {
        out.message = "SimplicialLDLT solve failed";
        return out;
      }
      cached_col = col;
    }

    out.entries[k].covariance = sol[row];
  }

  out.success = true;
  out.message = "ok";
  return out;
}

struct SelectedInverseSubmatrixResult {
  Eigen::MatrixXd covariance;
  std::vector<int> indices;
  bool success = false;
  std::string message;
};

// Return inv(H)[indices, indices] for SPD H.
//
// This Level-1 implementation solves H x_j = e_{indices[j]} once for each
// requested column and reads the requested rows. It avoids forming the full
// inverse while producing a dense covariance block suitable for trajectory
// uncertainty, projection simulation, and reporting diagnostics.
inline SelectedInverseSubmatrixResult
selected_inverse_submatrix_from_spd_hessian(
    const Eigen::SparseMatrix<double> &hessian,
    const std::vector<int> &indices) {
  SelectedInverseSubmatrixResult out;
  out.indices = indices;

  const int n = static_cast<int>(hessian.rows());
  const int m = static_cast<int>(indices.size());
  out.covariance =
      Eigen::MatrixXd::Constant(m, m, std::numeric_limits<double>::quiet_NaN());

  if (hessian.rows() != hessian.cols()) {
    out.message = "Hessian is not square";
    return out;
  }

  for (int idx : indices) {
    if (idx < 0 || idx >= n) {
      out.message = "Requested submatrix index is out of range";
      return out;
    }
  }

  if (m == 0) {
    out.success = true;
    out.message = "empty request";
    return out;
  }

  Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> ldlt;
  ldlt.analyzePattern(hessian);
  ldlt.factorize(hessian);

  if (ldlt.info() != Eigen::Success) {
    out.message = "SimplicialLDLT factorization failed";
    return out;
  }

  Eigen::VectorXd rhs = Eigen::VectorXd::Zero(n);

  for (int col_block = 0; col_block < m; ++col_block) {
    const int full_col = indices[static_cast<std::size_t>(col_block)];

    rhs.setZero();
    rhs[full_col] = 1.0;

    const Eigen::VectorXd sol = ldlt.solve(rhs);
    if (ldlt.info() != Eigen::Success) {
      out.message = "SimplicialLDLT solve failed";
      return out;
    }

    for (int row_block = 0; row_block < m; ++row_block) {
      const int full_row = indices[static_cast<std::size_t>(row_block)];
      out.covariance(row_block, col_block) = sol[full_row];
    }
  }

  // Symmetrize tiny numerical drift from solve order.
  out.covariance = 0.5 * (out.covariance + out.covariance.transpose());

  out.success = true;
  out.message = "ok";
  return out;
}

} // namespace uncertainty
} // namespace quadra
