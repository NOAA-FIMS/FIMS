/**
 * @file catch_at_age_adapter.hpp
 * @brief Copy a reference-year snapshot from an evaluated CatchAtAge model.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_REFERENCE_POINTS_CATCH_AT_AGE_ADAPTER_HPP
#define FIMS_POPULATION_DYNAMICS_REFERENCE_POINTS_CATCH_AT_AGE_ADAPTER_HPP

#include "reference_points.hpp"
#include "../../models/functors/catch_at_age.hpp"

namespace fims_popdy {

/**
 * @brief Copy biological inputs and fixed fleet shares for one modeled year.
 * @details Call after model evaluation so growth products are current. Empty
 * shares use the year's fleet F coefficients normalized to sum to one. Explicit
 * shares follow population->fleets order; zero shares exclude survey fleets.
 * Catch weights follow CatchAtAge's population-weight convention. Parameters
 * and population trajectories are not changed by this adapter.
 */
inline ReferencePointInputs<double> MakeReferencePointInputs(
    CatchAtAge<double>& model,
    const std::shared_ptr<Population<double>>& population, size_t year,
    const std::vector<double>& shares = {}) {
  if (!population || year >= population->n_years || population->n_ages < 2 ||
      population->ages.size() != population->n_ages || !population->maturity ||
      population->log_M.size() < population->n_years * population->n_ages ||
      (population->proportion_female.size() != 1 &&
       population->proportion_female.size() != population->n_ages)) {
    throw std::invalid_argument("Invalid population or reference year");
  }
  const size_t n = population->n_ages;
  for (size_t a = 0; a < n; ++a) {
    if (!std::isfinite(population->ages[a]) ||
        (a > 0 && std::abs(population->ages[a] - population->ages[a - 1] -
                           1.0) > 1e-8)) {
      throw std::invalid_argument(
          "Reference points require consecutive annual ages");
    }
  }
  if (!shares.empty() && shares.size() != population->fleets.size()) {
    throw std::invalid_argument("Provide one share per population fleet");
  }
  ReferencePointInputs<double> inputs;
  for (size_t a = 0; a < n; ++a) {
    inputs.natural_mortality.push_back(
        std::exp(population->log_M[year * n + a]));
    inputs.weight.push_back(model.PopulationMeanWeightAA(population, year, a));
    inputs.maturity.push_back(
        population->maturity->evaluate(population->ages[a]));
    inputs.proportion_female.push_back(
        double(population->proportion_female.get_force_scalar(a)));
  }
  double total_f = 0.0;
  for (size_t f = 0; f < population->fleets.size(); ++f) {
    const auto& fleet = population->fleets[f];
    if (!fleet || !fleet->selectivity || year >= fleet->Fmort.size()) {
      throw std::invalid_argument("Population fleet is not prepared");
    }
    ReferencePointFleet<double> input_fleet;
    input_fleet.share = shares.empty() ? fleet->Fmort[year] : shares[f];
    total_f += input_fleet.share;
    for (size_t a = 0; a < n; ++a) {
      input_fleet.selectivity.push_back(
          fleet->selectivity->evaluate(population->ages[a], year));
      input_fleet.weight.push_back(double(inputs.weight[a]));
    }
    inputs.fleets.push_back(input_fleet);
  }
  if (shares.empty() && !inputs.fleets.empty()) {
    if (!std::isfinite(total_f) || total_f <= 0.0) {
      throw std::invalid_argument(
          "No positive fleet F; supply explicit fleet shares");
    }
    for (auto& fleet : inputs.fleets) fleet.share /= total_f;
  }
  ValidateReferencePointInputs(inputs);
  return inputs;
}

}  // namespace fims_popdy
#endif
