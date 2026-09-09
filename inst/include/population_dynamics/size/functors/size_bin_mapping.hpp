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
#include <cstddef>
#include <stdexcept>

#include "../size_grid.hpp"
#include "../../../common/fims_vector.hpp"

namespace fims_popdy {

/**
 * @brief Helper for mapping population biological size bins to observed fleet
 * bins.
 *
 * This helper operates only on bin geometry. It does not depend on growth,
 * age-to-length conversion, selectivity, or catch-at-age logic.
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
  static double OverlapWidth(double left_a, double right_a, double left_b,
                             double right_b) {
    return (std::max)(
        0.0, (std::min)(right_a, right_b) - (std::max)(left_a, left_b));
  }

  /**
   * @brief Check that a bin-edge vector is usable for geometric mapping.
   * @param edges Bin edges to validate.
   * @return True when there are at least 2 edges and they are strictly
   * increasing.
   */
  static bool HasStrictlyIncreasingEdges(const fims::Vector<double>& edges) {
    if (edges.size() < 2) {
      return false;
    }

    for (std::size_t i = 0; i + 1 < edges.size(); ++i) {
      if (!(edges[i + 1] > edges[i])) {
        return false;
      }
    }

    return true;
  }

  /**
   * @brief Build overlap-based rebin weights from source bins to
   * destination bins.
   * @param source_edges Population biological size-grid edges.
   * @param destination_edges Fleet observation-bin edges.
   * @return Matrix of weights stored as [destination_bin][source_bin].
   *
   * Convert a probability distribution from FIMS's internal biological size
   * grid into the same bins as observed by a fleet using geometric mass
   * conservation when translating the growth-derived biological size
   * distribution into the bins actually observed by a fleet. It requires
   * destination support to cover source support and does not apply
   * higher-level tail handling such as biological plus-group interpretation or
   * observation-side compression rules.
   */
  static fims::Vector<fims::Vector<double>> BuildRebinWeights(
      const fims::Vector<double>& source_edges,
      const fims::Vector<double>& destination_edges) {
    if (!HasStrictlyIncreasingEdges(source_edges) ||
        !HasStrictlyIncreasingEdges(destination_edges)) {
      throw std::runtime_error(
          "SizeBinMapping requires strictly increasing source and "
          "destination edges");
    }

    if (!DestinationCoversSourceRange(source_edges, destination_edges)) {
      throw std::runtime_error(
          "SizeBinMapping requires destination edges to cover the full "
          "source-bin range");
    }

    const std::size_t n_source_bins = source_edges.size() - 1;
    const std::size_t n_destination_bins = destination_edges.size() - 1;

    fims::Vector<fims::Vector<double>> weights;
    weights.resize(n_destination_bins);

    for (std::size_t i = 0; i < n_destination_bins; ++i) {
      weights[i].resize(n_source_bins);

      for (std::size_t j = 0; j < n_source_bins; ++j) {
        const double source_left = source_edges[j];
        const double source_right = source_edges[j + 1];
        const double destination_left = destination_edges[i];
        const double destination_right = destination_edges[i + 1];
        const double source_width = source_right - source_left;
        const double overlap = OverlapWidth(
            source_left, source_right, destination_left, destination_right);

        weights[i][j] = source_width > 0.0 ? overlap / source_width : 0.0;
      }
    }

    return weights;
  }

  /**
   * @brief Check that the destination edges fully cover the source-bin range.
   * @param source_edges Population biological size-grid edges.
   * @param destination_edges Fleet observation-bin edges.
   * @return True when the destination range contains the full source range.
   */
  static bool DestinationCoversSourceRange(
      const fims::Vector<double>& source_edges,
      const fims::Vector<double>& destination_edges) {
    if (!HasStrictlyIncreasingEdges(source_edges) ||
        !HasStrictlyIncreasingEdges(destination_edges)) {
      return false;
    }

    return destination_edges[0] <= source_edges[0] &&
           destination_edges[destination_edges.size() - 1] >=
               source_edges[source_edges.size() - 1];
  }

