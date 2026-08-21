#pragma once

#include "sparse_factorization_cache.hpp"
#include "structure_detector.hpp"
#include "structured_logdet.hpp"
#include "structured_value_factory.hpp"

#include <Eigen/Sparse>

#include <cmath>
#include <stdexcept>

namespace quadra {
namespace laplace {

struct PersistentStructuredRuntimeState {
  bool initialized = false;

  BackendRecommendation recommendation;
  StructuredValues values;
  StructuredLogDetResult last_logdet;
  BandedLDLTWorkspace banded_workspace;
  ::quadra::SparseLDLTFactorizationCache sparse_ldlt_workspace;

  double compute_logdet() {
    if (std::holds_alternative<BandedValues>(values)) {
      return logdet_banded_values_ldlt(std::get<BandedValues>(values),
                                       banded_workspace);
    }
    if (std::holds_alternative<SparseMatrixValues>(values) &&
        recommendation.backend == LaplaceBackendKind::SparseLDLT) {
      sparse_ldlt_workspace.factorize(std::get<SparseMatrixValues>(values).H);
      return sparse_ldlt_workspace.logdet();
    }
    return logdet_structured_values(values);
  }

  void clear() {
    initialized = false;
    recommendation = BackendRecommendation();
    values = DiagonalValues();
    last_logdet = StructuredLogDetResult();
    banded_workspace.clear();
    sparse_ldlt_workspace.reset();
  }

  void update_from_hessian(const Eigen::SparseMatrix<double> &H,
                           const BackendRecommendation &rec) {
    recommendation = rec;
    if (recommendation.backend == LaplaceBackendKind::SparseLDLT ||
        recommendation.backend == LaplaceBackendKind::DenseLDLT) {
      if (!std::holds_alternative<SparseMatrixValues>(values))
        values = SparseMatrixValues();
      std::get<SparseMatrixValues>(values).H = H;
    } else {
      update_structured_values_from_hessian(values, H, recommendation);
    }

    last_logdet.structure = recommendation.structure;
    last_logdet.backend = recommendation.backend;
    last_logdet.bandwidth = recommendation.bandwidth;
    last_logdet.rows = static_cast<int>(H.rows());
    last_logdet.nnz = static_cast<int>(H.nonZeros());
    last_logdet.logdet = compute_logdet();

    initialized = true;
  }

  void update_from_hessian(
      const Eigen::SparseMatrix<double> &H,
      const StructureDetectorOptions &options = StructureDetectorOptions()) {
    StructureDetector detector(options);
    update_from_hessian(H, detector.Analyze(H));
  }

  void update_values_only(const Eigen::SparseMatrix<double> &H) {
    if (!initialized) {
      throw std::runtime_error("PersistentStructuredRuntimeState::update_"
                               "values_only used before initialization");
    }

    if (recommendation.backend == LaplaceBackendKind::SparseLDLT ||
        recommendation.backend == LaplaceBackendKind::DenseLDLT) {
      if (!std::holds_alternative<SparseMatrixValues>(values))
        values = SparseMatrixValues();
      std::get<SparseMatrixValues>(values).H = H;
    } else {
      update_structured_values_from_hessian(values, H, recommendation);
    }

    last_logdet.structure = recommendation.structure;
    last_logdet.backend = recommendation.backend;
    last_logdet.bandwidth = recommendation.bandwidth;
    last_logdet.rows = static_cast<int>(H.rows());
    last_logdet.nnz = static_cast<int>(H.nonZeros());
    last_logdet.logdet = compute_logdet();
  }

  void update_direct(const DiagonalValues &new_values) {
    BackendRecommendation rec;
    rec.backend = LaplaceBackendKind::Diagonal;
    rec.structure = HessianStructure::Diagonal;
    rec.bandwidth = 0;
    rec.random_size = static_cast<int>(new_values.diag.size());
    rec.nnz = static_cast<int>(new_values.diag.size());
    rec.max_row_nnz = 1;
    rec.fill_ratio =
        rec.random_size > 0 ? 1.0 / static_cast<double>(rec.random_size) : 0.0;
    rec.symmetric = true;
    rec.reason = "direct diagonal values";

    update_direct(new_values, rec);
  }

  void update_direct(const TridiagonalValues &new_values) {
    const int n = static_cast<int>(new_values.diag.size());

    BackendRecommendation rec;
    rec.backend = LaplaceBackendKind::Tridiagonal;
    rec.structure = HessianStructure::Tridiagonal;
    rec.bandwidth = 1;
    rec.random_size = n;
    rec.nnz = n + 2 * static_cast<int>(new_values.offdiag.size());
    rec.max_row_nnz = n <= 1 ? 1 : 3;
    rec.fill_ratio =
        n > 0 ? static_cast<double>(rec.nnz) / static_cast<double>(n * n) : 0.0;
    rec.symmetric = true;
    rec.reason = "direct tridiagonal values";

    update_direct(new_values, rec);
  }

  void update_direct(const BandedValues &new_values) {
    const int n = static_cast<int>(new_values.diag.size());

    int offdiag_nnz = 0;
    for (const auto &band : new_values.lower_bands) {
      offdiag_nnz += static_cast<int>(band.size());
    }

    BackendRecommendation rec;
    rec.backend = LaplaceBackendKind::Banded;
    rec.structure = HessianStructure::Banded;
    rec.bandwidth = new_values.bandwidth;
    rec.random_size = n;
    rec.nnz = n + 2 * offdiag_nnz;
    rec.max_row_nnz = std::min(n, 2 * new_values.bandwidth + 1);
    rec.fill_ratio =
        n > 0 ? static_cast<double>(rec.nnz) / static_cast<double>(n * n) : 0.0;
    rec.symmetric = true;
    rec.reason = "direct banded values";

    update_direct(new_values, rec);
  }

