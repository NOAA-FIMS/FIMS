#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace quadra {
namespace laplace {

// Sparse factorization wrapper for H_uu.
//
// Uses Eigen::SimplicialLDLT for symmetric positive definite sparse Hessians.
// The wrapper provides:
//   - solve(rhs)
//   - selected inverse columns
//   - sparse trace contraction trace(H^{-1} Hdot)
//   - logdet from D
class SparseHuuFactorization {
public:
  explicit SparseHuuFactorization(const Eigen::SparseMatrix<double> &Huu)
      : Huu_(Huu), n_(static_cast<int>(Huu.rows())) {
    if (Huu.rows() != Huu.cols()) {
      throw std::invalid_argument(
          "SparseHuuFactorization: Huu must be square.");
    }
    if (n_ <= 0) {
      throw std::invalid_argument(
          "SparseHuuFactorization: Huu must be nonempty.");
    }

    factor_.compute(Huu_);
    if (factor_.info() != Eigen::Success) {
      throw std::runtime_error(
          "SparseHuuFactorization: SimplicialLDLT factorization failed.");
    }
  }

  Eigen::VectorXd solve(const Eigen::VectorXd &rhs) const {
    if (rhs.size() != n_) {
      throw std::invalid_argument(
          "SparseHuuFactorization::solve: rhs has wrong length.");
    }

    Eigen::VectorXd out = factor_.solve(rhs);
    if (factor_.info() != Eigen::Success) {
      throw std::runtime_error("SparseHuuFactorization::solve failed.");
    }
    return out;
  }

  Eigen::MatrixXd solve_columns(const std::vector<int> &columns) const {
    Eigen::MatrixXd out(n_, static_cast<int>(columns.size()));

    for (int k = 0; k < static_cast<int>(columns.size()); ++k) {
      const int col = columns[static_cast<size_t>(k)];
      if (col < 0 || col >= n_) {
        throw std::out_of_range(
            "SparseHuuFactorization::solve_columns: column out of range.");
      }

      Eigen::VectorXd rhs = Eigen::VectorXd::Zero(n_);
      rhs[col] = 1.0;
      out.col(k) = solve(rhs);
    }

    return out;
  }

  double trace_inverse_times(const Eigen::SparseMatrix<double> &Hdot) const {
    if (Hdot.rows() != n_ || Hdot.cols() != n_) {
      throw std::invalid_argument(
          "trace_inverse_times: Hdot has wrong dimensions.");
    }

    std::vector<int> columns;
    columns.reserve(static_cast<size_t>(Hdot.nonZeros()));

    std::vector<char> seen(static_cast<size_t>(n_), 0);
    for (int outer = 0; outer < Hdot.outerSize(); ++outer) {
      for (Eigen::SparseMatrix<double>::InnerIterator it(Hdot, outer); it;
           ++it) {
        const int col = static_cast<int>(it.col());
        if (!seen[static_cast<size_t>(col)]) {
          seen[static_cast<size_t>(col)] = 1;
          columns.push_back(col);
        }
      }
    }

    if (columns.empty()) {
      return 0.0;
    }

    Eigen::MatrixXd selected = solve_columns(columns);

    double trace = 0.0;
    for (int outer = 0; outer < Hdot.outerSize(); ++outer) {
      for (Eigen::SparseMatrix<double>::InnerIterator it(Hdot, outer); it;
           ++it) {
        const int row = static_cast<int>(it.row());
        const int col = static_cast<int>(it.col());

        int selected_index = -1;
        for (int k = 0; k < static_cast<int>(columns.size()); ++k) {
          if (columns[static_cast<size_t>(k)] == col) {
            selected_index = k;
            break;
          }
        }

        if (selected_index < 0) {
          throw std::runtime_error(
              "trace_inverse_times: internal selected column lookup failed.");
        }

        trace += it.value() * selected(row, selected_index);
      }
    }

    return trace;
  }

  double logdet() const {
    // SimplicialLDLT has vectorD() in Eigen. The determinant of LDLT is
    // product(D), ignoring permutation sign for SPD matrices.
    const auto D = factor_.vectorD();

    double out = 0.0;
    for (int i = 0; i < D.size(); ++i) {
      const double d = D[i];
      if (!(d > 0.0) || !std::isfinite(d)) {
        throw std::runtime_error(
            "SparseHuuFactorization::logdet: nonpositive/nonfinite D entry.");
      }
      out += std::log(d);
    }

    return out;
  }

  int rows() const { return n_; }

  const Eigen::SparseMatrix<double> &matrix() const { return Huu_; }

private:
  Eigen::SparseMatrix<double> Huu_;
  int n_;
  Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> factor_;
};

inline Eigen::SparseMatrix<double> dense_to_sparse(const Eigen::MatrixXd &H,
                                                   double drop_tol = 0.0) {
  if (H.rows() != H.cols()) {
    throw std::invalid_argument("dense_to_sparse: H must be square.");
  }
  if (drop_tol < 0.0) {
    throw std::invalid_argument(
        "dense_to_sparse: drop_tol must be nonnegative.");
  }

  std::vector<Eigen::Triplet<double>> triplets;
  triplets.reserve(static_cast<size_t>(H.rows() * H.cols()));

  for (int i = 0; i < H.rows(); ++i) {
    for (int j = 0; j < H.cols(); ++j) {
      const double v = H(i, j);
      if (std::abs(v) > drop_tol) {
        triplets.emplace_back(i, j, v);
      }
    }
  }

  Eigen::SparseMatrix<double> S(H.rows(), H.cols());
  S.setFromTriplets(triplets.begin(), triplets.end());
  S.makeCompressed();
  return S;
}

} // namespace laplace
} // namespace quadra
