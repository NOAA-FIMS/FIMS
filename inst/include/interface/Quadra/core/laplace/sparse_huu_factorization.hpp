#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace quadra {
namespace laplace {

class TakahashiSelectedInverse {
public:
  TakahashiSelectedInverse(
      const Eigen::SparseMatrix<double> &unit_lower,
      const Eigen::VectorXd &diagonal,
      const Eigen::PermutationMatrix<Eigen::Dynamic, Eigen::Dynamic, int>
          &permutation)
      : n_(static_cast<int>(diagonal.size())),
        original_to_factor_(static_cast<size_t>(n_), -1) {
    if (unit_lower.rows() != n_ || unit_lower.cols() != n_)
      throw std::invalid_argument(
          "TakahashiSelectedInverse: factor dimensions do not match.");
    for (int original_index = 0; original_index < n_; ++original_index)
      original_to_factor_[static_cast<size_t>(original_index)] =
          permutation.indices()[original_index];

    values_.reserve(static_cast<size_t>(unit_lower.nonZeros() * 2));
    for (int i = n_ - 1; i >= 0; --i) {
      std::vector<std::pair<int, double>> children;
      for (Eigen::SparseMatrix<double>::InnerIterator it(unit_lower, i); it;
           ++it) {
        if (it.row() > i)
          children.emplace_back(static_cast<int>(it.row()), it.value());
      }
      for (const auto &child_j : children) {
        double sum = 0.0;
        for (const auto &child_k : children)
          sum += child_k.second * factor_value(child_k.first, child_j.first);
        values_.emplace(pair_key(i, child_j.first), -sum);
      }
      const double d = diagonal[i];
      if (!(d > 0.0) || !std::isfinite(d))
        throw std::runtime_error(
            "TakahashiSelectedInverse: invalid LDLT diagonal.");
      double inverse_diagonal = 1.0 / d;
      for (const auto &child : children)
        inverse_diagonal -= child.second * factor_value(i, child.first);
      values_.emplace(pair_key(i, i), inverse_diagonal);
    }
  }

  bool contains(int original_row, int original_col) const {
    validate_original_index(original_row);
    validate_original_index(original_col);
    return values_.find(pair_key(
               original_to_factor_[static_cast<size_t>(original_row)],
               original_to_factor_[static_cast<size_t>(original_col)])) !=
           values_.end();
  }

  double value(int original_row, int original_col) const {
    validate_original_index(original_row);
    validate_original_index(original_col);
    return factor_value(original_to_factor_[static_cast<size_t>(original_row)],
                        original_to_factor_[static_cast<size_t>(original_col)]);
  }

  double
  trace_inverse_times(const Eigen::SparseMatrix<double> &derivative) const {
    if (derivative.rows() != n_ || derivative.cols() != n_)
      throw std::invalid_argument(
          "TakahashiSelectedInverse::trace_inverse_times: wrong dimensions.");
    double trace = 0.0;
    for (int outer = 0; outer < derivative.outerSize(); ++outer)
      for (Eigen::SparseMatrix<double>::InnerIterator it(derivative, outer); it;
           ++it)
        trace += it.value() * value(it.row(), it.col());
    return trace;
  }

  bool supports(const Eigen::SparseMatrix<double> &derivative) const {
    if (derivative.rows() != n_ || derivative.cols() != n_)
      return false;
    for (int outer = 0; outer < derivative.outerSize(); ++outer)
      for (Eigen::SparseMatrix<double>::InnerIterator it(derivative, outer); it;
           ++it)
        if (!contains(it.row(), it.col()))
          return false;
    return true;
  }

  std::vector<double> traces_inverse_times(
      const std::vector<Eigen::SparseMatrix<double>> &derivatives) const {
    std::vector<double> traces;
    traces.reserve(derivatives.size());
    for (const auto &derivative : derivatives)
      traces.push_back(trace_inverse_times(derivative));
    return traces;
  }

