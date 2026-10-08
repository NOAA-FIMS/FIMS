/**
 * @file per_recruit.hpp
 * @brief Equilibrium age-structured calculations for one annual recruit.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_REFERENCE_POINTS_PER_RECRUIT_HPP
#define FIMS_POPULATION_DYNAMICS_REFERENCE_POINTS_PER_RECRUIT_HPP

#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

#include "../../../common/fims_math.hpp"

namespace fims_popdy {

/** @brief One fishing fleet's fixed share, selectivity, and catch weights. */
template <typename Type>
struct ReferencePointFleet {
  double share = 1.0; /*!< Shares across fleets must sum to one. */
  fims::Vector<Type>
      selectivity;           /*!< Nonnegative; used without normalization. */
  fims::Vector<Type> weight; /*!< Catch weight at age, in biomass units. */
  bool include_in_msy =
      true; /*!< Count yield in the objective, without changing mortality. */
};

/**
 * @brief A fixed biological snapshot; all vectors have one entry per age.
 * @details Ages are annual, recruitment enters the first age, and the last age
 * is a plus group (including when there is only one age). Natural mortality
 * must be positive. Biomass and spawning biomass are measured at the start of
 * the year, matching CatchAtAge. Include fishing fleets only. Fleet mortality
 * is F * share * selectivity; F is the sum of fleet coefficients, not
 * necessarily the maximum fishing mortality at age. No averaging or implicit
 * selectivity normalization is performed here.
 */
template <typename Type>
struct ReferencePointInputs {
  fims::Vector<Type> natural_mortality; /*!< Annual instantaneous mortality. */
  fims::Vector<Type> weight;            /*!< Population weight at age. */
  fims::Vector<Type> maturity;          /*!< Mature fraction at age. */
  fims::Vector<Type> proportion_female; /*!< Female fraction at age. */
  std::vector<ReferencePointFleet<Type>> fleets; /*!< Fixed fishing pattern. */
};

/** @brief Equilibrium quantities per annual recruit. */
template <typename Type>
struct PerRecruitResult {
  fims::Vector<Type> numbers;  /*!< Numbers at age at the start of the year. */
  Type spawning_biomass = 0.0; /*!< Mature female biomass per recruit. */
  Type biomass = 0.0;          /*!< Total biomass per recruit. */
  Type yield = 0.0;            /*!< Total annual catch weight per recruit. */
  Type objective_yield =
      0.0; /*!< Annual yield per recruit from included fleets. */
  fims::Vector<Type> fleet_yield; /*!< Annual catch weight by fleet. */
};

/** @brief Check dimensions and biological ranges before calculation. */
template <typename Type>
void ValidateReferencePointInputs(const ReferencePointInputs<Type>& inputs) {
  const size_t n = inputs.natural_mortality.size();
  if (n == 0) {
    throw std::invalid_argument("Reference points require at least one age");
  }
  auto check_vector = [n](const fims::Vector<Type>& values, double upper,
                          bool positive) {
    if (values.size() != n) {
      throw std::invalid_argument("Reference point vectors must match ages");
    }
    for (size_t a = 0; a < n; ++a) {
      const double value = fims_math::Value(values[a]);
      if (!std::isfinite(value) || value < 0.0 || value > upper ||
          (positive && value == 0.0)) {
        throw std::invalid_argument("Invalid reference point biological value");
      }
    }
  };
  const double unlimited = std::numeric_limits<double>::infinity();
  check_vector(inputs.natural_mortality, unlimited, true);
  check_vector(inputs.weight, unlimited, false);
  check_vector(inputs.maturity, 1.0, false);
  check_vector(inputs.proportion_female, 1.0, false);
  double shares = 0.0;
  for (const auto& fleet : inputs.fleets) {
    if (!std::isfinite(fleet.share) || fleet.share < 0.0 || fleet.share > 1.0) {
      throw std::invalid_argument("Fleet shares must be between zero and one");
    }
    shares += fleet.share;
    check_vector(fleet.selectivity, unlimited, false);
    check_vector(fleet.weight, unlimited, false);
  }
  if (!inputs.fleets.empty() && std::abs(shares - 1.0) > 1e-10) {
    throw std::invalid_argument("Fleet shares must sum to one");
  }
}

/**
 * @brief Calculate equilibrium survival, biomass, and Baranov catch.
 * @details Inputs are never modified. The arithmetic is templated, but the
 * input validation is value-based; this is not a differentiable root solver.
 */
template <typename Type>
PerRecruitResult<Type> CalculatePerRecruit(
    const ReferencePointInputs<Type>& inputs, const Type& fishing_mortality) {
  ValidateReferencePointInputs(inputs);
  const double f = fims_math::Value(fishing_mortality);
  if (!std::isfinite(f) || f < 0.0) {
    throw std::invalid_argument(
        "Fishing mortality must be finite and nonnegative");
  }
  const size_t n = inputs.natural_mortality.size();
  fims::Vector<Type> mortality(n);
  fims::Vector<Type> survival(n);
  for (size_t a = 0; a < n; ++a) {
    mortality[a] = inputs.natural_mortality[a];
    for (const auto& fleet : inputs.fleets) {
      mortality[a] += fishing_mortality * fleet.share * fleet.selectivity[a];
    }
    if (!std::isfinite(fims_math::Value(mortality[a]))) {
      throw std::domain_error("Reference point mortality overflow");
    }
    survival[a] = fims_math::exp(-mortality[a]);
  }

  PerRecruitResult<Type> result;
  result.numbers.resize(n);
  result.fleet_yield = fims::Vector<Type>(inputs.fleets.size(), Type(0.0));
  result.numbers[0] = 1.0;
  for (size_t a = 1; a < n; ++a) {
    result.numbers[a] = result.numbers[a - 1] * survival[a - 1];
  }
  const Type plus_group_loss = Type(1.0) - survival[n - 1];
  if (fims_math::Value(plus_group_loss) <= 0.0) {
    throw std::domain_error(
        "Plus-group mortality is too small for equilibrium");
  }
  result.numbers[n - 1] /= plus_group_loss;

  for (size_t a = 0; a < n; ++a) {
    result.biomass += result.numbers[a] * inputs.weight[a];
    result.spawning_biomass += result.numbers[a] * inputs.weight[a] *
                               inputs.maturity[a] * inputs.proportion_female[a];
    for (size_t fleet = 0; fleet < inputs.fleets.size(); ++fleet) {
      const auto& fishing = inputs.fleets[fleet];
      const Type fleet_mortality =
          fishing_mortality * fishing.share * fishing.selectivity[a];
      result.fleet_yield[fleet] +=
          result.numbers[a] * (fleet_mortality / mortality[a]) *
          (Type(1.0) - survival[a]) * fishing.weight[a];
    }
  }
  for (size_t fleet = 0; fleet < inputs.fleets.size(); ++fleet) {
    result.yield += result.fleet_yield[fleet];
    if (inputs.fleets[fleet].include_in_msy) {
      result.objective_yield += result.fleet_yield[fleet];
    }
  }
  if (!std::isfinite(fims_math::Value(result.biomass)) ||
      !std::isfinite(fims_math::Value(result.spawning_biomass)) ||
      !std::isfinite(fims_math::Value(result.yield)) ||
      !std::isfinite(fims_math::Value(result.objective_yield))) {
    throw std::domain_error("Nonfinite reference point result");
  }
  return result;
}

}  // namespace fims_popdy
#endif
