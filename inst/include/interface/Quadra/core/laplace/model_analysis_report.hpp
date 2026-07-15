#pragma once

#include "hessian_structure.hpp"
#include "laplace_backend_factory.hpp"
#include "structure_detector.hpp"

#include <Eigen/Sparse>

#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>

namespace quadra {
namespace laplace {

enum class SolverRecommendation { Newton, SparseNewton, LBFGS };

inline const char *ToString(const SolverRecommendation solver) {
  switch (solver) {
  case SolverRecommendation::Newton:
    return "Newton";
  case SolverRecommendation::SparseNewton:
    return "SparseNewton";
  case SolverRecommendation::LBFGS:
    return "LBFGS";
  }

  return "Unknown";
}

inline const char *ComplexityForBackend(const LaplaceBackendKind backend) {
  switch (backend) {
  case LaplaceBackendKind::Diagonal:
    return "O(n)";
  case LaplaceBackendKind::Tridiagonal:
    return "O(n)";
  case LaplaceBackendKind::Banded:
    return "O(n*b^2)";
  case LaplaceBackendKind::SparseLDLT:
    return "problem dependent";
  case LaplaceBackendKind::DenseLDLT:
    return "O(n^3)";
  }

  return "unknown";
}

inline SolverRecommendation
RecommendSolver(const BackendRecommendation &recommendation) {
  switch (recommendation.backend) {
  case LaplaceBackendKind::Diagonal:
  case LaplaceBackendKind::Tridiagonal:
  case LaplaceBackendKind::Banded:
    return SolverRecommendation::Newton;

  case LaplaceBackendKind::SparseLDLT:
    return SolverRecommendation::SparseNewton;

  case LaplaceBackendKind::DenseLDLT:
    return SolverRecommendation::LBFGS;
  }

  return SolverRecommendation::LBFGS;
}

struct ModelAnalysisReport {
  int random_effects = 0;
  int rows = 0;
  int cols = 0;
  int nnz = 0;
  int diagonal_nnz = 0;
  int offdiagonal_nnz = 0;
  int max_row_nnz = 0;
  int bandwidth = 0;

  double fill_ratio = 0.0;
  double max_abs_asymmetry = 0.0;

  bool structurally_symmetric = false;
  bool numerically_symmetric = false;

  HessianStructure structure = HessianStructure::Dense;
  LaplaceBackendKind backend = LaplaceBackendKind::DenseLDLT;
  SolverRecommendation solver = SolverRecommendation::LBFGS;

  std::string backend_reason;
  std::string complexity;
  bool supports_symbolic_reuse = false;

  bool is_diagonal() const { return structure == HessianStructure::Diagonal; }

  bool is_tridiagonal() const {
    return structure == HessianStructure::Tridiagonal;
  }

  bool is_banded() const { return structure == HessianStructure::Banded; }

  bool has_sparse_structure() const {
    return is_diagonal() || is_tridiagonal() || is_banded() ||
           structure == HessianStructure::SparsePattern;
  }

  std::string summary() const {
    std::ostringstream os;
    Print(os);
    return os.str();
  }

  void Print(std::ostream &os) const {
    os << "Model Analysis Report\n";
    os << "=====================\n\n";

    os << "Random Effects\n";
    os << "--------------\n";
    os << "Count:                 " << random_effects << "\n\n";

    os << "Hessian Structure\n";
    os << "-----------------\n";
    os << "Rows:                  " << rows << "\n";
    os << "Columns:               " << cols << "\n";
    os << "Nonzeros:              " << nnz << "\n";
    os << "Diagonal nonzeros:     " << diagonal_nnz << "\n";
    os << "Off-diagonal nonzeros: " << offdiagonal_nnz << "\n";
    os << "Max row nonzeros:      " << max_row_nnz << "\n";
    os << "Bandwidth:             " << bandwidth << "\n";
    os << "Fill ratio:            " << std::fixed << std::setprecision(6)
       << fill_ratio << "\n";
    os << "Structurally symmetric:" << (structurally_symmetric ? " yes" : " no")
       << "\n";
    os << "Numerically symmetric: " << (numerically_symmetric ? " yes" : " no")
       << "\n";
    os << "Detected structure:    " << ToString(structure) << "\n\n";

    os << "Recommendations\n";
    os << "---------------\n";
    os << "Random solver:          " << ToString(solver) << "\n";
    os << "Laplace backend:        " << ToString(backend) << "\n";
    os << "Backend reason:         " << backend_reason << "\n";
    os << "Expected complexity:    " << complexity << "\n";
    os << "Symbolic reuse:         "
       << (supports_symbolic_reuse ? "supported" : "not needed / unavailable")
       << "\n\n";

    os << "Interpretation\n";
    os << "--------------\n";
    if (is_diagonal()) {
      os << "Random effects are conditionally independent under this "
            "Hessian.\n";
      os << "A diagonal Newton/update path should be very cheap.\n";
    } else if (is_tridiagonal()) {
      os << "Random effects have first-order Markov structure.\n";
      os << "A tridiagonal Newton solver and tridiagonal logdet are "
            "recommended.\n";
    } else if (is_banded()) {
      os << "Random effects have fixed-width local dependence.\n";
      os << "A banded Newton/backend path is recommended.\n";
    } else if (structure == HessianStructure::SparsePattern) {
      os << "Random effects are sparse but not a simple narrow band.\n";
      os << "Sparse Newton/SparseLDLT paths are recommended.\n";
    } else {
      os << "No exploitable sparse structure was detected.\n";
      os << "Dense or quasi-Newton methods may be required.\n";
    }
  }
};

inline ModelAnalysisReport analyze_hessian_structure(
    const Eigen::SparseMatrix<double> &H,
    const StructureDetectorOptions &options = StructureDetectorOptions()) {
  StructureDetector detector(options);
  const BackendRecommendation recommendation = detector.Analyze(H);
  const StructureInfo info =
      InspectHessianStructure(H, options.structure_options);

  ModelAnalysisReport report;
  report.random_effects = static_cast<int>(H.rows());
  report.rows = info.rows;
  report.cols = info.cols;
  report.nnz = info.nnz;
  report.diagonal_nnz = info.diagonal_nnz;
  report.offdiagonal_nnz = info.offdiagonal_nnz;
  report.max_row_nnz = info.max_row_nnz;
  report.bandwidth = info.max_bandwidth;
  report.fill_ratio = info.fill_ratio;
  report.max_abs_asymmetry = info.max_abs_asymmetry;
  report.structurally_symmetric = info.structurally_symmetric;
  report.numerically_symmetric = info.numerically_symmetric;
  report.structure = recommendation.structure;

  report.backend = recommendation.backend;
  report.solver = RecommendSolver(recommendation);
  report.backend_reason = recommendation.reason;
  report.complexity = ComplexityForBackend(recommendation.backend);
  report.supports_symbolic_reuse = recommendation.supports_symbolic_reuse;

  return report;
}

inline void PrintModelAnalysisReport(std::ostream &os,
                                     const ModelAnalysisReport &report) {
  report.Print(os);
}

} // namespace laplace
} // namespace quadra