  /**
   * @brief Return explicit mapping edges whose support covers the source range
   * while preserving the canonical destination bin breaks.
   * @param source_edges Population biological size-grid edges that must be
   * covered.
   * @param destination_edges Fleet observation-bin edges.
   * @return Copy of destination edges with only the outer edges widened as
   * needed to cover the full source-bin range.
   *
   * This is an explicit higher-level mapping policy. It does not redefine the
   * canonical destination geometry itself; it only prepares a temporary edge
   * vector for geometric translation when destination support is narrower than
   * source support.
   */
  static fims::Vector<double> ExpandDestinationEdgesToCoverSourceRange(
      const fims::Vector<double>& source_edges,
      const fims::Vector<double>& destination_edges) {
    if (!HasStrictlyIncreasingEdges(source_edges) ||
        !HasStrictlyIncreasingEdges(destination_edges)) {
      throw std::runtime_error(
          "SizeBinMapping requires strictly increasing source and "
          "destination edges");
    }

    fims::Vector<double> expanded_edges = destination_edges;

    if (expanded_edges[0] > source_edges[0]) {
      expanded_edges[0] = source_edges[0];
    }

    const std::size_t last_destination_edge = expanded_edges.size() - 1;
    const std::size_t last_source_edge = source_edges.size() - 1;

    if (expanded_edges[last_destination_edge] <
        source_edges[last_source_edge]) {
      expanded_edges[last_destination_edge] = source_edges[last_source_edge];
    }

    return expanded_edges;
  }

  /**
   * @brief Apply a source-to-destination bin mapping to a size-mass vector.
   *
   * The source vector contains probability mass or another size-indexed
   * quantity on the population biological size grid. The weight matrix must
   * have been created by `BuildRebinWeights()`, so each row corresponds to one
   * destination bin and each column corresponds to one source bin. For each
   * destination bin, this function multiplies the source mass in every source
   * bin by its overlap fraction and sums the results. In matrix notation, the
   * returned vector is `destination_mass = weights * source_mass`.
   *
   * In the growth-derived age-to-length path, the source vector is the
   * population-level age-to-size probability row and the destination vector is
   * the fleet-specific age-to-length probability row. The caller may first
   * use `ExpandDestinationEdgesToCoverSourceRange()` to extend the outer
   * destination edges, then use `BuildRebinWeights()` to construct the
   * geometric mapping. This function performs only the weighted aggregation;
   * it does not validate or normalize the resulting row.
   *
   * @param weights Overlap weights stored as
   *   `[destination_bin][source_bin]`, typically returned by
   *   `BuildRebinWeights()`.
   * @param source_mass Mass or probability values indexed by source bin.
   * @return Values aggregated onto the destination bins, in destination-bin
   *   order.
   * @throws std::runtime_error If a weight row has a different number of
   *   columns than `source_mass`.
   * @see BuildRebinWeights()
   * @see ExpandDestinationEdgesToCoverSourceRange()
   * @see
   * AgeToLengthConversionDerived::BuildMappedFleetAgeToLengthConversionRow()
   */
  template <typename Type>
  static fims::Vector<Type> ApplyRebinWeights(
      const fims::Vector<fims::Vector<double>>& weights,
      const fims::Vector<Type>& source_mass) {
    fims::Vector<Type> destination_mass;
    destination_mass.resize(weights.size());

    for (std::size_t i = 0; i < weights.size(); ++i) {
      Type total = Type(0.0);

      if (weights[i].size() != source_mass.size()) {
        throw std::runtime_error(
            "SizeBinMapping weight rows must match source mass length");
      }

      for (std::size_t j = 0; j < source_mass.size(); ++j) {
        total += source_mass[j] * weights[i][j];
      }

      destination_mass[i] = total;
    }

    return destination_mass;
  }
};

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_SIZE_BIN_MAPPING_HPP */