  void update_direct(const StructuredValues &new_values) {
    std::visit([this](const auto &v) { this->update_direct(v); }, new_values);
  }

  void update_direct(const DiagonalValues &new_values,
                     const BackendRecommendation &rec) {
    recommendation = rec;
    values = new_values;

    last_logdet.structure = recommendation.structure;
    last_logdet.backend = recommendation.backend;
    last_logdet.bandwidth = recommendation.bandwidth;
    last_logdet.rows = static_cast<int>(new_values.diag.size());
    last_logdet.nnz = static_cast<int>(new_values.diag.size());
    last_logdet.logdet = compute_logdet();

    initialized = true;
  }

  void update_direct(const TridiagonalValues &new_values,
                     const BackendRecommendation &rec) {
    recommendation = rec;
    values = new_values;

    last_logdet.structure = recommendation.structure;
    last_logdet.backend = recommendation.backend;
    last_logdet.bandwidth = recommendation.bandwidth;
    last_logdet.rows = static_cast<int>(new_values.diag.size());
    last_logdet.nnz = static_cast<int>(new_values.diag.size()) +
                      2 * static_cast<int>(new_values.offdiag.size());
    last_logdet.logdet = compute_logdet();

    initialized = true;
  }

  void update_direct(const BandedValues &new_values,
                     const BackendRecommendation &rec) {
    recommendation = rec;
    values = new_values;

    int offdiag_nnz = 0;
    for (const auto &band : new_values.lower_bands) {
      offdiag_nnz += static_cast<int>(band.size());
    }

    last_logdet.structure = recommendation.structure;
    last_logdet.backend = recommendation.backend;
    last_logdet.bandwidth = recommendation.bandwidth;
    last_logdet.rows = static_cast<int>(new_values.diag.size());
    last_logdet.nnz =
        static_cast<int>(new_values.diag.size()) + 2 * offdiag_nnz;
    last_logdet.logdet = compute_logdet();

    initialized = true;
  }

  double logdet() const {
    if (!initialized) {
      throw std::runtime_error(
          "PersistentStructuredRuntimeState used before initialization");
    }

    return last_logdet.logdet;
  }

  const BackendRecommendation &backend_recommendation() const {
    if (!initialized) {
      throw std::runtime_error(
          "PersistentStructuredRuntimeState recommendation requested before "
          "initialization");
    }

    return recommendation;
  }

  const StructuredValues &structured_values() const {
    if (!initialized) {
      throw std::runtime_error("PersistentStructuredRuntimeState values "
                               "requested before initialization");
    }

    return values;
  }
};

struct PersistentStructuredLaplaceResult {
  double logdet = 0.0;
  BackendRecommendation recommendation;
  StructuredLogDetResult structured_logdet;
  bool initialized_before_call = false;
  bool detected_structure = false;
};

class PersistentStructuredLaplaceRuntime {
public:
  explicit PersistentStructuredLaplaceRuntime(
      StructureDetectorOptions options = StructureDetectorOptions())
      : options_(options) {}

  PersistentStructuredLaplaceResult
  evaluate(const Eigen::SparseMatrix<double> &H) {
    PersistentStructuredLaplaceResult out;
    out.initialized_before_call = state_.initialized;

    if (!state_.initialized || !current_backend_supports(H)) {
      state_.update_from_hessian(H, options_);
      out.detected_structure = true;
    } else {
      state_.update_values_only(H);
      out.detected_structure = false;
    }

    out.logdet = state_.logdet();
    out.recommendation = state_.backend_recommendation();
    out.structured_logdet = state_.last_logdet;

    return out;
  }

  void reset() { state_.clear(); }

  bool initialized() const { return state_.initialized; }

  const PersistentStructuredRuntimeState &state() const { return state_; }

  PersistentStructuredRuntimeState &mutable_state() { return state_; }

  const StructureDetectorOptions &options() const { return options_; }

private:
  bool current_backend_supports(const Eigen::SparseMatrix<double> &H) const {
    const BackendRecommendation &rec = state_.recommendation;
    if (H.rows() != rec.random_size || H.cols() != rec.random_size) {
      return false;
    }

    int supported_bandwidth = -1;
    switch (rec.backend) {
    case LaplaceBackendKind::Diagonal:
      supported_bandwidth = 0;
      break;
    case LaplaceBackendKind::Tridiagonal:
      supported_bandwidth = 1;
      break;
    case LaplaceBackendKind::Banded:
      supported_bandwidth = rec.bandwidth;
      break;
    case LaplaceBackendKind::SparseLDLT:
    case LaplaceBackendKind::DenseLDLT:
      return true;
    }

    const double zero_tol = options_.structure_options.zero_tol;
    for (int outer = 0; outer < H.outerSize(); ++outer) {
      for (Eigen::SparseMatrix<double>::InnerIterator entry(H, outer); entry;
           ++entry) {
        if (std::abs(entry.value()) > zero_tol &&
            std::abs(entry.row() - entry.col()) > supported_bandwidth) {
          return false;
        }
      }
    }
    return true;
  }

  StructureDetectorOptions options_;
  PersistentStructuredRuntimeState state_;
};

} // namespace laplace
} // namespace quadra
