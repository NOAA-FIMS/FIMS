/**
 * @file enumerations.hpp
 * @brief Common enumerations used across FIMS interfaces.
 */
#ifndef FIMS_COMMON_ENUMERATIONS_HPP
#define FIMS_COMMON_ENUMERATIONS_HPP

#include <cstdint>

namespace fims_enum {
enum class EstimationStatus : uint8_t {
  kAssumedKnown = 0,
  kFixedEffects = 1,
  kRandomEffects = 2,
  kDerivedQuantity = 3
};
}  // namespace fims_enum

#endif
