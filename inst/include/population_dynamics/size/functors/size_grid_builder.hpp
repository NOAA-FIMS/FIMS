/**
 * @file size_grid_builder.hpp
 * @brief Defines helpers for constructing population-level biological size grids.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_SIZE_GRID_BUILDER_HPP
#define FIMS_POPULATION_DYNAMICS_SIZE_GRID_BUILDER_HPP

#include <stdexcept>

#include "../size_grid.hpp"

namespace fims_popdy {

/**
 * @brief Helper for constructing internally consistent population size grids.
 *
 * This builder centralizes grid creation so bin edges, centers, and dimensions
 * are created once in a consistent way.
 */
struct SizeGridBuilder {
  /**
   * @brief Build a size grid from an explicit vector of bin edges.
   * @param edges Biological size-bin edges on the natural scale.
   * @return Internally consistent population size grid.
   */
  static SizeGrid BuildFromEdges(const fims::Vector<double>& edges) {
    if (edges.size() < 2) {
      throw std::runtime_error(
          "SizeGridBuilder requires at least 2 edges to define a size grid");
    }

    SizeGrid grid;
    grid.n_bins = edges.size() - 1;
    grid.edges = edges;
    grid.centers.resize(grid.n_bins);

    for (std::size_t i = 0; i < grid.n_bins; ++i) {
      grid.centers[i] = 0.5 * (grid.edges[i] + grid.edges[i + 1]);
    }

    if (!grid.IsConsistent()) {
      throw std::runtime_error(
          "SizeGridBuilder produced an inconsistent population size grid");
    }

    return grid;
  }

  /**
   * @brief Build a regular size grid from lower edge, upper edge, and bin width.
   * @param lower_edge Lower edge of the first biological size bin.
   * @param upper_edge Upper edge of the final biological size bin.
   * @param bin_width Width of each biological size bin.
   * @return Internally consistent population size grid.
   */
  static SizeGrid BuildRegularGrid(double lower_edge,
                                   double upper_edge,
                                   double bin_width) {
    if (!(upper_edge > lower_edge)) {
      throw std::runtime_error(
          "SizeGridBuilder requires upper_edge > lower_edge");
    }

    if (!(bin_width > 0.0)) {
      throw std::runtime_error(
          "SizeGridBuilder requires bin_width > 0");
    }

    fims::Vector<double> edges;
    double edge = lower_edge;
    edges.emplace_back(edge);

    while (edge + bin_width < upper_edge) {
      edge += bin_width;
      edges.emplace_back(edge);
    }

    edges.emplace_back(upper_edge);

    return BuildFromEdges(edges);
  }
};

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_SIZE_GRID_BUILDER_HPP */