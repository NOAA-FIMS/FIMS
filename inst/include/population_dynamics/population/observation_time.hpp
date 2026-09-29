/**
 * @file observation_time.hpp
 * @brief Shared observation dates, expressed as model year and elapsed fraction.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_OBSERVATION_TIME_HPP
#define FIMS_OBSERVATION_TIME_HPP

#include <cmath>
#include <memory>
#include <stdexcept>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>

namespace fims_popdy {
struct CalendarDate {
  int year;
  double fraction;
};

inline bool IsAnnualTiming(const std::string& timing) {
  return timing.size() == 10 && timing.substr(4) == "-12-31";
}

inline CalendarDate ParseObservationDate(const std::string& date) {
  if (date.size() != 10 || date[4] != '-' || date[7] != '-') {
    throw std::invalid_argument("Data timing must use ISO YYYY-MM-DD dates");
  }
  for (size_t i = 0; i < date.size(); ++i) {
    if (i != 4 && i != 7 && (date[i] < '0' || date[i] > '9')) {
      throw std::invalid_argument("Data timing must use ISO YYYY-MM-DD dates");
    }
  }
  const int year = std::stoi(date.substr(0, 4));
  const int month = std::stoi(date.substr(5, 2));
  const int day = std::stoi(date.substr(8, 2));
  const bool leap = year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
  const int days[] = {31, leap ? 29 : 28, 31, 30, 31, 30,
                      31, 31, 30, 31, 30, 31};
  if (month < 1 || month > 12 || day < 1 || day > days[month - 1]) {
    throw std::invalid_argument("Data timing contains an invalid calendar date");
  }
  int elapsed = day - 1;
  for (int m = 1; m < month; ++m) elapsed += days[m - 1];
  return {year, elapsed / (leap ? 366.0 : 365.0)};
}

inline std::string JanuaryFirst(int year) {
  std::ostringstream date;
  date << std::setfill('0') << std::setw(4) << year << "-01-01";
  return date.str();
}

struct ObservationTime {
  size_t year;      /*!< Zero-based model year. */
  double fraction; /*!< Elapsed fraction of that calendar year. */
  std::string date; /*!< Normalized ISO date supplied by R, for reporting. */

  bool operator==(const ObservationTime& other) const {
    return year == other.year && fraction == other.fraction && date == other.date;
  }
};

// Validated once at model creation, before any parameter evaluations.
inline void ValidateObservationTimes(const std::vector<ObservationTime>& times,
                                     size_t n_years) {
  for (size_t i = 0; i < times.size(); ++i) {
    const auto& time = times[i];
    if (time.year >= n_years || !std::isfinite(time.fraction) ||
        time.fraction < 0.0 || time.fraction >= 1.0) {
      throw std::invalid_argument("Invalid observation year or fraction");
    }
    if (i > 0 && (time.year < times[i - 1].year ||
        (time.year == times[i - 1].year &&
         time.fraction <= times[i - 1].fraction))) {
      throw std::invalid_argument("Observation times must be sorted and unique");
    }
  }
}
}  // namespace fims_popdy
#endif
