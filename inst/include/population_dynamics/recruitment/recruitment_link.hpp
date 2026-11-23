/**
 * @file recruitment_link.hpp
 * @brief Fixed recruitment allocations connecting independently tracked
 * populations.
 */
#ifndef FIMS_RECRUITMENT_LINK_HPP
#define FIMS_RECRUITMENT_LINK_HPP

#include <cmath>
#include <cstdint>
#include <set>
#include <stdexcept>
#include <vector>

namespace fims_popdy {

/** @brief Share of total stock recruits entering one population. */
struct RecruitmentAllocation {
  uint32_t population_id;
  double fraction;
};

/**
 * @brief Populations sharing one stock-recruitment mean and process.
 *
 * Spawning output is summed over recipients. Their maturity modules determine
 * reproductive contribution. Fractions allocate both annual and unfished
 * recruitment; R0 is total stock recruitment. Initial abundance remains an
 * independent input for each population.
 */
struct RecruitmentLink {
  std::vector<RecruitmentAllocation> allocations;

  /** @brief Validate fixed allocations independently of model state. */
  void Validate() const {
    if (allocations.size() < 2) {
      throw std::invalid_argument(
          "Recruitment links require at least two populations");
    }
    std::set<uint32_t> ids;
    double sum = 0.0;
    for (const auto &allocation : allocations) {
      if (!ids.insert(allocation.population_id).second) {
        throw std::invalid_argument("Duplicate population in recruitment link");
      }
      if (!std::isfinite(allocation.fraction) || allocation.fraction < 0.0 ||
          allocation.fraction > 1.0) {
        throw std::invalid_argument(
            "Recruitment allocations must be finite and in [0, 1]");
      }
      sum += allocation.fraction;
    }
    if (std::abs(sum - 1.0) > 1e-10) {
      throw std::invalid_argument("Recruitment allocations must sum to one");
    }
  }
};

} // namespace fims_popdy
#endif
