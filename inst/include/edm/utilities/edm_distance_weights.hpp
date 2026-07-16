/**
 * @file edm_distance_weights.hpp
 * @brief Modular distance and weighting utilities for EDM prediction algorithms.
 *
 * @details This header provides free functions in the fims_edm namespace that
 * are shared across all EDM prediction algorithms:
 *
 *  - SquaredEuclideanDistance(): pairwise squared distance between two
 *    embedding rows (raw-pointer interface for CppAD compatibility).
 *  - SimplexWeights(): exponential weights normalized by d_min (used by
 *    Simplex Projection, Sugihara & May 1990).
 *  - SMapWeights(): exponential or Gaussian weights scaled by theta.
 *    Kernel choice is controlled by the SMapKernel enum:
 *      - SMapKernel::kExponential (default): w = exp(-θ * d / d̄)
 *        Classic formulation from Sugihara (1994) and rEDM.
 *      - SMapKernel::kGaussian: w = exp(-θ² * (d/D)²)
 *        Gaussian kernel from Esguerra & Munch (2024); avoids sqrt inside
 *        the AD tape.
 *  - NormalizeWeights(): in-place normalization so weights sum to 1.
 *
 * ### Design rationale
 * All weight vectors are passed by reference so callers can pre-allocate
 * storage and avoid repeated heap allocation inside hot prediction loops.
 * Raw-pointer distance computation keeps the inner loops visible to TMB's
 * CppAD tracer, which cannot trace STL algorithms (e.g., std::transform,
 * std::inner_product).
 *
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_EDM_DISTANCE_WEIGHTS_HPP
#define FIMS_EDM_DISTANCE_WEIGHTS_HPP

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

#include "../../common/fims_math.hpp"

namespace fims_edm {

// ---------------------------------------------------------------------------
// Kernel selector
// ---------------------------------------------------------------------------

/**
 * @brief Selects the weighting kernel used by SMapWeights().
 *
 * @details Both kernels are controlled by the nonlinearity parameter theta
 * (θ ≥ 0).  When θ = 0 every row has weight 1 regardless of the kernel
 * chosen, so the distinction only matters for θ > 0.
 *
 * | Value | Formula | Source |
 * |---|---|---|
 * | kExponential | w = exp(−θ · d / d̄)  | Sugihara (1994), rEDM |
 * | kGaussian    | w = exp(−θ² · (d/D)²) | Esguerra & Munch (2024) |
 *
 * The Gaussian kernel has slightly fatter tails and avoids a sqrt() call
 * inside the AD tape (distances are kept squared throughout).
 * Per Steve Munch (pers. comm.): "the difference in performance is rarely
 * large; use whatever is easier/faster."
 */
enum class SMapKernel {
  kExponential,  ///< Classic exponential kernel: exp(-θ * d / d̄)
  kGaussian      ///< Gaussian kernel:             exp(-θ² * (d/D)²)
};

// ---------------------------------------------------------------------------
// Distance
// ---------------------------------------------------------------------------

/**
 * @brief Compute the squared Euclidean distance between two embedding rows.
 *
 * @details Operates on raw pointer rows (as returned by the flat
 * embedded_values storage in DelayEmbeddingMatrix) rather than on
 * fims::Vector so the loop is visible to TMB's CppAD tracer.
 *
 * @tparam Type Numeric scalar type (double or TMB AD scalar).
 * @param a_row Pointer to the first element of row a (length >= dim).
 * @param b_row Pointer to the first element of row b (length >= dim).
 * @param dim   Number of coordinates per row (embedding dimension E).
 * @return Squared Euclidean distance between the two rows.
 * @throws std::invalid_argument if dim == 0.
 */
template <typename Type>
Type SquaredEuclideanDistance(const Type* a_row, const Type* b_row,
                              size_t dim) {
  if (dim == 0) {
    throw std::invalid_argument(
        "SquaredEuclideanDistance: dim must be greater than 0.");
  }
  Type dist = Type(0);
  for (size_t k = 0; k < dim; ++k) {
    Type diff = a_row[k] - b_row[k];
    dist += diff * diff;
  }
  return dist;
}

// ---------------------------------------------------------------------------
// Normalization
// ---------------------------------------------------------------------------

/**
 * @brief Normalize a weight vector in-place so that the weights sum to 1.
 *
 * @details All weights are divided by their total sum. If the sum is zero
 * (e.g., all distances are infinite and all weights are zero), a
 * std::runtime_error is thrown.
 *
 * @tparam Type Numeric scalar type.
 * @param weights Weight vector to normalize in-place.
 * @throws std::runtime_error if the weight sum is effectively zero.
 */
template <typename Type>
void NormalizeWeights(std::vector<Type>& weights) {
  Type total = Type(0);
  for (size_t i = 0; i < weights.size(); ++i) {
    total += weights[i];
  }
  if (total <= Type(0)) {
    throw std::runtime_error(
        "NormalizeWeights: weight sum is zero. "
        "All distances may be infinite or weights all zero.");
  }
  for (size_t i = 0; i < weights.size(); ++i) {
    weights[i] /= total;
  }
}

// ---------------------------------------------------------------------------
// Simplex Projection weights
// ---------------------------------------------------------------------------

