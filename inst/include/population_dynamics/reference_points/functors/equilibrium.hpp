/**
 * @file equilibrium.hpp
 * @brief Equilibrium biomass and yield under Beverton--Holt recruitment.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_REFERENCE_POINTS_EQUILIBRIUM_HPP
#define FIMS_POPULATION_DYNAMICS_REFERENCE_POINTS_EQUILIBRIUM_HPP

#include "per_recruit.hpp"

namespace fims_popdy {

/** @brief Mean recruitment parameters, without recruitment deviations. */
struct ReferencePointRecruitment {
  double rzero; /*!< Unfished recruitment in the stock--recruit relationship. */
  double
      steepness; /*!< Beverton--Holt steepness, strictly between 0.2 and 1. */
  double phi0;   /*!< Unfished SBPR defining the stock--recruit baseline. */
};

/** @brief Validate the supported Beverton--Holt parameterization. */
inline void ValidateReferencePointRecruitment(
    const ReferencePointRecruitment& recruitment) {
  if (!std::isfinite(recruitment.rzero) || recruitment.rzero <= 0.0 ||
      !std::isfinite(recruitment.steepness) || recruitment.steepness <= 0.2 ||
      recruitment.steepness >= 1.0 || !std::isfinite(recruitment.phi0) ||
      recruitment.phi0 <= 0.0) {
    throw std::invalid_argument(
        "Beverton-Holt reference points require R0 > 0, 0.2 < h < 1, and phi0 "
        "> 0");
  }
}

/** @brief Equilibrium quantities, including extinction when no positive root
 * exists. */
struct EquilibriumResult {
  double recruitment = 0.0;         /*!< Annual recruitment. */
  double biomass = 0.0;             /*!< Total population biomass. */
  double spawning_biomass = 0.0;    /*!< Mature female biomass. */
  double yield = 0.0;               /*!< Total annual catch weight. */
  fims::Vector<double> fleet_yield; /*!< Annual catch weight by fleet. */
  bool collapsed = false; /*!< True when there is no positive equilibrium. */
};

/**
 * @brief Scale per-recruit quantities by the positive recruitment equilibrium.
 * @details Substituting S = R * SBPR into the FIMS Beverton--Holt equation
 * gives R/R0 = [0.8 h - 0.2 (1-h) phi0/SBPR] / (h-0.2). Nonpositive solutions
 * mean extinction. The supplied phi0 preserves the stock--recruit baseline even
 * when the reference year's biology differs.
 */
inline EquilibriumResult CalculateEquilibrium(
    const ReferencePointInputs<double>& inputs, double fishing_mortality,
    const ReferencePointRecruitment& recruitment) {
  ValidateReferencePointRecruitment(recruitment);
  const auto pr = CalculatePerRecruit(inputs, fishing_mortality);
  EquilibriumResult result;
  result.fleet_yield = fims::Vector<double>(inputs.fleets.size(), 0.0);
  const double h = recruitment.steepness;
  const double replacement_threshold = recruitment.phi0 * (1.0 - h) / (4.0 * h);
  if (pr.spawning_biomass <= replacement_threshold) {
    result.collapsed = true;
    return result;
  }
  result.recruitment =
      recruitment.rzero *
      (0.8 * h - 0.2 * (1.0 - h) * (recruitment.phi0 / pr.spawning_biomass)) /
      (h - 0.2);
  result.biomass = result.recruitment * pr.biomass;
  result.spawning_biomass = result.recruitment * pr.spawning_biomass;
  result.yield = result.recruitment * pr.yield;
  for (size_t f = 0; f < inputs.fleets.size(); ++f) {
    result.fleet_yield[f] = result.recruitment * pr.fleet_yield[f];
  }
  if (!std::isfinite(result.recruitment) || !std::isfinite(result.biomass) ||
      !std::isfinite(result.spawning_biomass) || !std::isfinite(result.yield)) {
    throw std::domain_error("Nonfinite equilibrium result");
  }
  return result;
}

}  // namespace fims_popdy
#endif
