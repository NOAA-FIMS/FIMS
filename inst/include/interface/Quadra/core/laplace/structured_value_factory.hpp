#pragma once

#include "structure_detector.hpp"
#include "structured_value_backend.hpp"

#include <Eigen/Sparse>

#include <stdexcept>
#include <variant>

namespace quadra {
namespace laplace {

using StructuredValues = std::variant<DiagonalValues, TridiagonalValues,
                                      BandedValues, SparseMatrixValues>;

inline DiagonalValues
extract_diagonal_values(const Eigen::SparseMatrix<double> &H) {
  if (H.rows() != H.cols()) {
    throw std::invalid_argument(
        "Diagonal value extraction requires square matrix");
  }

  DiagonalValues values;
  values.diag = Eigen::VectorXd::Zero(H.rows());

  for (int i = 0; i < H.rows(); ++i) {
    values.diag[i] = H.coeff(i, i);
  }

  return values;
}

inline TridiagonalValues
extract_tridiagonal_values(const Eigen::SparseMatrix<double> &H) {
  if (H.rows() != H.cols()) {
    throw std::invalid_argument(
        "Tridiagonal value extraction requires square matrix");
  }

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

  return values;
}

inline BandedValues extract_banded_values(const Eigen::SparseMatrix<double> &H,
                                          const int bandwidth) {
  if (H.rows() != H.cols()) {
    throw std::invalid_argument(
        "Banded value extraction requires square matrix");
  }
  if (bandwidth < 0) {
    throw std::invalid_argument(
        "Banded value extraction requires non-negative bandwidth");
  }

  const int n = static_cast<int>(H.rows());
  const int bw = std::min(bandwidth, std::max(0, n - 1));

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
      values.lower_bands[static_cast<std::size_t>(d - 1)][j] = H.coeff(i, j);
    }
  }

  return values;
}

inline StructuredValues
extract_structured_values(const Eigen::SparseMatrix<double> &H,
                          const BackendRecommendation &rec) {
  switch (rec.backend) {
  case LaplaceBackendKind::Diagonal:
    return extract_diagonal_values(H);

  case LaplaceBackendKind::Tridiagonal:
    return extract_tridiagonal_values(H);

  case LaplaceBackendKind::Banded:
    return extract_banded_values(H, rec.bandwidth);

  case LaplaceBackendKind::SparseLDLT:
  case LaplaceBackendKind::DenseLDLT: {
    SparseMatrixValues values;
    values.H = H;
    return values;
  }
  }

  throw std::invalid_argument("Unknown backend recommendation");
}

inline void
update_diagonal_values_from_hessian(DiagonalValues &values,
                                    const Eigen::SparseMatrix<double> &H) {
  if (H.rows() != H.cols()) {
    throw std::invalid_argument("Diagonal value update requires square matrix");
  }

  const int n = static_cast<int>(H.rows());

  if (values.diag.size() != n) {
    values.diag = Eigen::VectorXd::Zero(n);
  }

  for (int i = 0; i < n; ++i) {
    values.diag[i] = H.coeff(i, i);
  }
}

inline void
update_tridiagonal_values_from_hessian(TridiagonalValues &values,
                                       const Eigen::SparseMatrix<double> &H) {
  if (H.rows() != H.cols()) {
    throw std::invalid_argument(
        "Tridiagonal value update requires square matrix");
  }

  const int n = static_cast<int>(H.rows());

  if (values.diag.size() != n) {
    values.diag = Eigen::VectorXd::Zero(n);
  }

  if (values.offdiag.size() != std::max(0, n - 1)) {
    values.offdiag = Eigen::VectorXd::Zero(std::max(0, n - 1));
  }

  for (int i = 0; i < n; ++i) {
    values.diag[i] = H.coeff(i, i);

    if (i > 0) {
      values.offdiag[i - 1] = H.coeff(i, i - 1);
    }
  }
}

inline void
update_banded_values_from_hessian(BandedValues &values,
                                  const Eigen::SparseMatrix<double> &H,
                                  const int bandwidth) {
  if (H.rows() != H.cols()) {
    throw std::invalid_argument("Banded value update requires square matrix");
  }

  if (bandwidth < 0) {
    throw std::invalid_argument(
        "Banded value update requires non-negative bandwidth");
  }

  const int n = static_cast<int>(H.rows());
  const int bw = std::min(bandwidth, std::max(0, n - 1));

  values.bandwidth = bw;

  if (values.diag.size() != n) {
    values.diag = Eigen::VectorXd::Zero(n);
  }

  if (static_cast<int>(values.lower_bands.size()) != bw) {
    values.lower_bands.resize(static_cast<std::size_t>(bw));
  }

  for (int d = 1; d <= bw; ++d) {
    const int expected = std::max(0, n - d);
    Eigen::VectorXd &band = values.lower_bands[static_cast<std::size_t>(d - 1)];

    if (band.size() != expected) {
      band = Eigen::VectorXd::Zero(expected);
    }
  }

  for (int i = 0; i < n; ++i) {
    values.diag[i] = H.coeff(i, i);

    for (int d = 1; d <= bw; ++d) {
      const int j = i - d;
      if (j < 0)
        continue;

      values.lower_bands[static_cast<std::size_t>(d - 1)][j] = H.coeff(i, j);
    }
  }
}

inline void
update_structured_values_from_hessian(StructuredValues &values,
                                      const Eigen::SparseMatrix<double> &H,
                                      const BackendRecommendation &rec) {
  switch (rec.backend) {
  case LaplaceBackendKind::Diagonal:
    if (!std::holds_alternative<DiagonalValues>(values)) {
      values = DiagonalValues();
    }
    update_diagonal_values_from_hessian(std::get<DiagonalValues>(values), H);
    return;

  case LaplaceBackendKind::Tridiagonal:
    if (!std::holds_alternative<TridiagonalValues>(values)) {
      values = TridiagonalValues();
    }
    update_tridiagonal_values_from_hessian(std::get<TridiagonalValues>(values),
                                           H);
    return;

  case LaplaceBackendKind::Banded:
    if (!std::holds_alternative<BandedValues>(values)) {
      values = BandedValues();
    }
    update_banded_values_from_hessian(std::get<BandedValues>(values), H,
                                      rec.bandwidth);
    return;

  case LaplaceBackendKind::SparseLDLT:
  case LaplaceBackendKind::DenseLDLT:
    if (!std::holds_alternative<SparseMatrixValues>(values)) {
      values = SparseMatrixValues();
    }
    std::get<SparseMatrixValues>(values).H = H;
    return;
  }

  throw std::invalid_argument("Unknown backend recommendation");
}

inline double logdet_structured_values(const StructuredValues &values) {
  return std::visit(
      [](const auto &v) -> double {
        using T = std::decay_t<decltype(v)>;

        if constexpr (std::is_same_v<T, DiagonalValues>) {
          return logdet_diagonal_values(v);
        } else if constexpr (std::is_same_v<T, TridiagonalValues>) {
          return logdet_tridiagonal_values_ldlt(v);
        } else if constexpr (std::is_same_v<T, BandedValues>) {
          return logdet_banded_values_ldlt(v);
        } else if constexpr (std::is_same_v<T, SparseMatrixValues>) {
          return logdet_sparse_matrix_values_ldlt(v);
        } else {
          throw std::invalid_argument("Unsupported structured value type");
        }
      },
      values);
}

} // namespace laplace
} // namespace quadra
