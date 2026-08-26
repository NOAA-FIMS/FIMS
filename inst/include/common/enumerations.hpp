/**
 * @file enumerations.hpp
 * @brief Enumerations shared across more than one part of FIMS.
 *
 * @details An enum belongs here when it has users in more than one place --
 * EstimationStatus governs parameter registration throughout the model, and
 * DistributionType is read by both the interface and the Jacobian
 * transformation code (wip). Enums used by a single module family (GrowthType,
 * MaturityType, SelectivityType, and so on) live beside the classes they name,
 * in the corresponding rcpp_*.hpp, so adding a functional form touches that
 * family rather than this shared file.
 *
 * Nothing here may depend on Rcpp: this header is reachable from the model
 * core, which must compile without it. The string converters therefore report
 * failure with std::invalid_argument, which Rcpp catches at the boundary and
 * surfaces as an R error.
 */
#ifndef FIMS_COMMON_ENUMERATIONS_HPP
#define FIMS_COMMON_ENUMERATIONS_HPP

#include <cstdint>
#include <stdexcept>
#include <string>

namespace fims_enum {

/**
 * @brief How a parameter is treated by the estimation machinery.
 */
enum class EstimationStatus : uint8_t {
  kAssumedKnown = 0,
  kFixedEffects = 1,
  kRandomEffects = 2,
  kDerivedQuantity = 3
};

/**
 * @brief Convert an estimation status string to enum.
 */
inline EstimationStatus EstimationStatusFromString(const std::string &status) {
  if (status == "assumed_known") {
    return EstimationStatus::kAssumedKnown;
  }
  if (status == "fixed_effects") {
    return EstimationStatus::kFixedEffects;
  }
  if (status == "random_effects") {
    return EstimationStatus::kRandomEffects;
  }
  if (status == "derived_quantity") {
    return EstimationStatus::kDerivedQuantity;
  }

  throw std::invalid_argument(
      "Invalid estimation_status: " + status +
      ". Valid options are: assumed_known, fixed_effects, random_effects, or "
      "derived_quantity.");
}

/**
 * @brief Convert an estimation status enum to string.
 */
inline std::string EstimationStatusToString(EstimationStatus status) {
  switch (status) {
    case EstimationStatus::kAssumedKnown:
      return "assumed_known";
    case EstimationStatus::kFixedEffects:
      return "fixed_effects";
    case EstimationStatus::kRandomEffects:
      return "random_effects";
    case EstimationStatus::kDerivedQuantity:
      return "derived_quantity";
  }
  return "assumed_known";
}

/**
 * @brief The distributions FIMS can build.
 *
 * @details The create_distribution_() function takes one of these names from R
 * and builds the matching class: "dnorm" builds a DnormDistributionsInterface,
 * and so on.
 */
enum class DistributionType : uint8_t {
  kDnorm = 0,
  kDlnorm = 1,
  kDmultinom = 2
};

/**
 * @brief Convert a distribution name supplied from R to a DistributionType.
 */
inline DistributionType DistributionTypeFromString(const std::string &name) {
  if (name == "dnorm") return DistributionType::kDnorm;
  if (name == "dlnorm") return DistributionType::kDlnorm;
  if (name == "dmultinom") return DistributionType::kDmultinom;

  throw std::invalid_argument(
      "Unsupported distribution '" + name +
      "'. Valid options are: dnorm, dlnorm, dmultinom.");
}

/**
 * @brief Convert a DistributionType to its name.
 */
inline std::string DistributionTypeToString(DistributionType type) {
  switch (type) {
    case DistributionType::kDnorm:
      return "dnorm";
    case DistributionType::kDlnorm:
      return "dlnorm";
    case DistributionType::kDmultinom:
      return "dmultinom";
  }
  return "dnorm";
}

}  // namespace fims_enum

#endif
