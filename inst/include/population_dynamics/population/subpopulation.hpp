/**
 * @file subpopulation.hpp
 * @brief Partition structure and indexing for population subpopulations.
 * @copyright This file is part of the NOAA, National Marine Fisheries Service
 * Fisheries Integrated Modeling System project. See LICENSE in the source
 * folder for reuse information.
 */
#ifndef FIMS_POPULATION_DYNAMICS_POPULATION_SUBPOPULATION_HPP
#define FIMS_POPULATION_DYNAMICS_POPULATION_SUBPOPULATION_HPP

#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace fims_popdy {

/**
 * @brief One partition axis (e.g. sex with levels female and male).
 */
struct Axis {
  std::string name;                /*!< axis name (e.g. sex) */
  std::vector<std::string> levels; /*!< level labels for this axis */

  /** @brief Number of levels on this axis. */
  size_t size() const { return levels.size(); }
};

/**
 * @brief Selects one level per axis, or kWildcard for all levels on that axis.
 *
 * @details A group is a partial specification (e.g. female only, all sexes).
 * Use PartitionSpec::expand_group_to_strata() to resolve a group to concrete
 * stratum indices.
 */
struct GroupSelector {
  static constexpr int kWildcard = -1; /*!< all levels on an axis */
  std::vector<int> level;              /*!< level index per axis, or kWildcard */
};

/**
 * @brief Collection of axes defining strata for a partitioned population.
 */
struct PartitionSpec {
  std::vector<Axis> axes; /*!< partition axes (e.g. sex) */

  /**
   * @brief Total number of strata across all axes.
   */
  size_t n_strata() const {
    if (axes.empty()) {
      return 1;
    }
    size_t n = 1;
    for (const Axis &axis : axes) {
      n *= axis.size();
    }
    return n;
  }

  /**
   * @brief Encode one level per axis as a flat stratum index.
   *
   * @details The last axis in axes varies fastest.
   */
  size_t stratum_id(const std::vector<size_t> &levels) const {
    if (levels.size() != axes.size()) {
      throw std::invalid_argument(
          "PartitionSpec::stratum_id: levels vector size " +
          std::to_string(levels.size()) +
          " does not match number of axes " + std::to_string(axes.size()));
    }
    size_t id = 0;
    size_t multiplier = 1;
    for (int i = static_cast<int>(axes.size()) - 1; i >= 0; --i) {
      const size_t axis_size = axes[i].size();
      if (levels[i] >= axis_size) {
        throw std::invalid_argument(
            "PartitionSpec::stratum_id: level index " +
            std::to_string(levels[i]) + " out of bounds for axis " +
            std::to_string(i) + " (size " + std::to_string(axis_size) + ")");
      }
      id += levels[i] * multiplier;
      multiplier *= axis_size;
    }
    return id;
  }

  /**
   * @brief Decode a flat stratum index to one level per axis.
   */
  std::vector<size_t> levels_from_stratum(size_t stratum) const {
    if (stratum >= n_strata()) {
      throw std::invalid_argument(
          "PartitionSpec::levels_from_stratum: stratum " +
          std::to_string(stratum) + " out of bounds (n_strata = " +
          std::to_string(n_strata()) + ")");
    }
    std::vector<size_t> levels(axes.size());
    for (int i = static_cast<int>(axes.size()) - 1; i >= 0; --i) {
      levels[i] = stratum % axes[i].size();
      stratum /= axes[i].size();
    }
    return levels;
  }

  /**
   * @brief Expand a group selector to the matching stratum indices.
   */
  std::vector<size_t> expand_group_to_strata(const GroupSelector &group) const {
    std::vector<size_t> strata;
    if (axes.empty()) {
      strata.push_back(0);
      return strata;
    }

    std::vector<size_t> current_levels(axes.size(), 0);
    expand_group_to_strata_recursive(group, 0, &current_levels, &strata);
    return strata;
  }

 private:
  void expand_group_to_strata_recursive(const GroupSelector &group,
                                        size_t axis_index,
                                        std::vector<size_t> *current_levels,
                                        std::vector<size_t> *strata) const {
    if (axis_index == axes.size()) {
      strata->push_back(stratum_id(*current_levels));
      return;
    }

    if (group.level[axis_index] == GroupSelector::kWildcard) {
      for (size_t level = 0; level < axes[axis_index].size(); ++level) {
        (*current_levels)[axis_index] = level;
        expand_group_to_strata_recursive(group, axis_index + 1, current_levels,
                                         strata);
      }
    } else {
      (*current_levels)[axis_index] =
          static_cast<size_t>(group.level[axis_index]);
      expand_group_to_strata_recursive(group, axis_index + 1, current_levels,
                                       strata);
    }
  }
};

/**
 * @brief Folded indices for pooled and partitioned derived quantities.
 *
 * @details Pooled quantities use i_age_year(). Partitioned quantities add
 * stratum as the leading dimension via i_stratum_age_year().
 */
struct IndexLayout {
  size_t n_strata = 1; /*!< number of partition strata */
  size_t n_years = 0;  /*!< number of years */
  size_t n_ages = 0;   /*!< number of ages */

  /** @brief Folded index for pooled (year, age) derived quantities. */
  size_t i_age_year(size_t year, size_t age) const {
    return year * n_ages + age;
  }

  /** @brief Folded index for partitioned (stratum, year, age) quantities. */
  size_t i_stratum_age_year(size_t stratum, size_t year, size_t age) const {
    return stratum * (n_years * n_ages) + i_age_year(year, age);
  }

  /** @brief Length of a partitioned (stratum, year, age) vector. */
  size_t n_partitioned_age_year() const {
    return n_strata * n_years * n_ages;
  }
};

/**
 * @brief Default sex partition used when initializing populations.
 */
inline PartitionSpec MakeDefaultSexPartitionSpec() {
  PartitionSpec spec;
  Axis sex_axis;
  sex_axis.name = "sex";
  sex_axis.levels = {"female", "male"};
  spec.axes.push_back(std::move(sex_axis));
  return spec;
}

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_POPULATION_SUBPOPULATION_HPP */
