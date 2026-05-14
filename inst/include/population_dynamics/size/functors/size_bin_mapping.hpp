/**
 * @file size_bin_mapping.hpp
 * @brief Defines helpers for mapping population biological size bins to fleet
 * observation bins.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_SIZE_BIN_MAPPING_HPP
#define FIMS_POPULATION_DYNAMICS_SIZE_BIN_MAPPING_HPP

#include <algorithm>

#include "../size_grid.hpp"
#include "../../../common/fims_vector.hpp"

namespace fims_popdy {

/**
 * @brief Helper for mapping population biological size bins to observed fleet
 * bins.
 *
 * This helper operates only on bin geometry. It does not depend on growth,
 * ALK, selectivity, or catch-at-age logic.
 */
struct SizeBinMapping {
  /**
   * @brief Return the overlap width between two 1-D closed-open intervals.
   * @param left_a Left edge of interval A.
   * @param right_a Right edge of interval A.
   * @param left_b Left edge of interval B.
   * @param right_b Right edge of interval B.
   * @return Non-negative overlap width.
   */
  static double OverlapWidth(double left_a,
                             double right_a,
                             double left_b,
                             double right_b) {
    return std::max(
        0.0, std::min(right_a, right_b) - std::max(left_a, left_b));
  }

  /**
   * @brief Map one population size bin onto all fleet observation bins.
   * @param population_grid Population-level biological size grid.
   * @param population_bin Index of the biological size bin.
   * @param fleet_edges Fleet observation-bin edges.
   * @return Vector of overlap proportions by fleet bin.
   */
  static fims::Vector<double> MapPopulationBinToFleetBins(
      const SizeGrid& population_grid,
      std::size_t population_bin,
      const fims::Vector<double>& fleet_edges) {
    fims::Vector<double> proportions;

    if (!population_grid.IsConsistent() || fleet_edges.size() < 2 ||
        population_bin >= population_grid.n_bins) {
      return proportions;
    }

    const double left = population_grid.edges[population_bin];
    const double right = population_grid.edges[population_bin + 1];
    const double width = right - left;

    proportions.resize(fleet_edges.size() - 1);

    if (width <= 0.0) {
      return proportions;
    }

    for (std::size_t i = 0; i + 1 < fleet_edges.size(); ++i) {
      const double overlap =
          OverlapWidth(left, right, fleet_edges[i], fleet_edges[i + 1]);
      proportions[i] = overlap / width;
    }

    return proportions;
  }
};

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_SIZE_BIN_MAPPING_HPP */
