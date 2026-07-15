#pragma once

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace quadra {
namespace laplace {

struct DiagonalValues {
  Eigen::VectorXd diag;
};

struct TridiagonalValues {
  Eigen::VectorXd diag;
  Eigen::VectorXd offdiag; // offdiag[i - 1] = H(i, i - 1)
};

struct SparseMatrixValues {
  Eigen::SparseMatrix<double> H;
};

struct BandedValues {
  int bandwidth = 0;
  Eigen::VectorXd diag;

  // lower_bands[d - 1][j] = H(j + d, j), d = 1..bandwidth.
  // The d-th lower band has length max(0, n - d).
  std::vector<Eigen::VectorXd> lower_bands;
};

inline double logdet_diagonal_values(const DiagonalValues &H) {
  double logdet = 0.0;
  for (int i = 0; i < H.diag.size(); ++i) {
    const double d = H.diag[i];
    if (!(d > 0.0) || !std::isfinite(d)) {
      throw std::runtime_error(
          "Diagonal value Hessian is not positive definite");
    }
    logdet += std::log(d);
  }
  return logdet;
}

inline double logdet_tridiagonal_values_ldlt(const TridiagonalValues &H) {
  const int n = static_cast<int>(H.diag.size());
  if (n == 0)
    return 0.0;

  if (H.offdiag.size() != std::max(0, n - 1)) {
    throw std::invalid_argument(
        "TridiagonalValues has inconsistent offdiag size");
  }

  double d_prev = H.diag[0];
  if (!(d_prev > 0.0) || !std::isfinite(d_prev)) {
    throw std::runtime_error(
        "Tridiagonal value Hessian is not positive definite");
  }

  double logdet = std::log(d_prev);

  for (int i = 1; i < n; ++i) {
    const double e = H.offdiag[i - 1];
    const double d = H.diag[i] - (e * e) / d_prev;

    if (!(d > 0.0) || !std::isfinite(d)) {
      throw std::runtime_error(
          "Tridiagonal value Hessian is not positive definite");
    }

    logdet += std::log(d);
    d_prev = d;
  }

  return logdet;
}

inline double lower_band_value(const BandedValues &H, const int i,
                               const int j) {
  if (i < j)
    return 0.0;

  const int d = i - j;
  if (d == 0)
    return H.diag[i];
  if (d < 0 || d > H.bandwidth)
    return 0.0;

  const Eigen::VectorXd &band = H.lower_bands[static_cast<std::size_t>(d - 1)];

  if (j < 0 || j >= band.size())
    return 0.0;
  return band[j];
}

inline double logdet_sparse_matrix_values_ldlt(const SparseMatrixValues &H) {
  Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver;
  solver.compute(H.H);

  if (solver.info() != Eigen::Success) {
    throw std::runtime_error("SparseMatrixValues LDLT factorization failed");
  }

  const auto d = solver.vectorD();
  double logdet = 0.0;

  for (Eigen::Index i = 0; i < d.size(); ++i) {
    const double di = std::abs(d[i]);
    if (!(di > 0.0) || !std::isfinite(di)) {
      throw std::runtime_error(
          "SparseMatrixValues Hessian has invalid LDLT diagonal");
    }
    logdet += std::log(di);
  }

  return logdet;
}

inline double logdet_banded_values_ldlt(const BandedValues &H) {
  const int n = static_cast<int>(H.diag.size());
  if (n == 0)
    return 0.0;

  if (H.bandwidth < 0) {
    throw std::invalid_argument("BandedValues bandwidth must be non-negative");
  }

  if (static_cast<int>(H.lower_bands.size()) != H.bandwidth) {
    throw std::invalid_argument(
        "BandedValues lower_bands size must equal bandwidth");
  }

  for (int d = 1; d <= H.bandwidth; ++d) {
    const int expected = std::max(0, n - d);
    const int actual =
        static_cast<int>(H.lower_bands[static_cast<std::size_t>(d - 1)].size());
    if (actual != expected) {
      throw std::invalid_argument("BandedValues lower band has wrong length");
    }
  }

  const int bw = std::min(H.bandwidth, std::max(0, n - 1));
  const int stride = bw + 1;
  std::vector<double> work(static_cast<std::size_t>(n * stride), 0.0);

  auto at = [&](const int i, const int j) -> double & {
    return work[static_cast<std::size_t>(i * stride + (i - j))];
  };

  auto get = [&](const int i, const int j) -> double {
    if (i < j)
      return 0.0;
    const int d = i - j;
    if (d < 0 || d > bw)
      return 0.0;
    return work[static_cast<std::size_t>(i * stride + d)];
  };

  for (int i = 0; i < n; ++i) {
    at(i, i) = H.diag[i];
    for (int d = 1; d <= bw; ++d) {
      const int j = i - d;
      if (j >= 0) {
        at(i, j) = H.lower_bands[static_cast<std::size_t>(d - 1)][j];
      }
    }
  }

  double logdet = 0.0;
  for (int k = 0; k < n; ++k) {
    double dkk = get(k, k);
    const int j_begin = std::max(0, k - bw);
    for (int j = j_begin; j < k; ++j) {
      const double lkj = get(k, j);
      const double djj = get(j, j);
      dkk -= lkj * lkj * djj;
    }

    if (!(dkk > 0.0) || !std::isfinite(dkk)) {
      throw std::runtime_error("Banded value Hessian is not positive definite");
    }
    at(k, k) = dkk;
    logdet += std::log(dkk);

    const int i_end = std::min(n - 1, k + bw);
    for (int i = k + 1; i <= i_end; ++i) {
      double lik_num = get(i, k);
      const int j_begin2 = std::max(0, std::max(i - bw, k - bw));
      for (int j = j_begin2; j < k; ++j) {
        if (i - j <= bw && k - j <= bw) {
          lik_num -= get(i, j) * get(k, j) * get(j, j);
        }
      }
      at(i, k) = lik_num / dkk;
    }
  }

  return logdet;
}

inline Eigen::SparseMatrix<double>
sparse_from_diagonal_values(const DiagonalValues &H) {
  const int n = static_cast<int>(H.diag.size());
  std::vector<Eigen::Triplet<double>> triplets;
  triplets.reserve(static_cast<std::size_t>(n));

  for (int i = 0; i < n; ++i) {
    if (std::abs(H.diag[i]) > 0.0) {
      triplets.emplace_back(i, i, H.diag[i]);
    }
  }

  Eigen::SparseMatrix<double> S(n, n);
  S.setFromTriplets(triplets.begin(), triplets.end());
  S.makeCompressed();
  return S;
}

inline Eigen::SparseMatrix<double>
sparse_from_banded_values(const BandedValues &H) {
  const int n = static_cast<int>(H.diag.size());
  if (static_cast<int>(H.lower_bands.size()) != H.bandwidth) {
    throw std::invalid_argument(
        "BandedValues lower_bands size must equal bandwidth");
  }

  std::vector<Eigen::Triplet<double>> triplets;
  triplets.reserve(static_cast<std::size_t>(n * (2 * H.bandwidth + 1)));
  for (int i = 0; i < n; ++i) {
    if (std::abs(H.diag[i]) > 0.0) {
      triplets.emplace_back(i, i, H.diag[i]);
    }
    for (int d = 1; d <= H.bandwidth; ++d) {
      const int j = i - d;
      if (j < 0)
        continue;
      const Eigen::VectorXd &band =
          H.lower_bands[static_cast<std::size_t>(d - 1)];
      if (j >= band.size())
        continue;
      const double v = band[j];
      if (std::abs(v) > 0.0) {
        triplets.emplace_back(i, j, v);
        triplets.emplace_back(j, i, v);
      }
    }
  }

  Eigen::SparseMatrix<double> S(n, n);
  S.setFromTriplets(triplets.begin(), triplets.end());
  S.makeCompressed();
  return S;
}

inline Eigen::SparseMatrix<double>
sparse_from_tridiagonal_values(const TridiagonalValues &H) {
  const int n = static_cast<int>(H.diag.size());
  std::vector<Eigen::Triplet<double>> triplets;
  triplets.reserve(static_cast<std::size_t>(3 * n));

  for (int i = 0; i < n; ++i) {
    if (std::abs(H.diag[i]) > 0.0) {
      triplets.emplace_back(i, i, H.diag[i]);
    }

    if (i > 0 && std::abs(H.offdiag[i - 1]) > 0.0) {
      triplets.emplace_back(i, i - 1, H.offdiag[i - 1]);
      triplets.emplace_back(i - 1, i, H.offdiag[i - 1]);
    }
  }

  Eigen::SparseMatrix<double> S(n, n);
  S.setFromTriplets(triplets.begin(), triplets.end());
  S.makeCompressed();
  return S;
}

} // namespace laplace
} // namespace quadra
