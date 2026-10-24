/**
 * @file msy.hpp
 * @brief Bounded equilibrium yield maximization with fixed fleet shares.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_REFERENCE_POINTS_MSY_HPP
#define FIMS_POPULATION_DYNAMICS_REFERENCE_POINTS_MSY_HPP

#include "equilibrium.hpp"

namespace fims_popdy {

/** @brief MSY search outcomes; a boundary result is not an interior optimum. */
enum class MSYStatus {
  converged,
  upper_bound,
  no_positive_yield,
  iteration_limit
};

/** @brief Controls for a grid search followed by golden-section refinement. */
struct MSYOptions {
  double max_f = 5.0;      /*!< Search [0, max_f]. */
  double tolerance = 1e-8; /*!< Absolute tolerance on the refined F interval. */
  size_t max_iterations = 100; /*!< Maximum refinement steps. */
  size_t grid_intervals = 100; /*!< Initial evenly spaced search intervals. */
};

/** @brief Best candidate and search diagnostics. */
struct MSYResult {
  double fishing_mortality = 0.0; /*!< F at the best observed yield. */
  EquilibriumResult
      equilibrium;       /*!< Recruitment, biomass, and MSY candidate. */
  size_t iterations = 0; /*!< Golden-section refinement steps. */
  MSYStatus status = MSYStatus::iteration_limit; /*!< Search outcome. */
};

/**
 * @brief Maximize included fleets' equilibrium yield within supplied bounds.
 * @details Every fleet still contributes mortality and catch. At least one
 * fleet must be included in the objective; otherwise MSY is undefined.
 * A grid locates candidate local maxima, then golden-section search refines
 * each neighboring interval. Both endpoints remain candidates. This
 * numerical search does not guarantee a global maximum for arbitrary fishing
 * patterns; inspect sensitivity to bounds and grid resolution when needed.
 */
inline MSYResult CalculateMSY(const ReferencePointInputs<double>& inputs,
                              const ReferencePointRecruitment& recruitment,
                              const MSYOptions& options = MSYOptions()) {
  if (!std::isfinite(options.max_f) || options.max_f <= 0.0 ||
      !std::isfinite(options.tolerance) || options.tolerance <= 0.0 ||
      options.max_iterations == 0 || options.grid_intervals < 2) {
    throw std::invalid_argument("Invalid MSY solver options");
  }
  MSYResult result;
  result.equilibrium = CalculateEquilibrium(inputs, 0.0, recruitment);
  bool has_included_fleet = false;
  for (const auto& fleet : inputs.fleets) {
    has_included_fleet = has_included_fleet || fleet.include_in_msy;
  }
  if (!has_included_fleet) {
    throw std::invalid_argument("MSY requires at least one included fleet");
  }

  auto evaluate = [&](double f) {
    const auto equilibrium = CalculateEquilibrium(inputs, f, recruitment);
    if (equilibrium.objective_yield > result.equilibrium.objective_yield) {
      result.fishing_mortality = f;
      result.equilibrium = equilibrium;
    }
    return equilibrium.objective_yield;
  };
  std::vector<double> grid_yield(options.grid_intervals + 1);
  grid_yield[0] = result.equilibrium.objective_yield;
  for (size_t i = 1; i <= options.grid_intervals; ++i) {
    const double f = options.max_f * (static_cast<double>(i) /
                                       options.grid_intervals);
    grid_yield[i] = evaluate(f);
  }
  // A coarse grid can step past a narrow productive region into collapse.
  // Search below the first grid point before declaring no positive yield.
  double first_positive_f = options.max_f / options.grid_intervals;
  bool found_below_grid = false;
  if (result.equilibrium.objective_yield <= 0.0 &&
      !result.equilibrium.collapsed) {
    while (first_positive_f > options.tolerance) {
      first_positive_f *= 0.5;
      if (evaluate(first_positive_f) > 0.0) {
        found_below_grid = true;
        break;
      }
    }
  }
  if (result.equilibrium.objective_yield <= 0.0) {
    result.status = MSYStatus::no_positive_yield;
    return result;
  }

  const double ratio = (std::sqrt(5.0) - 1.0) / 2.0;
  bool hit_iteration_limit = false;
  auto refine = [&](double lower, double upper) {
    double left = upper - ratio * (upper - lower);
    double right = lower + ratio * (upper - lower);
    double left_yield = evaluate(left);
    double right_yield = evaluate(right);
    size_t iterations = 0;
    while (upper - lower > options.tolerance &&
           iterations < options.max_iterations) {
      if (left_yield < right_yield) {
        lower = left;
        left = right;
        left_yield = right_yield;
        right = lower + ratio * (upper - lower);
        right_yield = evaluate(right);
      } else {
        upper = right;
        right = left;
        right_yield = left_yield;
        left = upper - ratio * (upper - lower);
        left_yield = evaluate(left);
      }
      ++iterations;
    }
    result.iterations += iterations;
    hit_iteration_limit = hit_iteration_limit ||
                          upper - lower > options.tolerance;
  };

  for (size_t i = 1; i < options.grid_intervals; ++i) {
    if (grid_yield[i] > grid_yield[i - 1] &&
        grid_yield[i] > grid_yield[i + 1]) {
      const double lower = options.max_f *
                           (static_cast<double>(i - 1) / options.grid_intervals);
      const double upper = options.max_f *
                           (static_cast<double>(i + 1) / options.grid_intervals);
      refine(lower, upper);
    }
  }
  if (found_below_grid) {
    refine(0.0, (std::min)(options.max_f, 2.0 * first_positive_f));
  }

  if (result.fishing_mortality == options.max_f) {
    result.status = MSYStatus::upper_bound;
  } else if (hit_iteration_limit) {
    result.status = MSYStatus::iteration_limit;
  } else {
    result.status = MSYStatus::converged;
  }
  return result;
}

}  // namespace fims_popdy
#endif
