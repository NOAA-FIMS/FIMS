#pragma once

#include "hessian_structure.hpp"
#include "structure_detector.hpp"
#include "structured_value_backend.hpp"

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace quadra {
namespace laplace {

class LaplaceBackend {
public:
  virtual ~LaplaceBackend() = default;

  virtual const char *name() const = 0;

  virtual void analyze_pattern(const Eigen::SparseMatrix<double> &H) {
    analyzed_ = true;
    rows_ = static_cast<int>(H.rows());
    cols_ = static_cast<int>(H.cols());
  }

  virtual void factorize(const Eigen::SparseMatrix<double> &H) = 0;

  virtual double logdet() const = 0;

  virtual bool is_spd() const = 0;

  virtual int rows() const { return rows_; }
  virtual int cols() const { return cols_; }
  virtual bool analyzed() const { return analyzed_; }

protected:
  bool analyzed_ = false;
  int rows_ = 0;
  int cols_ = 0;
};

class DiagonalBackend final : public LaplaceBackend {
public:
  const char *name() const override { return "diagonal"; }

  void analyze_pattern(const Eigen::SparseMatrix<double> &H) override {
    LaplaceBackend::analyze_pattern(H);
    if (H.rows() != H.cols()) {
      throw std::invalid_argument("DiagonalBackend requires square matrix");
    }
  }

  void factorize(const Eigen::SparseMatrix<double> &H) override {
    analyze_pattern(H);

    try {
      DiagonalValues values;
      values.diag = Eigen::VectorXd::Zero(H.rows());

      for (int i = 0; i < H.rows(); ++i) {
        values.diag[i] = H.coeff(i, i);
      }

      logdet_ = logdet_diagonal_values(values);
      spd_ = true;
    } catch (...) {
      logdet_ = std::numeric_limits<double>::quiet_NaN();
      spd_ = false;
    }
  }

  double logdet() const override { return logdet_; }
  bool is_spd() const override { return spd_; }

private:
  double logdet_ = 0.0;
  bool spd_ = false;
};

class TridiagonalBackend final : public LaplaceBackend {
public:
  const char *name() const override { return "tridiagonal"; }

  void analyze_pattern(const Eigen::SparseMatrix<double> &H) override {
    LaplaceBackend::analyze_pattern(H);
    if (H.rows() != H.cols()) {
      throw std::invalid_argument("TridiagonalBackend requires square matrix");
    }
  }

  void factorize(const Eigen::SparseMatrix<double> &H) override {
    analyze_pattern(H);

    try {
      const int n = static_cast<int>(H.rows());

      TridiagonalValues values;
      values.diag = Eigen::VectorXd::Zero(n);
      values.offdiag = Eigen::VectorXd::Zero(std::max(0, n - 1));

      for (int i = 0; i < n; ++i) {
        values.diag[i] = H.coeff(i, i);

        if (i > 0) {
          values.offdiag[i - 1] = H.coeff(i, i - 1);
        }
      }

      logdet_ = logdet_tridiagonal_values_ldlt(values);
      spd_ = true;
    } catch (...) {
      logdet_ = std::numeric_limits<double>::quiet_NaN();
      spd_ = false;
    }
  }

  double logdet() const override { return logdet_; }
  bool is_spd() const override { return spd_; }

private:
  double logdet_ = 0.0;
  bool spd_ = false;
};

class BandedBackend final : public LaplaceBackend {
public:
  explicit BandedBackend(int bandwidth) : bandwidth_(bandwidth) {
    if (bandwidth_ < 0) {
      throw std::invalid_argument(
          "BandedBackend bandwidth must be non-negative");
    }
  }

  const char *name() const override { return "banded"; }

  int bandwidth() const { return bandwidth_; }

  void analyze_pattern(const Eigen::SparseMatrix<double> &H) override {
    LaplaceBackend::analyze_pattern(H);
    if (H.rows() != H.cols()) {
      throw std::invalid_argument("BandedBackend requires square matrix");
    }

    // Validate that the pattern fits the declared band.
    for (int outer = 0; outer < H.outerSize(); ++outer) {
      for (Eigen::SparseMatrix<double>::InnerIterator it(H, outer); it; ++it) {
        const int i = static_cast<int>(it.row());
        const int j = static_cast<int>(it.col());
        if (std::abs(i - j) > bandwidth_ && std::abs(it.value()) > 0.0) {
          throw std::invalid_argument(
              "BandedBackend pattern exceeds bandwidth");
        }
      }
    }
  }

  void factorize(const Eigen::SparseMatrix<double> &H) override {
    analyze_pattern(H);

    try {
      const int n = static_cast<int>(H.rows());
      const int bw = std::min(bandwidth_, std::max(0, n - 1));

      BandedValues values;
      values.bandwidth = bw;
      values.diag = Eigen::VectorXd::Zero(n);
      values.lower_bands.resize(static_cast<std::size_t>(bw));

      for (int d = 1; d <= bw; ++d) {
        values.lower_bands[static_cast<std::size_t>(d - 1)] =
            Eigen::VectorXd::Zero(std::max(0, n - d));
      }

      for (int i = 0; i < n; ++i) {
        values.diag[i] = H.coeff(i, i);

        for (int d = 1; d <= bw; ++d) {
          const int j = i - d;
          if (j < 0)
            continue;

          values.lower_bands[static_cast<std::size_t>(d - 1)][j] =
              H.coeff(i, j);
        }
      }

      logdet_ = logdet_banded_values_ldlt(values);
      spd_ = true;
    } catch (...) {
      logdet_ = std::numeric_limits<double>::quiet_NaN();
      spd_ = false;
    }
  }

  double logdet() const override { return logdet_; }
  bool is_spd() const override { return spd_; }

private:
  int bandwidth_ = 0;
  double logdet_ = 0.0;
  bool spd_ = false;
};

class SparseLDLTBackend final : public LaplaceBackend {
public:
  const char *name() const override { return "sparse_ldlt"; }

  void analyze_pattern(const Eigen::SparseMatrix<double> &H) override {
    LaplaceBackend::analyze_pattern(H);
    if (H.rows() != H.cols()) {
      throw std::invalid_argument("SparseLDLTBackend requires square matrix");
    }

    Eigen::SparseMatrix<double> canonical = H;
    canonical.makeCompressed();

    ldlt_.analyzePattern(canonical);
    analyzed_ = true;
    symbolic_ready_ = true;
  }

  void factorize(const Eigen::SparseMatrix<double> &H) override {
    Eigen::SparseMatrix<double> canonical = H;
    canonical.makeCompressed();

    if (!symbolic_ready_) {
      analyze_pattern(canonical);
    }

    ldlt_.factorize(canonical);

    if (ldlt_.info() != Eigen::Success) {
      spd_ = false;
      logdet_ = std::numeric_limits<double>::quiet_NaN();
      return;
    }

    const auto &D = ldlt_.vectorD();
    logdet_ = 0.0;
    spd_ = true;

    for (int i = 0; i < D.size(); ++i) {
      if (!(D[i] > 0.0) || !std::isfinite(D[i])) {
        spd_ = false;
        logdet_ = std::numeric_limits<double>::quiet_NaN();
        return;
      }
      logdet_ += std::log(D[i]);
    }
  }

  double logdet() const override { return logdet_; }
  bool is_spd() const override { return spd_; }

private:
  Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> ldlt_;
  bool symbolic_ready_ = false;
  double logdet_ = 0.0;
  bool spd_ = false;
};

class DenseLDLTBackend final : public LaplaceBackend {
public:
  const char *name() const override { return "dense_ldlt"; }

  void analyze_pattern(const Eigen::SparseMatrix<double> &H) override {
    LaplaceBackend::analyze_pattern(H);
    if (H.rows() != H.cols()) {
      throw std::invalid_argument("DenseLDLTBackend requires square matrix");
    }
  }

  void factorize(const Eigen::SparseMatrix<double> &H) override {
    analyze_pattern(H);

    Eigen::MatrixXd dense = Eigen::MatrixXd(H);
    Eigen::LDLT<Eigen::MatrixXd> ldlt(dense);

    if (ldlt.info() != Eigen::Success) {
      spd_ = false;
      logdet_ = std::numeric_limits<double>::quiet_NaN();
      return;
    }

    const auto &D = ldlt.vectorD();
    logdet_ = 0.0;
    spd_ = true;

    for (int i = 0; i < D.size(); ++i) {
      if (!(D[i] > 0.0) || !std::isfinite(D[i])) {
        spd_ = false;
        logdet_ = std::numeric_limits<double>::quiet_NaN();
        return;
      }
      logdet_ += std::log(D[i]);
    }
  }

  double logdet() const override { return logdet_; }
  bool is_spd() const override { return spd_; }

private:
  double logdet_ = 0.0;
  bool spd_ = false;
};

} // namespace laplace
} // namespace quadra
