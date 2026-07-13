/**
 * @file alk_runtime.hpp
 * @brief Shared runtime helpers for constructing and validating fleet ALKs.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef POPULATION_DYNAMICS_ALK_RUNTIME_HPP
#define POPULATION_DYNAMICS_ALK_RUNTIME_HPP

#include <cstddef>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "alk_base.hpp"
#include "fixed_matrix_alk.hpp"
#include "growth_derived_alk.hpp"
#include "../../population/population.hpp"

namespace fims_popdy {

/**
 * @brief Build the active ALK for a fleet from the current population and fleet
 * state.
 *
 * Tries the growth-derived ALK path when the linked population growth object
 * exposes the required observation capability. That migrated path depends
 * on a configured population size provider and a valid population biological
 * size grid so prepared population age-to-size products can be mapped into
 * fleet observation bins. When the growth-derived capability exists, that path
 * must succeed or the build fails. Fixed age-to-length conversion is only used
 * for growth objects that do not expose the growth-derived observation
 * capability.
 *
 * @param population Shared pointer to the owning population.
 * @param fleet Shared pointer to the fleet.
 * @return Shared pointer to an active ALK implementation, or nullptr when no
 * usable ALK path exists.
 */
template <typename Type>
std::shared_ptr<ALKBase<Type>> BuildFleetALK(
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
    std::shared_ptr<ALKBase<Type>> growth_alk =
        std::make_shared<GrowthDerivedALK<Type>>(
            fleet,
            growth_observation,
            population->size_distribution_provider);

    if (growth_alk->IsActive() &&
        growth_alk->PrepareForCurrentState()) {
      FIMS_INFO_LOG("Growth-derived ALK successfully set to fleet " +
                    fims::to_string(fleet->id) + " for population " +
                    fims::to_string(population->id));
      return growth_alk;
    }
    return nullptr;
  }

  std::shared_ptr<ALKBase<Type>> fixed_alk =
      std::make_shared<FixedMatrixALK<Type>>(fleet);

  if (fixed_alk->IsActive() &&
      fixed_alk->PrepareForCurrentState()) {
    FIMS_INFO_LOG("Fixed age-to-length matrix successfully set to fleet " +
                  fims::to_string(fleet->id));
    return fixed_alk;
  }

  return nullptr;
}

/**
 * @brief Ensure a fleet has an active ALK before length-based calculations.
 *
 * Reuses the current fleet ALK when it matches the current population growth
 * path and can prepare for the current model state. For the migrated
 * growth-derived path, this preparation includes both upstream growth products
 * and linked population size-provider products. Otherwise rebuilds the fleet
 * ALK from the current population and fleet state. Throws when the fleet has
 * length bins but no usable ALK path can be constructed.
 *
 * @param population Shared pointer to the owning population.
 * @param fleet Shared pointer to the fleet.
 */
template <typename Type>
void EnsureFleetALK(const std::shared_ptr<Population<Type>>& population,
                    const std::shared_ptr<Fleet<Type>>& fleet) {
  if (fleet == nullptr) {
    throw std::runtime_error("Fleet pointer was null while resolving ALK.");
  }

  if (population == nullptr) {
    throw std::runtime_error("Population pointer was null while resolving ALK.");
  }

  if (!fleet->requires_age_length_mapping || fleet->n_lengths == 0) {
    return;
  }

  std::shared_ptr<GrowthDerivedObservationBase<Type>> growth_observation =
      std::dynamic_pointer_cast<GrowthDerivedObservationBase<Type>>(
          population->growth);

  if (fleet->alk != nullptr) {
    if (growth_observation != nullptr) {
      std::shared_ptr<GrowthDerivedALK<Type>> growth_alk =
          std::dynamic_pointer_cast<GrowthDerivedALK<Type>>(fleet->alk);

      if (growth_alk != nullptr &&
          growth_alk->IsActive() &&
          growth_alk->PrepareForCurrentState()) {
        return;
      }
    } else if (fleet->alk->IsActive() &&
               fleet->alk->PrepareForCurrentState()) {
      return;
    }
  }

  fleet->alk = BuildFleetALK<Type>(population, fleet);

  if (fleet->alk != nullptr) {
    return;
  }

  std::stringstream ss;
  ss << "Fleet " << fleet->GetId()
     << " has length composition bins but no usable age-to-length conversion path.";

  if (growth_observation != nullptr) {
    ss << " This population uses a growth-derived-capable growth object, so "
       << "the growth-derived ALK path with population size-provider support "
       << "was required and could not be built or prepared.";
  } else {
    ss << " Provide a valid fixed age-to-length conversion matrix for this "
       << "fleet or use a supported growth-derived-capable growth object.";
  }

  FIMS_ERROR_LOG(ss.str());
  throw std::runtime_error(ss.str());
}

/**
 * @brief Ensure all fleets linked to a population have a usable ALK.
 *
 * Applies EnsureFleetALK() to each fleet linked to the population so runtime
 * length-based calculations can safely assume ALK availability.
 *
 * @param population Shared pointer to the population whose fleets should be
 * checked.
 */
template <typename Type>
void EnsurePopulationFleetALKs(
    const std::shared_ptr<Population<Type>>& population) {
  if (population == nullptr) {
    throw std::runtime_error("Population pointer was null while resolving ALKs.");
  }

  for (size_t i = 0; i < population->fleets.size(); ++i) {
    EnsureFleetALK<Type>(population, population->fleets[i]);
  }
}

}  // namespace fims_popdy

#endif
