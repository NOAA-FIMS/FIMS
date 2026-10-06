/**
 * @file spr.hpp
 * @brief Bounded post-fit solver for spawning potential ratio targets.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_REFERENCE_POINTS_SPR_HPP
#define FIMS_POPULATION_DYNAMICS_REFERENCE_POINTS_SPR_HPP

#include "per_recruit.hpp"

namespace fims_popdy {

/** @brief Distinguish success, insufficient bounds, and exhausted iterations.
 */
enum class SPRStatus { converged, not_bracketed, iteration_limit };

/** @brief Solver bounds and absolute tolerance on the SPR residual. */
struct SPROptions {
  double max_f = 5.0;      /*!< Search interval is [0, max_f]. */
  double tolerance = 1e-8; /*!< Absolute tolerance on achieved SPR - target. */
  size_t max_iterations = 100; /*!< Maximum number of bisection steps. */
};

/** @brief A candidate F and diagnostics; check status before using F. */
struct SPRResult {
  double fishing_mortality =
      0.0;               /*!< Candidate fishing mortality coefficient. */
  double spr = 1.0;      /*!< Achieved SBPR(F) / SBPR(0). */
  double residual = 0.0; /*!< Achieved SPR minus the requested target. */
  size_t iterations = 0; /*!< Number of bisection steps performed. */
  SPRStatus status = SPRStatus::iteration_limit; /*!< Solver outcome. */
};

/**
 * @brief Find F for an SPR target in (0, 1], using bisection.
 * @details This double-only solver is for post-fit use. A target outside the
 * supplied bounds returns not_bracketed with the upper-bound candidate.
 * Undefined SPR (zero unfished spawning biomass) throws a domain error.
 */
inline SPRResult CalculateSPR(const ReferencePointInputs<double>& inputs,
                              double target,
                              const SPROptions& options = SPROptions()) {
  if (!std::isfinite(target) || target <= 0.0 || target > 1.0 ||
      !std::isfinite(options.max_f) || options.max_f <= 0.0 ||
      !std::isfinite(options.tolerance) || options.tolerance <= 0.0 ||
      options.tolerance >= 1.0 || options.max_iterations == 0) {
    throw std::invalid_argument("Invalid SPR target or solver options");
  }
  const double unfished = CalculatePerRecruit(inputs, 0.0).spawning_biomass;
  if (unfished <= 0.0) {
    throw std::domain_error("SPR requires positive unfished spawning biomass");
  }
  SPRResult result;
  result.residual = 1.0 - target;
  if (std::abs(result.residual) <= options.tolerance) {
    result.status = SPRStatus::converged;
    return result;
  }
  auto evaluate = [&](double f) {
    result.fishing_mortality = f;
    result.spr = CalculatePerRecruit(inputs, f).spawning_biomass / unfished;
    result.residual = result.spr - target;
  };
  evaluate(options.max_f);
  if (std::abs(result.residual) <= options.tolerance) {
    result.status = SPRStatus::converged;
    return result;
  }
  if (result.residual > 0.0) {
    result.status = SPRStatus::not_bracketed;
    return result;
  }
  double lower = 0.0;
  double upper = options.max_f;
  for (size_t i = 0; i < options.max_iterations; ++i) {
    evaluate(lower + (upper - lower) / 2.0);
    result.iterations = i + 1;
    if (std::abs(result.residual) <= options.tolerance) {
      result.status = SPRStatus::converged;
      return result;
    }
    if (result.residual > 0.0) {
      lower = result.fishing_mortality;
    } else {
      upper = result.fishing_mortality;
    }
  }
  return result;
}

}  // namespace fims_popdy
#endif
