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
   * @brief Return whether the grid dimensions are internally consistent.
   * @return True when the number of edges and centers matches the bin count.
   */
  bool IsConsistent() const {
    return edges.size() == n_bins + 1 && centers.size() == n_bins;
  }
};

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_SIZE_GRID_HPP */
