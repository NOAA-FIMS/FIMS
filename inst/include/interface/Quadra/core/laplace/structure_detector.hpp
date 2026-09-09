#pragma once

#include "hessian_structure.hpp"

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace quadra {
namespace laplace {

enum class LaplaceBackendKind {
  Diagonal,
  Tridiagonal,
  Banded,
  SparseLDLT,
  DenseLDLT
};

inline const char *ToString(const LaplaceBackendKind backend) {
  switch (backend) {
  case LaplaceBackendKind::Diagonal:
    return "diagonal";
  case LaplaceBackendKind::Tridiagonal:
    return "tridiagonal";
  case LaplaceBackendKind::Banded:
    return "banded";
  case LaplaceBackendKind::SparseLDLT:
    return "sparse_ldlt";
  case LaplaceBackendKind::DenseLDLT:
    return "dense_ldlt";
  }
  return "unknown";
}

struct BackendRecommendation {
  LaplaceBackendKind backend = LaplaceBackendKind::DenseLDLT;
  HessianStructure structure = HessianStructure::Dense;

  int random_size = 0;
  int nnz = 0;
  int bandwidth = 0;
  int max_row_nnz = 0;

  double fill_ratio = 0.0;
  bool symmetric = false;
  bool pattern_reusable = true;
  bool supports_symbolic_reuse = false;
  bool supports_warm_start = true;

  std::string reason;
};

struct StructureDetectorOptions {
  StructureOptions structure_options;

  // Recommended cutoff for using specialized banded code.
  int banded_width_cutoff = 64;

  // Very small random-effect dimensions are usually cheaper as dense even when
  // technically sparse/banded.
  int dense_size_cutoff = 16;

  // Above this fill ratio, dense is usually preferred.
  double dense_fill_ratio = 0.25;

  bool prefer_dense_for_small_matrices = true;
};

class StructureDetector {
public:
  explicit StructureDetector(
      StructureDetectorOptions options = StructureDetectorOptions())
      : options_(options) {
    options_.structure_options.max_banded_width = options_.banded_width_cutoff;
    options_.structure_options.dense_fill_ratio = options_.dense_fill_ratio;
  }

  BackendRecommendation Analyze(const Eigen::SparseMatrix<double> &H) const {
    const StructureInfo info =
        InspectHessianStructure(H, options_.structure_options);

    return Recommend(info);
  }

  BackendRecommendation Analyze(const Eigen::MatrixXd &H) const {
    const StructureInfo info =
        InspectHessianStructure(H, options_.structure_options);

    return Recommend(info);
  }

  BackendRecommendation Recommend(const StructureInfo &info) const {
    BackendRecommendation rec;
    rec.random_size = info.rows;
    rec.nnz = info.nnz;
    rec.bandwidth = info.max_bandwidth;
    rec.max_row_nnz = info.max_row_nnz;
    rec.fill_ratio = info.fill_ratio;
    rec.symmetric = info.numerically_symmetric;
    rec.structure = info.detected;

    if (!info.square) {
      rec.backend = LaplaceBackendKind::DenseLDLT;
      rec.pattern_reusable = false;
      rec.supports_symbolic_reuse = false;
      rec.reason = "non-square matrix cannot use Hessian backend";
      return rec;
    }

    if (!info.numerically_symmetric) {
      rec.backend = LaplaceBackendKind::DenseLDLT;
      rec.pattern_reusable = false;
      rec.supports_symbolic_reuse = false;
      rec.reason = "non-symmetric Hessian; falling back to dense";
      return rec;
    }

    if (info.max_bandwidth == 0) {
      rec.backend = LaplaceBackendKind::Diagonal;
      rec.structure = HessianStructure::Diagonal;
      rec.supports_symbolic_reuse = false;
      rec.reason = "zero off-diagonal bandwidth";
      return rec;
    }

    if (info.max_bandwidth == 1) {
      rec.backend = LaplaceBackendKind::Tridiagonal;
      rec.structure = HessianStructure::Tridiagonal;
      rec.supports_symbolic_reuse = false;
      rec.reason = "unit bandwidth";
      return rec;
    }

    if (info.max_bandwidth <= options_.banded_width_cutoff &&
        info.fill_ratio < options_.dense_fill_ratio) {
      rec.backend = LaplaceBackendKind::Banded;
      rec.structure = HessianStructure::Banded;
      rec.supports_symbolic_reuse = true;
      rec.reason = "fixed narrow band detected";
      return rec;
    }

    if (options_.prefer_dense_for_small_matrices &&
        info.rows <= options_.dense_size_cutoff) {
      rec.backend = LaplaceBackendKind::DenseLDLT;
      rec.supports_symbolic_reuse = false;
      rec.reason = "small matrix; dense LDLT preferred";
      return rec;
    }

    if (info.fill_ratio < options_.dense_fill_ratio) {
      rec.backend = LaplaceBackendKind::SparseLDLT;
      rec.structure = HessianStructure::SparsePattern;
      rec.supports_symbolic_reuse = true;
      rec.reason = "general sparse pattern detected";
      return rec;
    }

    rec.backend = LaplaceBackendKind::DenseLDLT;
    rec.structure = HessianStructure::Dense;
    rec.supports_symbolic_reuse = false;
    rec.reason = "high fill ratio; dense LDLT preferred";
    return rec;
  }

  const StructureDetectorOptions &options() const { return options_; }

private:
  StructureDetectorOptions options_;
};

inline double LogDetWithRecommendation(const Eigen::SparseMatrix<double> &H,
                                       const BackendRecommendation &rec) {
  switch (rec.backend) {
  case LaplaceBackendKind::Diagonal:
    return LogDetDiagonal(H);
  case LaplaceBackendKind::Tridiagonal:
    return LogDetTridiagonalLDLT(H);
  case LaplaceBackendKind::Banded:
    return LogDetBandedLDLT(H, rec.bandwidth);
  case LaplaceBackendKind::SparseLDLT:
    return LogDetSparseLDLT(H);
  case LaplaceBackendKind::DenseLDLT:
    return LogDetDenseLDLT(Eigen::MatrixXd(H));
  }

  throw std::runtime_error("Unknown backend recommendation");
}

} // namespace laplace
} // namespace quadra
