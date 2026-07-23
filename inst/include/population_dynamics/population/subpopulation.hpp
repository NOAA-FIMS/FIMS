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
 * @brief Named level selection for one partition axis.
 *
 * @details Corresponds to one entry in a user-facing list such as
 * list(sex = "female") or list(sex = c("female", "male")). Level names must
 * match Axis::levels. A single "*" means all levels on that axis (wildcard).
 */
struct AxisLevelSelection {
  std::string axis_name;                 /*!< axis name (e.g. "sex") */
  std::vector<std::string> level_names; /*!< requested level labels */
};

/**
 * @brief User demand for which partition strata to materialize in output.
 *
 * @details Empty selections means pooled output only (no partitioned write) —
 * the backward-compatible default. Non-empty selections are a named list of
 * axis filters, e.g. {sex → female}. Omitted axes are treated as wildcards
 * when resolving to a GroupSelector. This shape extends to multi-axis demand
 * later (e.g. sex + area) without changing the internal GroupSelector path.
 */
struct PartitionDemand {
  std::vector<AxisLevelSelection> selections; /*!< empty => pooled */

  /** @brief True when no partitioned strata are requested. */
  bool is_pooled() const { return selections.empty(); }
};

/**
 * @brief Default pooled demand (no partitioned output).
 */
inline PartitionDemand MakePooledPartitionDemand() { return PartitionDemand{}; }

/**
 * @brief Convenience builder for sex-only demand (Model 1).
 *
 * @param level_names Level labels on the sex axis, e.g. {"female"} or
 *        {"female", "male"}.
 */
