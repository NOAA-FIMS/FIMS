/**
 * @file recruitment_time.hpp
 * @brief Fixed annual recruitment allocation coordinates, outside the AD tape.
 * @copyright See LICENSE in the source folder for reuse information.
 */
#ifndef FIMS_COMMON_RECRUITMENT_TIME_HPP
#define FIMS_COMMON_RECRUITMENT_TIME_HPP

#include <cmath>
#include <set>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace fims {
/** @brief One fraction of annual recruitment allocated to a fixed date. */
struct RecruitmentTime {
  int year;             /**< Zero-based modeled year. */
  double year_fraction; /**< Elapsed calendar-year fraction. */
  double fraction;      /**< Fraction of the annual recruitment total. */
  std::string phase;    /**< Stable phase identifier. */
  double entry_age =
      -1; /**< Biological entry age; -1 uses the first model age. */
};

/** @brief Validate a complete chronological schedule; empty means legacy mode.
 * @param schedule Fixed recruitment events.
 * @param n_years Number of modeled years, excluding the terminal report year.
 */
inline void ValidateRecruitmentSchedule(
    const std::vector<RecruitmentTime>& schedule, size_t n_years) {
  if (schedule.empty()) return;
  std::vector<double> totals(n_years, 0.0);
  std::vector<std::set<std::string>> phases(n_years);
  std::map<std::string, double> ages;
  int previous_year = -1;
  double previous_time = -1.0;
  for (const auto& event : schedule) {
    if (event.year < 0 || static_cast<size_t>(event.year) >= n_years ||
        !std::isfinite(event.year_fraction) || event.year_fraction < 0 ||
        event.year_fraction >= 1 || !std::isfinite(event.fraction) ||
        event.fraction < 0 || !std::isfinite(event.entry_age) ||
        (event.entry_age < 0 && event.entry_age != -1) ||
        event.phase.find_first_not_of(" \t\r\n") == std::string::npos) {
      throw std::invalid_argument("Invalid recruitment schedule coordinates.");
    }
    if (event.year < previous_year ||
        (event.year == previous_year && event.year_fraction <= previous_time)) {
      throw std::invalid_argument(
          "Recruitment dates must be unique and chronologically ordered.");
    }
    if (!phases[event.year].insert(event.phase).second) {
      throw std::invalid_argument("Duplicate recruitment phase within a year.");
    }
    const auto age = ages.emplace(event.phase, event.entry_age);
    if (!age.second && age.first->second != event.entry_age)
      throw std::invalid_argument(
          "Recruitment entry age must be constant per phase.");
    totals[event.year] += event.fraction;
    previous_year = event.year;
    previous_time = event.year_fraction;
  }
  for (size_t y = 0; y < n_years; ++y) {
    if (std::abs(totals[y] - 1.0) > 1e-8 || phases[y] != phases[0]) {
      throw std::invalid_argument(
          "Recruitment schedules require consistent phases and fractions "
          "summing to one in every modeled year.");
    }
  }
}

/** @brief Whether the unchanged annual evaluator is sufficient. */
inline bool IsAnnualRecruitmentSchedule(
    const std::vector<RecruitmentTime>& schedule, size_t n_years,
    double first_age) {
  if (schedule.empty()) return true;
  if (schedule.size() != n_years) return false;
  for (const auto& event : schedule) {
    if (event.year_fraction != 0 || event.fraction != 1 ||
        (event.entry_age != -1 && event.entry_age != first_age))
      return false;
  }
  return true;
}
}  // namespace fims
#endif
