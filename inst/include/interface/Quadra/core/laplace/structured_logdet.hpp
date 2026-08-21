#pragma once

#include "structure_detector.hpp"
#include "structured_value_factory.hpp"

#include <Eigen/Sparse>

#include <stdexcept>

namespace quadra {
namespace laplace {

struct StructuredLogDetResult {
  double logdet = 0.0;
  HessianStructure structure = HessianStructure::Dense;
  LaplaceBackendKind backend = LaplaceBackendKind::DenseLDLT;
  int bandwidth = 0;
  int rows = 0;
  int nnz = 0;
};

inline StructuredLogDetResult
ComputeStructuredLogDet(const Eigen::SparseMatrix<double> &H,
                        const BackendRecommendation &rec) {
  StructuredLogDetResult out;
  out.structure = rec.structure;
  out.backend = rec.backend;
  out.bandwidth = rec.bandwidth;
  out.rows = static_cast<int>(H.rows());
  out.nnz = static_cast<int>(H.nonZeros());

  const StructuredValues values = extract_structured_values(H, rec);
  out.logdet = logdet_structured_values(values);

  return out;
}

inline StructuredLogDetResult
ComputeStructuredLogDet(const Eigen::SparseMatrix<double> &H,
                        const LaplaceBackendKind backend,
                        const int bandwidth = 0) {
  BackendRecommendation rec;
  rec.backend = backend;
  rec.bandwidth = bandwidth;

  switch (backend) {
  case LaplaceBackendKind::Diagonal:
    rec.structure = HessianStructure::Diagonal;
    break;
  case LaplaceBackendKind::Tridiagonal:
    rec.structure = HessianStructure::Tridiagonal;
    rec.bandwidth = 1;
    break;
  case LaplaceBackendKind::Banded:
    rec.structure = HessianStructure::Banded;
    break;
  case LaplaceBackendKind::SparseLDLT:
    rec.structure = HessianStructure::SparsePattern;
    break;
  case LaplaceBackendKind::DenseLDLT:
    rec.structure = HessianStructure::Dense;
    break;
  }

  return ComputeStructuredLogDet(H, rec);
}

} // namespace laplace
} // namespace quadra
