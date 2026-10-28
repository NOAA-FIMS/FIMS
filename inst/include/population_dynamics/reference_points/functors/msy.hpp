/**
 * @file msy.hpp
 * @brief Bounded equilibrium yield maximization with fixed fleet shares.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_REFERENCE_POINTS_MSY_HPP
#define FIMS_POPULATION_DYNAMICS_REFERENCE_POINTS_MSY_HPP

#include <algorithm>
#include <utility>

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
  size_t max_iterations =
      100; /*!< Maximum refinement steps per candidate interval. */
  size_t grid_intervals = 100; /*!< Initial evenly spaced search intervals. */
};

/** @brief Best candidate and search diagnostics. */
struct MSYResult {
  double fishing_mortality = 0.0; /*!< F at the best observed yield. */
  EquilibriumResult
      equilibrium;       /*!< Recruitment, biomass, and MSY candidate. */
  size_t iterations = 0; /*!< Total golden-section refinement steps. */
  size_t intervals_refined = 0; /*!< Number of candidate intervals searched. */
  double bracket_width = 0.0;   /*!< Largest final refinement interval width. */
  MSYStatus status = MSYStatus::iteration_limit; /*!< Search outcome. */
};

/**
 * @brief Maximize included fleets' equilibrium yield within supplied bounds.
 * @details Every fleet still contributes mortality and catch. At least one
 * fleet must be included in the objective; otherwise MSY is undefined.
 * A grid locates candidate peaks (including plateaus), then golden-section
 * search refines each neighborhood and both boundary-adjacent intervals.
 * Both endpoints remain candidates. This
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
  std::vector<double> grid(options.grid_intervals + 1, 0.0);
  auto grid_f = [&](size_t i) {
    return options.max_f * (static_cast<double>(i) / options.grid_intervals);
  };
  for (size_t i = 1; i <= options.grid_intervals; ++i) {
    grid[i] = evaluate(grid_f(i));
  }
  // A coarse grid can step past a narrow productive region into collapse.
  double first_positive_f = grid_f(1);
  bool narrow_region = false;
  if (result.equilibrium.objective_yield <= 0.0 &&
      !result.equilibrium.collapsed) {
    while (first_positive_f > options.tolerance) {
      first_positive_f *= 0.5;
      if (evaluate(first_positive_f) > 0.0) {
        narrow_region = true;
        break;
      }
    }
  }
  if (result.equilibrium.objective_yield <= 0.0) {
    result.status = MSYStatus::no_positive_yield;
    return result;
  }
  std::vector<std::pair<double, double>> intervals;
  intervals.emplace_back(0.0, grid_f(1));
  intervals.emplace_back(grid_f(options.grid_intervals - 1), options.max_f);
  if (narrow_region) intervals.emplace_back(0.0, 2.0 * first_positive_f);
  // Group equal-height samples so a flat-topped peak is not discarded.
  for (size_t i = 1; i < options.grid_intervals;) {
    size_t end = i;
    while (end < options.grid_intervals && grid[end + 1] == grid[i]) ++end;
    if (grid[i] > 0.0 && grid[i] >= grid[i - 1] &&
        (end == options.grid_intervals || grid[end] >= grid[end + 1])) {
      intervals.emplace_back(
          grid_f(i - 1), grid_f((std::min)(end + 1, options.grid_intervals)));
    }
    i = end + 1;
  }
  bool all_refined = true;
  const double ratio = (std::sqrt(5.0) - 1.0) / 2.0;
  for (const auto& interval : intervals) {
    double lower = interval.first;
    double upper = interval.second;
    double left = upper - ratio * (upper - lower);
    double right = lower + ratio * (upper - lower);
    double left_yield = evaluate(left);
    double right_yield = evaluate(right);
    size_t steps = 0;
    while (upper - lower > options.tolerance &&
           steps < options.max_iterations) {
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
      ++steps;
    }
    result.iterations += steps;
    ++result.intervals_refined;
    result.bracket_width = (std::max)(result.bracket_width, upper - lower);
    all_refined = all_refined && upper - lower <= options.tolerance;
  }
  if (all_refined) {
    result.status = result.fishing_mortality == options.max_f
                        ? MSYStatus::upper_bound
                        : MSYStatus::converged;
  }
  return result;
}

}  // namespace fims_popdy
#endif
