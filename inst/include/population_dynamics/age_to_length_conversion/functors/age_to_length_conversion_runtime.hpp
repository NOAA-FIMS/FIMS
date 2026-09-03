/**
 * @file age_to_length_conversion_runtime.hpp
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
#include <sstream>
#include <stdexcept>

#include "age_to_length_conversion_base.hpp"
#include "age_to_length_conversion_fixed.hpp"
#include "age_to_length_conversion_derived.hpp"
#include "../../population/population.hpp"

namespace fims_popdy {

/**
 * @brief Build the active age-to-length conversion for a fleet from the current
 * population and fleet state.
 *
 * Tries the growth-derived age-to-length conversion path when the linked
 * population growth object exposes the required observation capability. That
 * migrated path depends on a configured population size provider and a valid
 * population biological size grid so prepared population age-to-size products
 * can be mapped into fleet observation bins. When the growth-derived capability
 * exists, that path must succeed or the build fails. Fixed age-to-length
 * conversion is only used for growth objects that do not expose the
 * growth-derived observation capability.
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

    if (age_to_length_conversion_derived->IsActive() &&
        age_to_length_conversion_derived->PrepareForCurrentState()) {
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

  if (age_to_length_conversion_fixed->IsActive() &&
      age_to_length_conversion_fixed->PrepareForCurrentState()) {
    FIMS_INFO_LOG("Fixed age-to-length matrix successfully set to fleet " +
                  fims::to_string(fleet->id));
    return age_to_length_conversion_fixed;
  }

  return nullptr;
}

/**
 * @brief Ensure a fleet has an active age-to-length conversion before
 * length-based calculations.
 *
 * Reuses the current fleet age-to-length conversion when it matches the current
 * population growth path and can prepare for the current model state. For the
 * migrated growth-derived path, this preparation includes both upstream growth
 * products and linked population size-provider products. Otherwise rebuilds the
 * fleet age-to-length conversion from the current population and fleet state.
 * Throws when the fleet has length bins but no usable age-to-length conversion
 * path can be constructed.
 *
 * @param population Shared pointer to the owning population.
 * @param fleet Shared pointer to the fleet.
 */
template <typename Type>
void EnsureAgeToLengthConversionFleet(
    const std::shared_ptr<Population<Type>>& population,
    const std::shared_ptr<Fleet<Type>>& fleet) {
  if (fleet == nullptr) {
    throw std::runtime_error(
        "Fleet pointer was null while resolving age-to-length conversion.");
  }

  if (population == nullptr) {
    throw std::runtime_error(
        "Population pointer was null while resolving age-to-length "
        "conversion.");
  }

  if (!fleet->requires_age_length_mapping || fleet->n_lengths == 0) {
    return;
  }

  std::shared_ptr<GrowthDerivedObservationBase<Type>> growth_observation =
      std::dynamic_pointer_cast<GrowthDerivedObservationBase<Type>>(
          population->growth);

  if (fleet->age_to_length_conversion_model != nullptr) {
    if (growth_observation != nullptr) {
      std::shared_ptr<AgeToLengthConversionDerived<Type>>
          age_to_length_conversion_derived =
              std::dynamic_pointer_cast<AgeToLengthConversionDerived<Type>>(
                  fleet->age_to_length_conversion_model);

      if (age_to_length_conversion_derived != nullptr &&
          age_to_length_conversion_derived->IsActive() &&
          age_to_length_conversion_derived->PrepareForCurrentState()) {
        return;
      }
    } else if (fleet->age_to_length_conversion_model->IsActive() &&
               fleet->age_to_length_conversion_model
                   ->PrepareForCurrentState()) {
      return;
    }
  }

  fleet->age_to_length_conversion_model =
      BuildAgeToLengthConversionFleet<Type>(population, fleet);

  if (fleet->age_to_length_conversion_model != nullptr) {
    return;
  }

  std::stringstream ss;
  ss << "Fleet " << fleet->GetId()
     << " has length composition bins but no usable age-to-length conversion "
        "path.";

  if (growth_observation != nullptr) {
    ss << " This population uses a growth-derived-capable growth object, so "
       << "the growth-derived age-to-length conversion path with population "
          "size-provider support "
       << "was required and could not be built or prepared.";
  } else {
    ss << " Provide a valid fixed age-to-length conversion matrix for this "
       << "fleet or use a supported growth-derived-capable growth object.";
  }

  FIMS_ERROR_LOG(ss.str());
  throw std::runtime_error(ss.str());
}

/**
 * @brief Ensure all fleets linked to a population have a usable age-to-length
 * conversion.
 *
 * Applies EnsureAgeToLengthConversionFleet() to each fleet linked to the
 * population so runtime length-based calculations can safely assume
 * age-to-length conversion availability.
 *
 * @param population Shared pointer to the population whose fleets should be
 * checked.
 */
template <typename Type>
void EnsurePopulationFleetAgeToLengthConversion(
    const std::shared_ptr<Population<Type>>& population) {
  if (population == nullptr) {
    throw std::runtime_error(
        "Population pointer was null while resolving age-to-length "
        "conversions.");
  }

  for (size_t i = 0; i < population->fleets.size(); ++i) {
    EnsureAgeToLengthConversionFleet<Type>(population, population->fleets[i]);
  }
}

}  // namespace fims_popdy

#endif
