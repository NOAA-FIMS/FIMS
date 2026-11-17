/**
 * @file runtime.hpp
 * @brief Shared runtime helpers for constructing and validating fleet
 * age-to-length conversions.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_AGE_TO_LENGTH_CONVERSION_RUNTIME_HPP
#define POPULATION_DYNAMICS_AGE_TO_LENGTH_CONVERSION_RUNTIME_HPP

#include <cstddef>
#include <memory>

#include "base.hpp"
#include "fixed.hpp"
#include "derived.hpp"
#include "population_dynamics/population/population.hpp"

namespace fims_popdy {

/**
 * @brief Build the active age-to-length conversion for a fleet from the current
 * population and fleet state.
 *
 * Tries the growth-derived age-to-length conversion path when the linked
 * population growth object exposes the required observation capability. That
 * path requires a configured population size provider and valid fleet
 * observation-bin geometry. This construction helper validates configuration
 * only; CatchAtAge prepares Growth and size products during evaluation before
 * the conversion is used. Fixed age-to-length conversion is only used for
 * growth objects that do not expose the growth-derived observation capability.
 *
 * @param population Shared pointer to the owning population.
 * @param fleet Shared pointer to the fleet.
 * @return Shared pointer to an active age-to-length conversion implementation,
 * or nullptr when no usable age-to-length conversion path exists.
 */
template <typename Type>
std::shared_ptr<AgeToLengthConversionBase<Type>>
BuildAgeToLengthConversionFleet(
    const std::shared_ptr<Population<Type>>& population,
    const std::shared_ptr<Fleet<Type>>& fleet) {
  if (population == nullptr || fleet == nullptr) {
    return nullptr;
  }

  if (!fleet->requires_age_length_mapping || fleet->n_lengths == 0) {
    return nullptr;
  }

  if (std::shared_ptr<GrowthDerivedObservationBase<Type>> growth_observation =
          std::dynamic_pointer_cast<GrowthDerivedObservationBase<Type>>(
              population->growth)) {
    std::shared_ptr<AgeToLengthConversionBase<Type>>
        age_to_length_conversion_derived =
            std::make_shared<AgeToLengthConversionDerived<Type>>(
                fleet, growth_observation,
                population->size_distribution_provider);

    if (age_to_length_conversion_derived->IsActive()) {
      FIMS_INFO_LOG(
          "Growth-derived age-to-length conversion successfully set to fleet " +
          fims::to_string(fleet->id) + " for population " +
          fims::to_string(population->id));
      return age_to_length_conversion_derived;
    }
    return nullptr;
  }

  std::shared_ptr<AgeToLengthConversionBase<Type>>
      age_to_length_conversion_fixed =
          std::make_shared<AgeToLengthConversionFixed<Type>>(fleet);

  if (age_to_length_conversion_fixed->IsActive()) {
    FIMS_INFO_LOG("Fixed age-to-length matrix successfully set to fleet " +
                  fims::to_string(fleet->id));
    return age_to_length_conversion_fixed;
  }

  return nullptr;
}


}  // namespace fims_popdy

#endif
