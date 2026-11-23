/** @file observation_link.hpp @brief Pool fleet predictions before likelihoods.
 */
#ifndef FIMS_FLEET_OBSERVATION_LINK_HPP
#define FIMS_FLEET_OBSERVATION_LINK_HPP
#include <cstdint>
#include <set>
#include <stdexcept>
#include <vector>
namespace fims_popdy {
/** @brief An observation-only fleet receiving predictions from dynamic
 * components. */
struct FleetObservationLink {
  uint32_t target_id;
  std::vector<uint32_t> source_ids;
  bool use_catch;
  void Validate() const {
    if (source_ids.size() < 2) {
      throw std::invalid_argument(
          "Observation links require at least two source fleets");
    }
    std::set<uint32_t> unique;
    for (auto id : source_ids) {
      if (id == target_id || !unique.insert(id).second) {
        throw std::invalid_argument(
            "Observation target and source fleets must be distinct");
      }
    }
  }
};
} // namespace fims_popdy
#endif
