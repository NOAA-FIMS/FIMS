#pragma once

#include <cmath>
#include <stdexcept>
#include <string>

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>

namespace quadra {

class SparseFactorizationCache {
public:
  using SparseMatrix = Eigen::SparseMatrix<double>;
  using Vector = Eigen::VectorXd;

  SparseFactorizationCache() = default;

  void analyze_pattern(const SparseMatrix &H) {
    solver_m.analyzePattern(H);

    if (solver_m.info() != Eigen::Success) {
      analyzed_m = false;
      throw std::runtime_error(
          "SparseFactorizationCache::analyze_pattern failed.");
    }

    analyzed_m = true;
    pattern_rows_m = H.rows();
    pattern_cols_m = H.cols();
    pattern_nnz_m = H.nonZeros();
  }

  void factorize(const SparseMatrix &H) {
    if (!analyzed_m) {
      analyze_pattern(H);
    }

    if (H.rows() != pattern_rows_m || H.cols() != pattern_cols_m) {
      throw std::invalid_argument(
          "SparseFactorizationCache::factorize matrix dimension changed.");
    }

    // This is a soft check. The sparsity count can remain the same even if
    // the pattern changes, so this is not a complete structural guarantee.
    // It is still useful for catching common accidental pattern changes.
    if (H.nonZeros() != pattern_nnz_m) {
      throw std::invalid_argument(
          "SparseFactorizationCache::factorize matrix nnz changed.");
    }

    solver_m.factorize(H);

    if (solver_m.info() != Eigen::Success) {
      factorized_m = false;
      throw std::runtime_error(
          "SparseFactorizationCache::factorize numeric factorization failed.");
    }

    factorized_m = true;
  }

  void compute(const SparseMatrix &H) {
    solver_m.compute(H);

    if (solver_m.info() != Eigen::Success) {
      analyzed_m = false;
      factorized_m = false;
      throw std::runtime_error("SparseFactorizationCache::compute failed.");
    }

    analyzed_m = true;
    factorized_m = true;
    pattern_rows_m = H.rows();
    pattern_cols_m = H.cols();
    pattern_nnz_m = H.nonZeros();
  }

  Vector solve(const Vector &b) const {
    if (!factorized_m) {
      throw std::runtime_error(
          "SparseFactorizationCache::solve called before factorize.");
    }

    Vector x = solver_m.solve(b);

    if (solver_m.info() != Eigen::Success) {
      throw std::runtime_error("SparseFactorizationCache::solve failed.");
    }

    return x;
  }

  double logdet() const {
    if (!factorized_m) {
      throw std::runtime_error(
          "SparseFactorizationCache::logdet called before factorize.");
    }

    const SparseMatrix L = solver_m.matrixL();

    double out = 0.0;

    for (int k = 0; k < L.outerSize(); ++k) {
      for (SparseMatrix::InnerIterator it(L, k); it; ++it) {
        if (it.row() == it.col()) {
          const double d = it.value();

          if (!(d > 0.0) || !std::isfinite(d)) {
            throw std::runtime_error(
                "SparseFactorizationCache::logdet nonpositive diagonal.");
          }

          out += 2.0 * std::log(d);
        }
      }
    }

    return out;
  }

  bool analyzed() const { return analyzed_m; }

  bool factorized() const { return factorized_m; }

  int rows() const { return pattern_rows_m; }

  int cols() const { return pattern_cols_m; }

  int nonzeros() const { return pattern_nnz_m; }

private:
  Eigen::SimplicialLLT<SparseMatrix> solver_m;

  bool analyzed_m = false;
  bool factorized_m = false;

  int pattern_rows_m = 0;
  int pattern_cols_m = 0;
  int pattern_nnz_m = 0;
};

class SparseLDLTFactorizationCache {
public:
  using SparseMatrix = Eigen::SparseMatrix<double>;
  using Vector = Eigen::VectorXd;
  using Matrix = Eigen::MatrixXd;

  SparseLDLTFactorizationCache() = default;

  void analyze_pattern(const SparseMatrix &H) {
    solver_m.analyzePattern(H);

    if (solver_m.info() != Eigen::Success) {
      analyzed_m = false;
      throw std::runtime_error(
          "SparseLDLTFactorizationCache::analyze_pattern failed.");
    }

    analyzed_m = true;
    pattern_rows_m = H.rows();
    pattern_cols_m = H.cols();
    pattern_nnz_m = H.nonZeros();
  }

  void factorize(const SparseMatrix &H) {
    if (!analyzed_m) {
      analyze_pattern(H);
    }

    if (H.rows() != pattern_rows_m || H.cols() != pattern_cols_m) {
      throw std::invalid_argument(
          "SparseLDLTFactorizationCache::factorize matrix dimension changed.");
    }

    if (H.nonZeros() != pattern_nnz_m) {
      throw std::invalid_argument(
          "SparseLDLTFactorizationCache::factorize matrix nnz changed.");
    }

    solver_m.factorize(H);

    if (solver_m.info() != Eigen::Success) {
      factorized_m = false;
      throw std::runtime_error("SparseLDLTFactorizationCache::factorize "
                               "numeric factorization failed.");
    }

    factorized_m = true;
  }

  void compute(const SparseMatrix &H) {
    solver_m.compute(H);

    if (solver_m.info() != Eigen::Success) {
      analyzed_m = false;
      factorized_m = false;
      throw std::runtime_error("SparseLDLTFactorizationCache::compute failed.");
    }

    analyzed_m = true;
    factorized_m = true;
    pattern_rows_m = H.rows();
    pattern_cols_m = H.cols();
    pattern_nnz_m = H.nonZeros();
  }

  Vector solve(const Vector &b) const {
    if (!factorized_m) {
      throw std::runtime_error(
          "SparseLDLTFactorizationCache::solve called before factorize.");
    }

    Vector x = solver_m.solve(b);

    if (solver_m.info() != Eigen::Success) {
      throw std::runtime_error("SparseLDLTFactorizationCache::solve failed.");
    }

    return x;
  }

  Matrix solve(const Matrix &B) const {
    if (!factorized_m) {
      throw std::runtime_error("SparseLDLTFactorizationCache::solve matrix "
                               "called before factorize.");
    }

    Matrix X = solver_m.solve(B);

    if (solver_m.info() != Eigen::Success) {
      throw std::runtime_error(
          "SparseLDLTFactorizationCache::solve matrix failed.");
    }

    return X;
  }

  bool analyzed() const { return analyzed_m; }
  bool factorized() const { return factorized_m; }
  int rows() const { return pattern_rows_m; }
  int cols() const { return pattern_cols_m; }
  int nonzeros() const { return pattern_nnz_m; }

private:
  Eigen::SimplicialLDLT<SparseMatrix> solver_m;

  bool analyzed_m = false;
  bool factorized_m = false;

  int pattern_rows_m = 0;
  int pattern_cols_m = 0;
  int pattern_nnz_m = 0;
};

inline double sparse_logdet_compute(const Eigen::SparseMatrix<double> &H) {
  SparseFactorizationCache cache;
  cache.compute(H);
  return cache.logdet();
}

} // namespace quadra
