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
 *  - SMapWeights(): exponential weights normalized by the mean distance and
 *    scaled by theta (used by S-Map, Sugihara 1994).
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
 * @brief Compute exponential weights for S-Map (Sugihara 1994).
 *
 * @details Implements the S-Map weighting scheme using all library points:
 * \f[
 *   w_i = \exp\!\Bigl(-\theta \cdot \frac{d_i}{\bar{d}}\Bigr)
 * \f]
 * where \f$\bar{d}\f$ is the mean distance across ALL library rows (not
 * just selected neighbors), and \f$\theta \geq 0\f$ controls nonlinearity:
 *  - \f$\theta = 0\f$: all weights are equal (global linear map).
 *  - Large \f$\theta\f$: only close neighbors contribute (highly local).
 *
 * Weights are NOT normalized here; call NormalizeWeights() separately.
 *
 * @tparam Type Numeric scalar type.
 * @param sq_distances  Vector of squared Euclidean distances from the query
 *                      point to every library row (length = n_library_rows).
 * @param[out] weights  Output weight vector. Resized to sq_distances.size().
 * @param theta         Nonlinearity parameter theta >= 0 (default 1.0).
 * @param epsilon       Small constant added to d_mean (default 1e-12).
 * @throws std::invalid_argument if sq_distances is empty or theta < 0.
 */
template <typename Type>
void SMapWeights(const std::vector<Type>& sq_distances,
                 std::vector<Type>& weights, double theta = 1.0,
                 double epsilon = 1e-12) {
  if (sq_distances.empty()) {
    throw std::invalid_argument("SMapWeights: sq_distances must not be empty.");
  }
  if (theta < 0.0) {
    throw std::invalid_argument("SMapWeights: theta must be >= 0.");
  }

  // Compute mean distance across all library rows
  Type d_mean = Type(0);
  for (size_t i = 0; i < sq_distances.size(); ++i) {
    d_mean += sq_distances[i];
  }
  d_mean /= Type(sq_distances.size());

  weights.resize(sq_distances.size());
  for (size_t i = 0; i < sq_distances.size(); ++i) {
    Type exponent = Type(theta) * sq_distances[i] / (d_mean + Type(epsilon));
    weights[i] = fims_math::exp(-exponent);
  }
}

}  // namespace fims_edm

#endif  // FIMS_EDM_DISTANCE_WEIGHTS_HPP
