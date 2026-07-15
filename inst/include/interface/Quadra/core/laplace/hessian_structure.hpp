#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>

namespace quadra {
namespace laplace {

enum class HessianStructure {
  Diagonal,
  Tridiagonal,
  Banded,
  SparsePattern,
  Dense
};

inline const char *ToString(const HessianStructure s) {
  switch (s) {
  case HessianStructure::Diagonal:
    return "diagonal";
  case HessianStructure::Tridiagonal:
    return "tridiagonal";
  case HessianStructure::Banded:
    return "banded";
  case HessianStructure::SparsePattern:
    return "sparse_pattern";
  case HessianStructure::Dense:
    return "dense";
  }
  return "unknown";
}

struct StructureInfo {
  int rows = 0;
  int cols = 0;
  int nnz = 0;
  int diagonal_nnz = 0;
  int offdiagonal_nnz = 0;
  int max_bandwidth = 0;
  int max_row_nnz = 0;
  double fill_ratio = 0.0;
  double max_abs_asymmetry = 0.0;
  bool square = false;
  bool structurally_symmetric = true;
  bool numerically_symmetric = true;
  HessianStructure detected = HessianStructure::Dense;
};

struct StructureOptions {
  double zero_tol = 1e-12;
  double symmetry_tol = 1e-10;

  // If max_bandwidth <= this, choose a banded backend.
  int max_banded_width = 64;

