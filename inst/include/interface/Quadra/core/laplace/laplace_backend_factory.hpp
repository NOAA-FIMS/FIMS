#pragma once

#include "laplace_backend.hpp"
#include "structure_detector.hpp"

#include <memory>
#include <stdexcept>

namespace quadra {
namespace laplace {

inline std::unique_ptr<LaplaceBackend>
CreateLaplaceBackend(const BackendRecommendation &rec) {
  switch (rec.backend) {
  case LaplaceBackendKind::Diagonal:
    return std::make_unique<DiagonalBackend>();

  case LaplaceBackendKind::Tridiagonal:
    return std::make_unique<TridiagonalBackend>();

  case LaplaceBackendKind::Banded:
    return std::make_unique<BandedBackend>(rec.bandwidth);

  case LaplaceBackendKind::SparseLDLT:
    return std::make_unique<SparseLDLTBackend>();

  case LaplaceBackendKind::DenseLDLT:
    return std::make_unique<DenseLDLTBackend>();
  }

  throw std::runtime_error("Unknown Laplace backend recommendation");
}

inline std::unique_ptr<LaplaceBackend> CreateLaplaceBackendForHessian(
    const Eigen::SparseMatrix<double> &H,
    BackendRecommendation *out_recommendation = nullptr,
    const StructureDetectorOptions &options = StructureDetectorOptions()) {
  StructureDetector detector(options);
  BackendRecommendation rec = detector.Analyze(H);

  if (out_recommendation != nullptr) {
    *out_recommendation = rec;
  }

  auto backend = CreateLaplaceBackend(rec);
  backend->analyze_pattern(H);
  return backend;
}

} // namespace laplace
} // namespace quadra