  size_t nonzeros() const { return values_.size(); }

private:
  static std::uint64_t pair_key(int row, int col) {
    if (row > col)
      std::swap(row, col);
    return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(row)) << 32) |
           static_cast<std::uint32_t>(col);
  }

  double factor_value(int row, int col) const {
    const auto found = values_.find(pair_key(row, col));
    if (found == values_.end())
      throw std::out_of_range(
          "TakahashiSelectedInverse: requested entry is outside the "
          "factor sparsity pattern.");
    return found->second;
  }

  void validate_original_index(int index) const {
    if (index < 0 || index >= n_)
      throw std::out_of_range(
          "TakahashiSelectedInverse: original index out of range.");
  }

  int n_;
  std::vector<int> original_to_factor_;
  std::unordered_map<std::uint64_t, double> values_;
};

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

    initialize_tridiagonal_selected_inverse();
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

  Eigen::MatrixXd solve(const Eigen::MatrixXd &rhs) const {
    if (rhs.rows() != n_) {
      throw std::invalid_argument(
          "SparseHuuFactorization::solve: rhs has wrong row count.");
    }
    Eigen::MatrixXd out = factor_.solve(rhs);
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

    // For a tridiagonal SPD matrix, the diagonal and first off-diagonal of
    // Huu^{-1} follow directly from the LDL' factors.  These are the only
    // inverse entries needed when Hdot has the same sparsity, so the trace is
    // O(n) rather than n sparse solves followed by a dense n-by-n workspace.
    if (has_tridiagonal_selected_inverse_) {
      bool hdot_is_tridiagonal = true;
      double trace = 0.0;
      for (int outer = 0; outer < Hdot.outerSize(); ++outer) {
        for (Eigen::SparseMatrix<double>::InnerIterator it(Hdot, outer); it;
             ++it) {
          const int row = static_cast<int>(it.row());
          const int col = static_cast<int>(it.col());
          const int distance = std::abs(row - col);
          if (distance > 1 && it.value() != 0.0) {
            hdot_is_tridiagonal = false;
            break;
          }
          if (distance == 0) {
            trace += it.value() * inverse_diagonal_[row];
          } else if (distance == 1) {
            trace +=
                it.value() * inverse_first_off_diagonal_[std::min(row, col)];
          }
        }
        if (!hdot_is_tridiagonal) {
          break;
        }
      }
      if (hdot_is_tridiagonal) {
        return trace;
      }
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

  // Exact trace contraction with O(n) working memory. Unlike
  // trace_inverse_times(), this does not retain all requested inverse columns.
  // It is intended for large derivative matrices whose nonzero pattern touches
  // most latent-variable columns.
  double
  trace_inverse_times_streaming(const Eigen::SparseMatrix<double> &Hdot) const {
    if (Hdot.rows() != n_ || Hdot.cols() != n_) {
      throw std::invalid_argument(
          "trace_inverse_times_streaming: Hdot has wrong dimensions.");
    }

    double trace = 0.0;
    Eigen::VectorXd rhs = Eigen::VectorXd::Zero(n_);
    for (int col = 0; col < Hdot.outerSize(); ++col) {
      bool has_nonzero = false;
      for (Eigen::SparseMatrix<double>::InnerIterator it(Hdot, col); it; ++it) {
        has_nonzero = true;
        break;
      }
      if (!has_nonzero)
        continue;

      rhs[col] = 1.0;
      const Eigen::VectorXd inverse_column = solve(rhs);
      rhs[col] = 0.0;
      for (Eigen::SparseMatrix<double>::InnerIterator it(Hdot, col); it; ++it)
        trace += it.value() * inverse_column[it.row()];
    }
    return trace;
  }

  std::vector<double> traces_inverse_times_streaming(
      const std::vector<Eigen::SparseMatrix<double>> &derivatives) const {
    for (const auto &derivative : derivatives) {
      if (derivative.rows() != n_ || derivative.cols() != n_) {
        throw std::invalid_argument(
            "traces_inverse_times_streaming: derivative has wrong dimensions.");
      }
    }
    std::vector<double> traces(derivatives.size(), 0.0);
    Eigen::VectorXd rhs = Eigen::VectorXd::Zero(n_);
    for (int col = 0; col < n_; ++col) {
      bool needed = false;
      for (const auto &derivative : derivatives) {
        if (col < derivative.outerSize() &&
            derivative.innerVector(col).nonZeros() > 0) {
          needed = true;
          break;
        }
      }
      if (!needed)
        continue;
      rhs[col] = 1.0;
      const Eigen::VectorXd inverse_column = solve(rhs);
      rhs[col] = 0.0;
      for (size_t j = 0; j < derivatives.size(); ++j) {
        if (col >= derivatives[j].outerSize())
          continue;
        for (Eigen::SparseMatrix<double>::InnerIterator it(derivatives[j], col);
             it; ++it)
          traces[j] += it.value() * inverse_column[it.row()];
      }
    }
    return traces;
  }

  TakahashiSelectedInverse selected_inverse() const {
    Eigen::SparseMatrix<double> unit_lower = factor_.matrixL();
    return TakahashiSelectedInverse(unit_lower, factor_.vectorD(),
                                    factor_.permutationP());
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

  bool has_tridiagonal_selected_inverse() const {
    return has_tridiagonal_selected_inverse_;
  }

  double tridiagonal_selected_inverse(int row, int col) const {
    if (!has_tridiagonal_selected_inverse_) {
      throw std::logic_error("tridiagonal selected inverse is not available");
    }
    if (row < 0 || col < 0 || row >= n_ || col >= n_) {
      throw std::out_of_range("selected inverse index out of range");
    }
    const int distance = std::abs(row - col);
    if (distance == 0) {
      return inverse_diagonal_[row];
    }
    if (distance == 1) {
      return inverse_first_off_diagonal_[std::min(row, col)];
    }
    throw std::invalid_argument(
        "only tridiagonal selected-inverse entries are cached");
  }

  const Eigen::SparseMatrix<double> &matrix() const { return Huu_; }

private:
  void initialize_tridiagonal_selected_inverse() {
    for (int outer = 0; outer < Huu_.outerSize(); ++outer) {
      for (Eigen::SparseMatrix<double>::InnerIterator it(Huu_, outer); it;
           ++it) {
        if (std::abs(it.row() - it.col()) > 1 && it.value() != 0.0) {
          return;
        }
      }
    }

    Eigen::VectorXd diagonal(n_);
    Eigen::VectorXd multiplier = Eigen::VectorXd::Zero(std::max(0, n_ - 1));
    diagonal[0] = Huu_.coeff(0, 0);
    if (!(diagonal[0] > 0.0) || !std::isfinite(diagonal[0])) {
      return;
    }

    for (int i = 1; i < n_; ++i) {
      const double lower = Huu_.coeff(i, i - 1);
      const double upper = Huu_.coeff(i - 1, i);
      const double scale = 1.0 + std::max(std::abs(lower), std::abs(upper));
      if (std::abs(lower - upper) > 1e-12 * scale) {
        return;
      }
      const double off_diagonal = 0.5 * (lower + upper);
      multiplier[i - 1] = off_diagonal / diagonal[i - 1];
      diagonal[i] = Huu_.coeff(i, i) -
                    multiplier[i - 1] * multiplier[i - 1] * diagonal[i - 1];
      if (!(diagonal[i] > 0.0) || !std::isfinite(diagonal[i])) {
        return;
      }
    }

    inverse_diagonal_.resize(n_);
    inverse_first_off_diagonal_.resize(std::max(0, n_ - 1));
    inverse_diagonal_[n_ - 1] = 1.0 / diagonal[n_ - 1];
    for (int i = n_ - 2; i >= 0; --i) {
      inverse_first_off_diagonal_[i] =
          -multiplier[i] * inverse_diagonal_[i + 1];
      inverse_diagonal_[i] =
          1.0 / diagonal[i] - multiplier[i] * inverse_first_off_diagonal_[i];
    }
    has_tridiagonal_selected_inverse_ = true;
  }

  Eigen::SparseMatrix<double> Huu_;
  int n_;
  Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> factor_;
  bool has_tridiagonal_selected_inverse_ = false;
  Eigen::VectorXd inverse_diagonal_;
  Eigen::VectorXd inverse_first_off_diagonal_;
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
