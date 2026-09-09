#pragma once

#include <Eigen/Sparse>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace quadra {
namespace laplace {

struct HdotActiveDirectionDiscoveryResult {
  std::vector<int> active_directions;
  std::vector<double> hdot_norms;
  double tolerance = 0.0;
};

inline double sparse_frobenius_norm(const Eigen::SparseMatrix<double> &A) {
  double sumsq = 0.0;
  for (int outer = 0; outer < A.outerSize(); ++outer) {
    for (Eigen::SparseMatrix<double>::InnerIterator it(A, outer); it; ++it) {
      sumsq += it.value() * it.value();
    }
  }
  return std::sqrt(sumsq);
}

// Discover active fixed-effect directions from already computed total Hdot
// matrices. This is the safest criterion because it includes both direct
// theta effects and indirect mode-sensitivity effects through u*(theta).
inline HdotActiveDirectionDiscoveryResult discover_active_directions_from_hdots(
    const std::vector<Eigen::SparseMatrix<double>> &Hdots, double tolerance) {
  if (tolerance < 0.0 || !std::isfinite(tolerance)) {
    throw std::invalid_argument(
        "Hdot discovery tolerance must be nonnegative and finite.");
  }

  HdotActiveDirectionDiscoveryResult out;
  out.tolerance = tolerance;
  out.hdot_norms.resize(Hdots.size(), 0.0);

  for (int j = 0; j < static_cast<int>(Hdots.size()); ++j) {
    const double norm = sparse_frobenius_norm(Hdots[static_cast<size_t>(j)]);
    out.hdot_norms[static_cast<size_t>(j)] = norm;

    if (norm > tolerance) {
      out.active_directions.push_back(j);
    }
  }

  return out;
}

// Generic discovery helper. Provider must support:
//   compute_all_sparse(theta, uhat)
//
// Typically provider is built with all candidate directions active.
template <class HdotProvider, class Theta, class Uhat>
HdotActiveDirectionDiscoveryResult
discover_active_directions_by_hdot_norm(const HdotProvider &provider,
                                        const Theta &theta, const Uhat &uhat,
                                        double tolerance) {
  const auto Hdots = provider.compute_all_sparse(theta, uhat);
  return discover_active_directions_from_hdots(Hdots, tolerance);
}

} // namespace laplace
} // namespace quadra