/**
 * @brief Compute exponential weights for Simplex Projection.
 *
 * @details Implements the Sugihara & May (1990) exponential weighting scheme:
 * \f[
 *   w_i = \exp\!\Bigl(-\frac{d_i}{d_{\min} + \varepsilon}\Bigr)
 * \f]
 * where \f$d_{\min}\f$ is the smallest distance in @p sq_distances and
 * \f$\varepsilon\f$ guards against division by zero when the query is an
 * exact match.
 *
 * Weights are NOT normalized here; call NormalizeWeights() separately if
 * required, or divide by their sum manually.
 *
 * @tparam Type Numeric scalar type.
 * @param sq_distances  Vector of squared Euclidean distances from the query
 *                      point to each selected nearest neighbor.
 * @param[out] weights  Output weight vector. Resized to sq_distances.size().
 * @param epsilon       Small constant added to d_min (default 1e-12).
 * @throws std::invalid_argument if sq_distances is empty.
 */
template <typename Type>
void SimplexWeights(const std::vector<Type>& sq_distances,
                    std::vector<Type>& weights, double epsilon = 1e-12) {
  if (sq_distances.empty()) {
    throw std::invalid_argument("SimplexWeights: sq_distances must not be empty.");
  }

  // Find d_min
  Type d_min = sq_distances[0];
  for (size_t i = 1; i < sq_distances.size(); ++i) {
    if (sq_distances[i] < d_min) {
      d_min = sq_distances[i];
    }
  }

  weights.resize(sq_distances.size());
  for (size_t i = 0; i < sq_distances.size(); ++i) {
    Type exponent = sq_distances[i] / (d_min + Type(epsilon));
    weights[i] = fims_math::exp(-exponent);
  }
}

// ---------------------------------------------------------------------------
// S-Map weights
// ---------------------------------------------------------------------------

/**
 * @brief Compute S-Map weights with a user-selectable kernel.
 *
 * @details Two kernel choices are available via the SMapKernel enum:
 *
 * **Exponential** (SMapKernel::kExponential, default — Sugihara 1994 / rEDM):
 * \f[
 *   w_i = \exp\!\Bigl(-\theta \cdot \frac{d_i}{\bar{d}}\Bigr)
 * \f]
 * where \f$\bar{d}\f$ is the mean squared distance across ALL library rows.
 *
 * **Gaussian** (SMapKernel::kGaussian — Esguerra & Munch 2024):
 * \f[
 *   w_i = \exp\!\Bigl(-\theta^2 \cdot \Bigl(\frac{d_i}{D}\Bigr)^2\Bigr)
 * \f]
 * where \f$D\f$ is the mean squared distance (same denominator, kept squared
 * to avoid a sqrt() inside the AD tape).
 *
 * For \f$\theta = 0\f$ both kernels reduce to \f$w_i = 1\f$ (global OLS).
 * Per Munch (pers. comm.) the performance difference between kernels is
 * rarely large; the Gaussian kernel avoids a sqrt() inside the AD tape.
 *
 * Weights are NOT normalized here; call NormalizeWeights() separately.
 *
 * @tparam Type Numeric scalar type (double or TMB AD scalar).
 * @param sq_distances  Squared Euclidean distances from the query point to
 *                      every library row (length = n_library_rows).
 * @param[out] weights  Output weight vector, resized to sq_distances.size().
 * @param theta         Nonlinearity parameter theta >= 0 (default 1.0).
 * @param kernel        Kernel choice: SMapKernel::kExponential (default) or
 *                      SMapKernel::kGaussian.
 * @param epsilon       Small constant added to the scale denominator to guard
 *                      against division by zero (default 1e-12).
 * @throws std::invalid_argument if sq_distances is empty or theta < 0.
 */
template <typename Type>
void SMapWeights(const std::vector<Type>& sq_distances,
                 std::vector<Type>& weights, double theta = 1.0,
                 SMapKernel kernel = SMapKernel::kExponential,
                 double epsilon = 1e-12) {
  if (sq_distances.empty()) {
    throw std::invalid_argument("SMapWeights: sq_distances must not be empty.");
  }
  if (theta < 0.0) {
    throw std::invalid_argument("SMapWeights: theta must be >= 0.");
  }

  // Compute mean squared distance across all library rows (used by both
  // kernels as the scale denominator D or d̄).
  Type d_mean = Type(0);
  for (size_t i = 0; i < sq_distances.size(); ++i) {
    d_mean += sq_distances[i];
  }
  d_mean /= Type(sq_distances.size());

  weights.resize(sq_distances.size());
  for (size_t i = 0; i < sq_distances.size(); ++i) {
    Type exponent;
    if (kernel == SMapKernel::kGaussian) {
      // Gaussian kernel: exp(-θ² · (d_i / D)²)
      // d_i and D are already squared, so (d_i/D)² = d_i²/D².  Keeping
      // everything in squared-distance units avoids any sqrt() call.
      Type ratio = sq_distances[i] / (d_mean + Type(epsilon));
      exponent = Type(theta * theta) * ratio * ratio;
    } else {
      // Exponential kernel (default): exp(-θ · d_i / d̄)
      exponent = Type(theta) * sq_distances[i] / (d_mean + Type(epsilon));
    }
    weights[i] = fims_math::exp(-exponent);
  }
}

}  // namespace fims_edm

#endif  // FIMS_EDM_DISTANCE_WEIGHTS_HPP
