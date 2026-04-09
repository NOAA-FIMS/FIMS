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
 * Prefers the supported growth-derived ALK path when the linked population
 * growth object exposes the required observation capability and the fleet has
 * a valid explicit length-bin definition. Falls back to the fixed
 * age-to-length conversion matrix when available.
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
    if (growth_observation->SupportsGrowthDerivedALK()) {
      const GrowthProducts<Type>& growth_products =
          growth_observation->GetProductsForReporting();

      if (growth_products.n_sexes == 1 &&
          fleet->n_ages > 0 &&
          fleet->n_lengths > 0 &&
          fleet->lengths.size() == fleet->n_lengths) {
        std::shared_ptr<ALKBase<Type>> alk =
            std::make_shared<GrowthDerivedALK<Type>>(
                fleet, growth_observation);
        if (alk != nullptr && alk->IsActive()) {
          FIMS_INFO_LOG("Growth-derived ALK successfully set to fleet " +
                        fims::to_string(fleet->id) + " for population " +
                        fims::to_string(population->id));
          return alk;
        }
      }
    }
  }

  const bool has_fixed_age_to_length_matrix =
      fleet->n_ages > 0 &&
      fleet->n_lengths > 0 &&
      fleet->age_to_length_conversion.size() ==
          (fleet->n_ages * fleet->n_lengths);

  if (has_fixed_age_to_length_matrix) {
    std::shared_ptr<ALKBase<Type>> alk =
        std::make_shared<FixedMatrixALK<Type>>(fleet);

    if (alk != nullptr && alk->IsActive()) {
      FIMS_INFO_LOG("Fixed age-to-length matrix successfully set to fleet " +
                    fims::to_string(fleet->id));
      return alk;
    }
  }

  return nullptr;
}

/**
 * @brief Ensure a fleet has an active ALK before length-based calculations.
 *
 * Rebuilds the fleet ALK from the current population and fleet state when the
 * stored ALK pointer is missing or inactive. Throws when the fleet has length
 * bins but no usable ALK path can be constructed.
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

  if (fleet->alk == nullptr || !fleet->alk->IsActive()) {
    fleet->alk = BuildFleetALK<Type>(population, fleet);
  }

  if (fleet->alk == nullptr || !fleet->alk->IsActive()) {
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
}

/**
 * @brief Ensure all fleets linked to a population have an active ALK.
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
