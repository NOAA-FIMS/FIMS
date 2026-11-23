/** @file parameter_link.hpp @brief Explicit ownership of shared fleet
 * parameters. */
#ifndef FIMS_FLEET_PARAMETER_LINK_HPP
#define FIMS_FLEET_PARAMETER_LINK_HPP
#include <cstdint>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>
namespace fims_popdy {
/** @brief Followers use an owner's log F or log q; only the owner is estimated.
 */
struct FleetParameterLink {
  uint32_t owner_id;
  std::vector<uint32_t> follower_ids;
  bool fishing_mortality;
  void Validate() const {
    if (follower_ids.empty())
      throw std::invalid_argument(
          "Shared fleet parameters need at least one follower");
    std::set<uint32_t> unique;
    for (auto id : follower_ids) {
      if (id == owner_id || !unique.insert(id).second) {
        throw std::invalid_argument(
            "Parameter owner and followers must be distinct");
      }
    }
  }
};
/** @brief Reject duplicate ownership, cycles, and indirect chains per
 * parameter. */
inline void
ValidateFleetParameterLinks(const std::vector<FleetParameterLink> &links) {
  std::set<std::pair<bool, uint32_t>> owners, followers;
  for (const auto &link : links) {
    link.Validate();
    owners.emplace(link.fishing_mortality, link.owner_id);
    for (auto id : link.follower_ids) {
      if (!followers.emplace(link.fishing_mortality, id).second) {
        throw std::invalid_argument(
            "A fleet parameter can have only one owner");
      }
    }
  }
  for (const auto &follower : followers) {
    if (owners.count(follower)) {
      throw std::invalid_argument(
          "Chained or cyclic fleet parameter links are not supported");
    }
  }
}
} // namespace fims_popdy
#endif
