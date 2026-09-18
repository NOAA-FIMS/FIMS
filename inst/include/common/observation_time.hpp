/** @file observation_time.hpp
 * @brief Fixed sample coordinates, independent of model parameters.
 * @copyright See LICENSE in the source folder for reuse information.
 */
#ifndef FIMS_OBSERVATION_TIME_HPP
#define FIMS_OBSERVATION_TIME_HPP

#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace fims {
/** @brief One sample's zero-based annual index and integer time coordinates. */
struct ObservationTime {
  double fraction;  ///< Elapsed fraction of the Gregorian calendar year.
  size_t year;      ///< Zero-based model year, not a sample index.
  int day;          ///< Whole days from the model epoch.
  int time_id;      ///< Shared time identifier assigned during setup.
  std::string date; ///< Normalized calendar date for reporting.
  std::string
      observation_id; ///< Stable sample identity from normalized support.
};
/** @brief Sample order for each observation stream. */
using ObservationTimes = std::map<std::string, std::vector<ObservationTime>>;
} // namespace fims
#endif
