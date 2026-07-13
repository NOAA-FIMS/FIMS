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
    edges.emplace_back(lower_edge);

    const double span = upper_edge - lower_edge;
    const double tolerance = 1e-12 * (span > 1.0 ? span : 1.0);

    std::size_t bin_index = 1;
    while (true) {
      const double next_edge = lower_edge + bin_index * bin_width;

      if (next_edge >= upper_edge - tolerance) {
        break;
      }

      edges.emplace_back(next_edge);
      ++bin_index;
    }

    edges.emplace_back(upper_edge);

    return BuildFromEdges(edges);
  }

  /**
   * @brief Build one fleet observation-bin edge vector from stored bin centers.
   *
   * For multiple centers, edges are inferred from adjacent midpoints. A single
   * center is not enough information to infer observation-bin geometry
   * consistently, so this helper requires at least 2 strictly increasing
   * centers.
   *
   * @param centers Fleet observation-bin centers.
   * @return Strictly increasing observation-bin edges inferred from the centers.
   */
  static fims::Vector<double> BuildObservationEdgesFromCenters(
      const fims::Vector<double>& centers) {
    if (centers.size() < 2) {
      throw std::runtime_error(
          "SizeGridBuilder requires at least 2 observation-bin centers to build fleet edges");
    }

    for (std::size_t center_index = 1; center_index < centers.size();
         ++center_index) {
      if (!(centers[center_index] > centers[center_index - 1])) {
        throw std::runtime_error(
            "SizeGridBuilder requires observation-bin centers to be strictly increasing");
      }
    }

    fims::Vector<double> edges;
    edges.resize(centers.size() + 1);

    edges[0] = centers[0] - 0.5 * (centers[1] - centers[0]);

    for (std::size_t center_index = 0;
         center_index + 1 < centers.size();
         ++center_index) {
      edges[center_index + 1] =
          0.5 * (centers[center_index] + centers[center_index + 1]);
    }

    edges[centers.size()] =
        centers[centers.size() - 1] +
        0.5 * (centers[centers.size() - 1] - centers[centers.size() - 2]);

    return edges;
  }

  /**
   * @brief Build the default population biological size grid from resolved
   * fleet observation-bin edges.
   *
   * The default rule uses the minimum fleet lower edge and maximum fleet upper
   * edge across the supplied fleet edge vectors to define the supported range.
   * It then builds fixed-width regular biological bins with built-in width 1.0
   * in the same size units used by the user-supplied data until that
   * regular-bin sequence covers the supported range. The built-in width is one
   * size-data unit, but the resulting biological-bin edges are anchored to the
   * minimum resolved fleet lower edge and therefore are not required to fall on
   * integer-valued boundaries. Finally, it appends one additional terminal bin
   * above the last regular bin so the final biological bin can serve as the
   * plus-group representation.
   *
   * @param fleet_edges Resolved fleet observation-bin edge vectors.
   * @return Internally consistent default population biological size grid.
   */
  static SizeGrid BuildDefaultFromFleetEdges(
      const fims::Vector<fims::Vector<double>>& fleet_edges) {
    if (fleet_edges.size() == 0) {
      throw std::runtime_error(
          "SizeGridBuilder requires at least 1 fleet edge vector to build the default population size grid");
    }

    double min_lower_edge = 0.0;
    double max_upper_edge = 0.0;
    bool first_fleet = true;

    for (std::size_t fleet_index = 0; fleet_index < fleet_edges.size();
         ++fleet_index) {
      const fims::Vector<double>& edges = fleet_edges[fleet_index];

      if (edges.size() < 2) {
        throw std::runtime_error(
            "SizeGridBuilder requires each fleet edge vector to contain at least 2 edges");
      }

      for (std::size_t edge_index = 1; edge_index < edges.size();
           ++edge_index) {
        if (!(edges[edge_index] > edges[edge_index - 1])) {
          throw std::runtime_error(
              "SizeGridBuilder requires fleet edge vectors to be strictly increasing");
        }
      }

      const double fleet_lower_edge = edges[0];
      const double fleet_upper_edge = edges[edges.size() - 1];

      if (first_fleet) {
        min_lower_edge = fleet_lower_edge;
        max_upper_edge = fleet_upper_edge;
        first_fleet = false;
      } else {
        if (fleet_lower_edge < min_lower_edge) {
          min_lower_edge = fleet_lower_edge;
        }

        if (fleet_upper_edge > max_upper_edge) {
          max_upper_edge = fleet_upper_edge;
        }
      }
    }

    const double built_in_bin_width = 1.0;
    double regular_upper_edge = min_lower_edge;
    const double span = max_upper_edge - min_lower_edge;
    const double tolerance = 1e-12 * (span > 1.0 ? span : 1.0);

    while (regular_upper_edge < max_upper_edge - tolerance) {
      regular_upper_edge += built_in_bin_width;
    }

    SizeGrid regular_grid =
        BuildRegularGrid(min_lower_edge, regular_upper_edge, built_in_bin_width);

    fims::Vector<double> default_edges = regular_grid.edges;
    default_edges.emplace_back(regular_upper_edge + built_in_bin_width);

    return BuildFromEdges(default_edges);
  }
};

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_SIZE_GRID_BUILDER_HPP */