inline PartitionDemand MakeSexPartitionDemand(
    const std::vector<std::string> &level_names) {
  PartitionDemand demand;
  AxisLevelSelection selection;
  selection.axis_name = "sex";
  selection.level_names = level_names;
  demand.selections.push_back(std::move(selection));
  return demand;
}

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
   * @brief Look up a precomputed split factor for a stratum.
   *
   * @details Split factors are indexed by flat stratum id (length n_strata()).
   * Each entry gives the fraction of a pooled quantity allocated to that
   * stratum when splitting on write. How factors are computed is separate
   * from this lookup. For the default sex-only partition, see
   * SexStratumSplitFactors(); for multi-axis specs or axis interactions,
   * callers supply a user-defined vector of length n_strata().
   *
   * @param stratum Flat stratum index.
   * @param split_factors One entry per stratum; size must equal n_strata().
   */
  template <typename Type>
  Type stratum_split_factor(size_t stratum,
                            const std::vector<Type> &split_factors) const {
    if (split_factors.size() != n_strata()) {
      throw std::invalid_argument(
          "PartitionSpec::stratum_split_factor: split_factors size " +
          std::to_string(split_factors.size()) +
          " does not match n_strata " + std::to_string(n_strata()));
    }
    if (stratum >= n_strata()) {
      throw std::invalid_argument(
          "PartitionSpec::stratum_split_factor: stratum " +
          std::to_string(stratum) + " out of bounds (n_strata = " +
          std::to_string(n_strata()) + ")");
    }
    return split_factors[stratum];
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

    if (group.level.size() != axes.size()) {
      throw std::invalid_argument(
          "PartitionSpec::expand_group_to_strata: group.level size " +
          std::to_string(group.level.size()) +
          " does not match number of axes " + std::to_string(axes.size()));
    }
    for (size_t i = 0; i < axes.size(); ++i) {
      if (group.level[i] == GroupSelector::kWildcard) {
        continue;
      }
      if (group.level[i] < 0 ||
          static_cast<size_t>(group.level[i]) >= axes[i].size()) {
        throw std::invalid_argument(
            "PartitionSpec::expand_group_to_strata: level index " +
            std::to_string(group.level[i]) + " out of bounds for axis " +
            std::to_string(i) + " (size " + std::to_string(axes[i].size()) +
            ")");
      }
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

namespace detail {

inline bool is_sex_only_partition(const PartitionSpec &spec) {
  return spec.axes.size() == 1 && spec.axes[0].name == "sex" &&
         spec.axes[0].size() == 2 && spec.n_strata() == 2;
}

inline int find_axis_index(const PartitionSpec &spec,
                           const std::string &axis_name) {
  for (size_t i = 0; i < spec.axes.size(); ++i) {
    if (spec.axes[i].name == axis_name) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

inline int find_level_index(const Axis &axis, const std::string &level_name) {
  for (size_t i = 0; i < axis.levels.size(); ++i) {
    if (axis.levels[i] == level_name) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

inline bool selection_covers_all_levels(
    const Axis &axis, const std::vector<std::string> &level_names) {
  if (level_names.size() != axis.size()) {
    return false;
  }
  std::vector<bool> seen(axis.size(), false);
  for (const std::string &name : level_names) {
    const int index = find_level_index(axis, name);
    if (index < 0 || seen[static_cast<size_t>(index)]) {
      return false;
    }
    seen[static_cast<size_t>(index)] = true;
  }
  return true;
}

}  // namespace detail

/**
 * @brief Resolve a named PartitionDemand into a GroupSelector.
 *
 * @details Empty (pooled) demand has no GroupSelector — call is_pooled()
 * first. Omitted axes become wildcards. A single level name pins that level.
 * Selecting all levels on an axis (or "*") becomes a wildcard. Partial
 * multi-level subsets on one axis are not supported yet (GroupSelector only
 * stores one fixed level or wildcard per axis).
 */
inline GroupSelector MakeGroupSelectorFromDemand(
    const PartitionSpec &spec, const PartitionDemand &demand) {
  if (demand.is_pooled()) {
    throw std::invalid_argument(
        "MakeGroupSelectorFromDemand: demand is pooled (empty selections)");
  }
  if (spec.axes.empty()) {
    throw std::invalid_argument(
        "MakeGroupSelectorFromDemand: partition spec has no axes");
  }

  for (const AxisLevelSelection &selection : demand.selections) {
    if (detail::find_axis_index(spec, selection.axis_name) < 0) {
      throw std::invalid_argument(
          "MakeGroupSelectorFromDemand: unknown axis \"" +
          selection.axis_name + "\"");
    }
  }

  GroupSelector group;
  group.level.assign(spec.axes.size(), GroupSelector::kWildcard);

  for (size_t axis_index = 0; axis_index < spec.axes.size(); ++axis_index) {
    const Axis &axis = spec.axes[axis_index];
    const AxisLevelSelection *selection = nullptr;
    for (const AxisLevelSelection &candidate : demand.selections) {
      if (candidate.axis_name == axis.name) {
        selection = &candidate;
        break;
      }
    }
    if (selection == nullptr) {
      continue;  // omitted axis => wildcard
    }
    if (selection->level_names.empty()) {
      throw std::invalid_argument(
          "MakeGroupSelectorFromDemand: level_names empty for axis \"" +
          axis.name + "\"");
    }
    if (selection->level_names.size() == 1 &&
        selection->level_names[0] == "*") {
      group.level[axis_index] = GroupSelector::kWildcard;
      continue;
    }
    if (detail::selection_covers_all_levels(axis, selection->level_names)) {
      group.level[axis_index] = GroupSelector::kWildcard;
      continue;
    }
    if (selection->level_names.size() != 1) {
      throw std::invalid_argument(
          "MakeGroupSelectorFromDemand: partial multi-level subsets are not "
          "supported yet for axis \"" +
          axis.name + "\"");
    }
    const int level_index =
        detail::find_level_index(axis, selection->level_names[0]);
    if (level_index < 0) {
      throw std::invalid_argument(
          "MakeGroupSelectorFromDemand: unknown level \"" +
          selection->level_names[0] + "\" for axis \"" + axis.name + "\"");
    }
    group.level[axis_index] = level_index;
  }
  return group;
}

/**
 * @brief Stratum indices requested by a PartitionDemand.
 *
 * @details Pooled demand returns an empty vector. Otherwise resolves demand to
 * a GroupSelector and expands it via PartitionSpec::expand_group_to_strata().
 */
inline std::vector<size_t> RequestedStrata(const PartitionSpec &spec,
                                           const PartitionDemand &demand) {
  if (demand.is_pooled()) {
    return {};
  }
  return spec.expand_group_to_strata(MakeGroupSelectorFromDemand(spec, demand));
}

/**
 * @brief Build stratum split factors for the default sex-only partition.
 *
 * @details Companion to MakeDefaultSexPartitionSpec(). Only valid when spec
 * has a single sex axis with levels female and male (n_strata = 2). Female
 * stratum (0) receives proportion_female; male stratum (1) receives
 * (1 - proportion_female). For any other partition (e.g. sex x area), callers
 * must supply user-defined per-stratum factors to stratum_split_factor().
 *
 * @param spec Partition structure (sex-only; see MakeDefaultSexPartitionSpec()).
 * @param proportion_female Value from Population::proportion_female at the
 *        relevant age (and year when time-varying).
 */
template <typename Type>
std::vector<Type> SexStratumSplitFactors(const PartitionSpec &spec,
                                         Type proportion_female) {
  if (!detail::is_sex_only_partition(spec)) {
    throw std::invalid_argument(
        "SexStratumSplitFactors: requires a sex-only partition spec "
        "(one sex axis with female and male)");
  }
  return {proportion_female, static_cast<Type>(1.0) - proportion_female};
}

}  // namespace fims_popdy

#endif /* FIMS_POPULATION_DYNAMICS_POPULATION_SUBPOPULATION_HPP */
