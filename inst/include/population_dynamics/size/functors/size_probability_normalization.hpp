/**
 * @file size_probability_normalization.hpp
 * @brief Defines shared helpers for normalizing probability rows in the size subsystem.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_SIZE_PROBABILITY_NORMALIZATION_HPP
#define FIMS_POPULATION_DYNAMICS_SIZE_PROBABILITY_NORMALIZATION_HPP

#include "../../../common/fims_math.hpp"

namespace fims_popdy {

/**
 * @brief Shared helpers for stable probability-row normalization.
 *
 * The minimum denominator is protected by a smooth maximum so the helper
 * remains AD-friendly while avoiding meaningful bias when row sums are already
 * near 1.
 */
struct SizeProbabilityNormalization {
  template <typename Type>
  static Type SafeDenominator(
      const Type& row_sum,
      const Type& minimum_sum = static_cast<Type>(1e-12)) {
    const Type smoothing = minimum_sum * minimum_sum;
    return fims_math::ad_max(row_sum, minimum_sum, smoothing);
  }
};

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_SIZE_PROBABILITY_NORMALIZATION_HPP */
