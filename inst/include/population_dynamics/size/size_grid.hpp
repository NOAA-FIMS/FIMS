/**
 * @file size_grid.hpp
 * @brief Defines the SizeGrid container for population-level biological size
 * support.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_SIZE_GRID_HPP
#define FIMS_POPULATION_DYNAMICS_SIZE_GRID_HPP

#include <cstddef>

#include "../../common/fims_vector.hpp"

namespace fims_popdy {

/**
 * @brief Population-level biological size grid.
 *
 * This container defines the internal biological size support used by a
 * population. It is intentionally separate from fleet observation bins so
 * population size structure can be represented independently of how fleets
 * observe it.
 */
struct SizeGrid {
  std::size_t n_bins = 0;        /*!< number of biological size bins */
  fims::Vector<double> edges;    /*!< size-bin edges on the natural scale */
  fims::Vector<double> centers;  /*!< size-bin centers on the natural scale */

  /**
   * @brief Return whether the bin edges are strictly increasing.
   * @return True when each edge is greater than the previous edge.
   */
  bool HasStrictlyIncreasingEdges() const {
    if (n_bins == 0) {
      return edges.size() == 0;
    }

    if (edges.size() != n_bins + 1) {
      return false;
    }

    for (std::size_t i = 1; i < edges.size(); ++i) {
      if (!(edges[i] > edges[i - 1])) {
        return false;
      }
    }

    return true;
  }

  /**
   * @brief Return whether each center falls inside its matching bin.
   * @return True when every center is between its lower and upper bin edge.
   */
  bool CentersFallWithinBins() const {
    if (n_bins == 0) {
      return centers.size() == 0;
    }

    if (centers.size() != n_bins || edges.size() != n_bins + 1) {
      return false;
    }

    for (std::size_t i = 0; i < n_bins; ++i) {
      if (!(centers[i] > edges[i] && centers[i] < edges[i + 1])) {
        return false;
      }
    }

    return true;
  }

  /**
   * @brief Return whether the grid dimensions and geometry are internally consistent.
   * @return True when the bin counts match and the grid geometry is valid.
   */
  bool IsConsistent() const {
    if (n_bins == 0) {
      return edges.size() == 0 && centers.size() == 0;
    }

    return HasStrictlyIncreasingEdges() && CentersFallWithinBins();
  }
};

}  // namespace fims_popdy
#endif /* FIMS_POPULATION_DYNAMICS_SIZE_GRID_HPP */