  // If fill ratio is above this, dense is usually cheaper/simpler.
  double dense_fill_ratio = 0.25;
};

inline StructureInfo
InspectHessianStructure(const Eigen::SparseMatrix<double> &H,
                        const StructureOptions &options = StructureOptions()) {
  StructureInfo info;
  info.rows = static_cast<int>(H.rows());
  info.cols = static_cast<int>(H.cols());
  info.square = (info.rows == info.cols);

  if (!info.square) {
    throw std::invalid_argument(
        "Hessian structure inspection requires a square matrix");
  }

  Eigen::SparseMatrix<double> canonical = H;
  canonical.makeCompressed();

  int max_row_nnz = 0;
  int current_outer_count = 0;
  int current_outer = -1;

  // For col-major sparse matrices, outer index is column. Row nnz is counted
  // separately below.
  Eigen::VectorXi row_counts = Eigen::VectorXi::Zero(info.rows);

  for (int outer = 0; outer < canonical.outerSize(); ++outer) {
    current_outer_count = 0;

    for (Eigen::SparseMatrix<double>::InnerIterator it(canonical, outer); it;
         ++it) {
      const int i = static_cast<int>(it.row());
      const int j = static_cast<int>(it.col());
      const double v = it.value();

      if (std::abs(v) <= options.zero_tol) {
        continue;
      }

      ++info.nnz;
      ++current_outer_count;
      ++row_counts[i];

      if (i == j) {
        ++info.diagonal_nnz;
      } else {
        ++info.offdiagonal_nnz;
      }

      info.max_bandwidth = std::max(info.max_bandwidth, std::abs(i - j));
    }

    (void)current_outer;
  }

  for (int i = 0; i < row_counts.size(); ++i) {
    max_row_nnz = std::max(max_row_nnz, row_counts[i]);
  }
  info.max_row_nnz = max_row_nnz;

  const double total =
      static_cast<double>(info.rows) * static_cast<double>(info.cols);
  info.fill_ratio = total > 0.0 ? static_cast<double>(info.nnz) / total : 0.0;

  // Numeric symmetry check. This is O(nnz * lookup), acceptable for detection
  // and tests. Production evaluators can cache this after pattern discovery.
  info.max_abs_asymmetry = 0.0;
  for (int outer = 0; outer < canonical.outerSize(); ++outer) {
    for (Eigen::SparseMatrix<double>::InnerIterator it(canonical, outer); it;
         ++it) {
      const int i = static_cast<int>(it.row());
      const int j = static_cast<int>(it.col());
      const double v = it.value();

      if (std::abs(v) <= options.zero_tol) {
        continue;
      }

      const double vt = canonical.coeff(j, i);
      const double diff = std::abs(v - vt);
      info.max_abs_asymmetry = std::max(info.max_abs_asymmetry, diff);

      if (std::abs(vt) <= options.zero_tol) {
        info.structurally_symmetric = false;
      }
    }
  }

  info.numerically_symmetric = info.max_abs_asymmetry <= options.symmetry_tol;

  if (info.max_bandwidth == 0) {
    info.detected = HessianStructure::Diagonal;
  } else if (info.max_bandwidth == 1) {
    info.detected = HessianStructure::Tridiagonal;
  } else if (info.max_bandwidth <= options.max_banded_width &&
             info.fill_ratio < options.dense_fill_ratio) {
    info.detected = HessianStructure::Banded;
  } else if (info.fill_ratio < options.dense_fill_ratio) {
    info.detected = HessianStructure::SparsePattern;
  } else {
    info.detected = HessianStructure::Dense;
  }

  return info;
}

inline StructureInfo
InspectHessianStructure(const Eigen::MatrixXd &H,
                        const StructureOptions &options = StructureOptions()) {
  if (H.rows() != H.cols()) {
    throw std::invalid_argument(
        "Hessian structure inspection requires a square matrix");
  }

  std::vector<Eigen::Triplet<double>> triplets;
  triplets.reserve(static_cast<std::size_t>(H.rows() * H.cols()));

  for (int i = 0; i < H.rows(); ++i) {
    for (int j = 0; j < H.cols(); ++j) {
      if (std::abs(H(i, j)) > options.zero_tol) {
        triplets.emplace_back(i, j, H(i, j));
      }
    }
  }

  Eigen::SparseMatrix<double> S(H.rows(), H.cols());
  S.setFromTriplets(triplets.begin(), triplets.end());
  return InspectHessianStructure(S, options);
}

inline HessianStructure ChooseFactorizationBackend(
    const StructureInfo &info,
    const StructureOptions &options = StructureOptions()) {
  if (!info.square) {
    throw std::invalid_argument("Cannot factor non-square Hessian");
  }
  if (!info.numerically_symmetric) {
    throw std::invalid_argument(
        "Cannot use symmetric Hessian backend on non-symmetric matrix");
  }

  if (info.max_bandwidth == 0) {
    return HessianStructure::Diagonal;
  }
  if (info.max_bandwidth == 1) {
    return HessianStructure::Tridiagonal;
  }
  if (info.max_bandwidth <= options.max_banded_width &&
      info.fill_ratio < options.dense_fill_ratio) {
    return HessianStructure::Banded;
  }
  if (info.fill_ratio < options.dense_fill_ratio) {
    return HessianStructure::SparsePattern;
  }
  return HessianStructure::Dense;
}

inline double LogDetDiagonal(const Eigen::SparseMatrix<double> &H) {
  if (H.rows() != H.cols()) {
    throw std::invalid_argument("Diagonal logdet requires square matrix");
  }

  double logdet = 0.0;

  for (int i = 0; i < H.rows(); ++i) {
    const double d = H.coeff(i, i);
    if (!(d > 0.0)) {
      throw std::runtime_error("Diagonal Hessian is not positive definite");
    }
    logdet += std::log(d);
  }

  return logdet;
}

// LDLT logdet for symmetric tridiagonal positive definite matrices.
// This avoids sparse symbolic overhead for max_bandwidth == 1.
inline double LogDetTridiagonalLDLT(const Eigen::SparseMatrix<double> &H) {
  if (H.rows() != H.cols()) {
    throw std::invalid_argument("Tridiagonal logdet requires square matrix");
  }

  const int n = static_cast<int>(H.rows());
  if (n == 0)
    return 0.0;

  double logdet = 0.0;

  double d_prev = H.coeff(0, 0);
  if (!(d_prev > 0.0)) {
    throw std::runtime_error("Tridiagonal Hessian is not positive definite");
  }
  logdet += std::log(d_prev);

  for (int i = 1; i < n; ++i) {
    const double e = H.coeff(i, i - 1);
    const double diag = H.coeff(i, i);

    const double d = diag - (e * e) / d_prev;
    if (!(d > 0.0)) {
      throw std::runtime_error("Tridiagonal Hessian is not positive definite");
    }

    logdet += std::log(d);
    d_prev = d;
  }

  return logdet;
}

// True banded LDLT log determinant for symmetric positive definite matrices.
// Uses compact lower-band storage and costs O(n * bandwidth^2).
inline double LogDetBandedLDLT(const Eigen::SparseMatrix<double> &H,
                               const int bandwidth) {
  if (H.rows() != H.cols()) {
    throw std::invalid_argument("Banded LDLT logdet requires square matrix");
  }
  if (bandwidth < 0) {
    throw std::invalid_argument("Banded LDLT bandwidth must be non-negative");
  }

  const int n = static_cast<int>(H.rows());
  if (n == 0)
    return 0.0;

  const int bw = std::min(bandwidth, n - 1);
  const int stride = bw + 1;

  // Lower-band values: A_band[i * stride + d] = A(i, i - d).
  std::vector<double> A_band(static_cast<std::size_t>(n * stride), 0.0);

  auto at = [&](const int i, const int j) -> double & {
    return A_band[static_cast<std::size_t>(i * stride + (i - j))];
  };

  auto get = [&](const int i, const int j) -> double {
    if (i < j)
      return 0.0;
    const int d = i - j;
    if (d < 0 || d > bw)
      return 0.0;
    return A_band[static_cast<std::size_t>(i * stride + d)];
  };

  Eigen::SparseMatrix<double> canonical = H;
  canonical.makeCompressed();

  for (int outer = 0; outer < canonical.outerSize(); ++outer) {
    for (Eigen::SparseMatrix<double>::InnerIterator it(canonical, outer); it;
         ++it) {
      const int r = static_cast<int>(it.row());
      const int c = static_cast<int>(it.col());
      const double v = it.value();

      const int i = std::max(r, c);
      const int j = std::min(r, c);
      const int d = i - j;

      if (d > bw) {
        if (std::abs(v) > 0.0) {
          throw std::runtime_error("Matrix has nonzero outside declared band");
        }
        continue;
      }

      at(i, j) = v;
    }
  }

  std::vector<double> D(static_cast<std::size_t>(n), 0.0);

  auto L = [&](const int i, const int j) -> double {
    if (i == j)
      return 1.0;
    if (i < j)
      return 0.0;
    const int d = i - j;
    if (d <= 0 || d > bw)
      return 0.0;
    return get(i, j);
  };

  double logdet = 0.0;

  for (int i = 0; i < n; ++i) {
    double diag = get(i, i);

    const int k0_diag = std::max(0, i - bw);
    for (int k = k0_diag; k < i; ++k) {
      const double Lik = L(i, k);
      diag -= Lik * Lik * D[static_cast<std::size_t>(k)];
    }

    if (!(diag > 0.0) || !std::isfinite(diag)) {
      throw std::runtime_error("Banded Hessian is not positive definite");
    }

    D[static_cast<std::size_t>(i)] = diag;
    logdet += std::log(diag);

    const int jmax = std::min(n - 1, i + bw);
    for (int j = i + 1; j <= jmax; ++j) {
      double lij_num = get(j, i);

      const int k0 = std::max(0, std::max(i - bw, j - bw));
      for (int k = k0; k < i; ++k) {
        lij_num -= L(j, k) * D[static_cast<std::size_t>(k)] * L(i, k);
      }

      at(j, i) = lij_num / diag;
    }
  }

  return logdet;
}

// Backwards-compatible name used by early dispatch code.
inline double LogDetBandedDenseLDLT(const Eigen::SparseMatrix<double> &H,
                                    const int bandwidth) {
  return LogDetBandedLDLT(H, bandwidth);
}

inline double LogDetSparseLDLT(const Eigen::SparseMatrix<double> &H) {
  Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> ldlt;
  ldlt.compute(H);

  if (ldlt.info() != Eigen::Success) {
    throw std::runtime_error("Sparse LDLT failed");
  }

  const auto &D = ldlt.vectorD();
  double logdet = 0.0;
  for (int i = 0; i < D.size(); ++i) {
    if (!(D[i] > 0.0)) {
      throw std::runtime_error("Sparse Hessian is not positive definite");
    }
    logdet += std::log(D[i]);
  }

  return logdet;
}

inline double LogDetDenseLDLT(const Eigen::MatrixXd &H) {
  Eigen::LDLT<Eigen::MatrixXd> ldlt(H);

  if (ldlt.info() != Eigen::Success) {
    throw std::runtime_error("Dense LDLT failed");
  }

  const auto &D = ldlt.vectorD();
  double logdet = 0.0;
  for (int i = 0; i < D.size(); ++i) {
    if (!(D[i] > 0.0)) {
      throw std::runtime_error("Dense Hessian is not positive definite");
    }
    logdet += std::log(D[i]);
  }

  return logdet;
}

inline double
AutomaticLogDet(const Eigen::SparseMatrix<double> &H,
                const StructureOptions &options = StructureOptions(),
                HessianStructure *selected_backend = nullptr,
                StructureInfo *out_info = nullptr) {
  const StructureInfo info = InspectHessianStructure(H, options);
  const HessianStructure backend = ChooseFactorizationBackend(info, options);

  if (selected_backend != nullptr) {
    *selected_backend = backend;
  }
  if (out_info != nullptr) {
    *out_info = info;
  }

  switch (backend) {
  case HessianStructure::Diagonal:
    return LogDetDiagonal(H);
  case HessianStructure::Tridiagonal:
    return LogDetTridiagonalLDLT(H);
  case HessianStructure::Banded:
    return LogDetBandedDenseLDLT(H, info.max_bandwidth);
  case HessianStructure::SparsePattern:
    return LogDetSparseLDLT(H);
  case HessianStructure::Dense:
    return LogDetDenseLDLT(Eigen::MatrixXd(H));
  }

  throw std::runtime_error("Unknown Hessian backend");
}

} // namespace laplace
} // namespace quadra
