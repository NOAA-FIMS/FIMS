/**
 * @file subpopulation.hpp
 * @brief Named biological groups within a population.
 * @copyright See LICENSE in the source folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_POPULATION_SUBPOPULATION_HPP
#define FIMS_POPULATION_DYNAMICS_POPULATION_SUBPOPULATION_HPP

#include <cmath>
#include <cstdint>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace fims_popdy {

/**
 * @brief Configuration of a group with its own abundance and survival.
 *
 * Fractions allocate pooled initial abundance and total recruitment separately.
 * Reproductive contribution multiplies mature biomass; it is not maturity.
 * These are fixed configuration values, validated before model evaluation.
 */
struct Subpopulation {
  std::string name;
  double initial_fraction = 1.0;
  double recruitment_fraction = 1.0;
  double reproductive_contribution = 1.0;
  int growth_id = -999;   /*!< omitted => population growth */
  int maturity_id = -999; /*!< omitted => population maturity */
  std::map<uint32_t, uint32_t>
      selectivity_ids; /*!< fleet id -> selectivity id */
};

/** @brief Validate named groups and conservation of entry allocations. */
inline void ValidateSubpopulations(const std::vector<Subpopulation>& groups) {
  if (groups.empty()) return;  // legacy pooled model
  std::set<std::string> names;
  double initial = 0.0;
  double recruits = 0.0;
  double reproduction = 0.0;
  for (const auto& group : groups) {
    if (group.name.empty() || !names.insert(group.name).second) {
      throw std::invalid_argument(
          "Subpopulation names must be nonempty and unique");
    }
    for (double value : {group.initial_fraction, group.recruitment_fraction,
                         group.reproductive_contribution}) {
      if (!std::isfinite(value) || value < 0.0 || value > 1.0) {
        throw std::invalid_argument(
            "Subpopulation fractions must be finite and in [0, 1]");
      }
    }
    initial += group.initial_fraction;
    recruits += group.recruitment_fraction;
    reproduction +=
        group.recruitment_fraction * group.reproductive_contribution;
  }
  if (std::abs(initial - 1.0) > 1e-10 || std::abs(recruits - 1.0) > 1e-10) {
    throw std::invalid_argument(
        "Initial and recruitment fractions must each sum to one");
  }
  if (reproduction <= 0.0) {
    throw std::invalid_argument(
        "Recruitment must include a reproductive subpopulation");
  }
}

/** @brief Female/male configuration; both sexes retain biological maturity. */
inline std::vector<Subpopulation> MakeTwoSexSubpopulations(
    double initial_female = 0.5, double recruitment_female = 0.5) {
  std::vector<Subpopulation> groups{
      {"female", initial_female, recruitment_female, 1.0},
      {"male", 1.0 - initial_female, 1.0 - recruitment_female, 0.0}};
  ValidateSubpopulations(groups);
  return groups;
}

/** @brief Contiguous group/year/age storage; age varies fastest. */
struct SubpopulationLayout {
  size_t n_years;
  size_t n_ages;
  size_t index(size_t group, size_t year, size_t age) const {
    return (group * n_years + year) * n_ages + age;
  }
};

}  // namespace fims_popdy
#endif
