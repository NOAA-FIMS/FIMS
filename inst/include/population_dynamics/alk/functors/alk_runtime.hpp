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
 * Tries the growth-derived ALK path first when the linked population growth
 * object exposes the required observation capability. Falls back to the fixed
 * age-to-length conversion matrix when the growth-derived path is not usable.
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

  if (std::shared_ptr<GrowthDerivedObservationBase<Type>> growth_observation =
          std::dynamic_pointer_cast<GrowthDerivedObservationBase<Type>>(
              population->growth)) {
    std::shared_ptr<ALKBase<Type>> growth_alk =
        std::make_shared<GrowthDerivedALK<Type>>(fleet, growth_observation);

    if (growth_alk != nullptr &&
        growth_alk->IsActive() &&
        growth_alk->PrepareForCurrentState()) {
      FIMS_INFO_LOG("Growth-derived ALK successfully set to fleet " +
                    fims::to_string(fleet->id) + " for population " +
                    fims::to_string(population->id));
      return growth_alk;
    }
  }

  std::shared_ptr<ALKBase<Type>> fixed_alk =
      std::make_shared<FixedMatrixALK<Type>>(fleet);

  if (fixed_alk != nullptr &&
      fixed_alk->IsActive() &&
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
 * Reuses the current fleet ALK when it is active and can prepare for the
 * current model state. Otherwise rebuilds the fleet ALK from the current
 * population and fleet state. Throws when the fleet has length bins but no
 * usable ALK path can be constructed.
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

  if (fleet->n_lengths == 0) {
    return;
  }

  if (fleet->alk != nullptr &&
      fleet->alk->IsActive() &&
      fleet->alk->PrepareForCurrentState()) {
    return;
  }

  fleet->alk = BuildFleetALK<Type>(population, fleet);

  if (fleet->alk != nullptr) {
    return;
  }

  std::stringstream ss;
  ss << "Fleet " << fleet->GetId()
     << " has length composition bins but no usable "
     << "age-to-length conversion path. Provide fixed "
     << "age-to-length conversion of size "
     << (fleet->n_ages * fleet->n_lengths)
     << " or use a supported growth-derived ALK path.";
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
