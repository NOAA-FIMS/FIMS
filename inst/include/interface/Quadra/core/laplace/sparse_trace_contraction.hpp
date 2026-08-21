#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <algorithm>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace quadra {
namespace laplace {

// Reference dense trace:
//
//   trace(H^{-1} Hdot)
//
// using one dense RHS solve.
inline double
trace_hinv_hdot_dense_rhs(const Eigen::LDLT<Eigen::MatrixXd> &ldlt,
                          const Eigen::SparseMatrix<double> &Hdot) {
  if (Hdot.rows() != Hdot.cols()) {
    throw std::invalid_argument("Hdot must be square.");
  }

  const Eigen::MatrixXd rhs = Eigen::MatrixXd(Hdot);
  return ldlt.solve(rhs).trace();
}

// Sparse contraction:
//
//   trace(H^{-1} Hdot) = sum_ij H^{-1}(j,i) Hdot(i,j)
//
// This solves only for inverse columns needed by Hdot nonzeros.
//
// For each nonzero Hdot(i,j), we need Hinv(j,i). Since H is symmetric,
// Hinv(j,i) = solution_to_H_x_equals_e_j[i].
//
// This implementation is intentionally simple and benchmark-oriented.
// A production version can cache selected inverse columns across Hdot_j or use
// sparse/banded solvers.
inline double trace_hinv_hdot_selected_inverse_columns(
    const Eigen::LDLT<Eigen::MatrixXd> &ldlt,
    const Eigen::SparseMatrix<double> &Hdot) {
  if (Hdot.rows() != Hdot.cols()) {
    throw std::invalid_argument("Hdot must be square.");
  }

  const int n = static_cast<int>(Hdot.rows());

  std::vector<int> needed_columns;
  needed_columns.reserve(static_cast<size_t>(Hdot.nonZeros()));

  for (int outer = 0; outer < Hdot.outerSize(); ++outer) {
    for (Eigen::SparseMatrix<double>::InnerIterator it(Hdot, outer); it; ++it) {
      needed_columns.push_back(static_cast<int>(it.col()));
    }
  }

  std::sort(needed_columns.begin(), needed_columns.end());
  needed_columns.erase(
      std::unique(needed_columns.begin(), needed_columns.end()),
      needed_columns.end());

  std::unordered_map<int, Eigen::VectorXd> inv_columns;
  inv_columns.reserve(needed_columns.size());

  for (int col : needed_columns) {
    Eigen::VectorXd e = Eigen::VectorXd::Zero(n);
    e[col] = 1.0;
    inv_columns.emplace(col, ldlt.solve(e));
  }

  double trace = 0.0;

  for (int outer = 0; outer < Hdot.outerSize(); ++outer) {
    for (Eigen::SparseMatrix<double>::InnerIterator it(Hdot, outer); it; ++it) {
      const int i = static_cast<int>(it.row());
      const int j = static_cast<int>(it.col());
      const double hdot_ij = it.value();

      const auto found = inv_columns.find(j);
      if (found == inv_columns.end()) {
        throw std::runtime_error("missing selected inverse column.");
      }

      trace += hdot_ij * found->second[i];
    }
  }

  return trace;
}

// Computes selected inverse columns once and reuses them across multiple Hdot
// matrices that share a common pattern.
//
// This is more representative for a full gradient, where all Hdot_j often share
// the same structural pattern.
class SelectedInverseColumnTraceCache {
public:
  SelectedInverseColumnTraceCache(const Eigen::LDLT<Eigen::MatrixXd> &ldlt,
                                  int n, std::vector<int> needed_columns)
      : n_(n) {
    if (n_ <= 0) {
      throw std::invalid_argument("n must be positive.");
    }

    std::sort(needed_columns.begin(), needed_columns.end());
    needed_columns.erase(
        std::unique(needed_columns.begin(), needed_columns.end()),
        needed_columns.end());

    columns_.reserve(needed_columns.size());
    values_.reserve(needed_columns.size());

    for (int col : needed_columns) {
      if (col < 0 || col >= n_) {
        throw std::out_of_range("needed inverse column out of range.");
      }

      Eigen::VectorXd e = Eigen::VectorXd::Zero(n_);
      e[col] = 1.0;

      columns_.push_back(col);
      values_.push_back(ldlt.solve(e));
    }
  }

  double trace(const Eigen::SparseMatrix<double> &Hdot) const {
    if (Hdot.rows() != n_ || Hdot.cols() != n_) {
      throw std::invalid_argument("Hdot has wrong dimensions.");
    }

    double out = 0.0;

    for (int outer = 0; outer < Hdot.outerSize(); ++outer) {
      for (Eigen::SparseMatrix<double>::InnerIterator it(Hdot, outer); it;
           ++it) {
        const int i = static_cast<int>(it.row());
        const int j = static_cast<int>(it.col());

        const auto pos = std::lower_bound(columns_.begin(), columns_.end(), j);

        if (pos == columns_.end() || *pos != j) {
          throw std::runtime_error(
              "Hdot references inverse column not in cache.");
        }

        const size_t idx =
            static_cast<size_t>(std::distance(columns_.begin(), pos));
        out += it.value() * values_[idx][i];
      }
    }

    return out;
  }

private:
  int n_;
  std::vector<int> columns_;
  std::vector<Eigen::VectorXd> values_;
};

inline std::vector<int> needed_columns_from_sparse_matrices(
    const std::vector<Eigen::SparseMatrix<double>> &matrices) {
  std::vector<int> cols;

  for (const auto &M : matrices) {
    for (int outer = 0; outer < M.outerSize(); ++outer) {
      for (Eigen::SparseMatrix<double>::InnerIterator it(M, outer); it; ++it) {
        cols.push_back(static_cast<int>(it.col()));
      }
    }
  }

  std::sort(cols.begin(), cols.end());
  cols.erase(std::unique(cols.begin(), cols.end()), cols.end());
  return cols;
}

} // namespace laplace
} // namespace quadra
