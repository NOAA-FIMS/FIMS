#include "gtest/gtest.h"
#include "../../inst/include/population_dynamics/population/subpopulation.hpp"

namespace {

TEST(PartitionSpec, DefaultSexPartitionHasTwoStrata) {
  fims_popdy::PartitionSpec spec = fims_popdy::MakeDefaultSexPartitionSpec();
  EXPECT_EQ(spec.axes.size(), 1);
  EXPECT_EQ(spec.axes[0].name, "sex");
  EXPECT_EQ(spec.axes[0].size(), 2);
  EXPECT_EQ(spec.axes[0].levels, (std::vector<std::string>{"female", "male"}));
  EXPECT_EQ(spec.n_strata(), 2);
}

TEST(PartitionSpec, StratumIdAndLevelsAreInverse) {
  fims_popdy::PartitionSpec spec = fims_popdy::MakeDefaultSexPartitionSpec();

  std::vector<size_t> female_level = {0};
  std::vector<size_t> male_level = {1};
  EXPECT_EQ(spec.stratum_id(female_level), 0);
  EXPECT_EQ(spec.stratum_id(male_level), 1);

  EXPECT_EQ(spec.levels_from_stratum(0), female_level);
  EXPECT_EQ(spec.levels_from_stratum(1), male_level);
}

TEST(PartitionSpec, StratumIdRejectsMismatchedLevelsSize) {
  fims_popdy::PartitionSpec spec = fims_popdy::MakeDefaultSexPartitionSpec();
  EXPECT_THROW(spec.stratum_id({0, 1}), std::invalid_argument);
}

TEST(PartitionSpec, StratumIdRejectsOutOfBoundsLevel) {
  fims_popdy::PartitionSpec spec = fims_popdy::MakeDefaultSexPartitionSpec();
  EXPECT_THROW(spec.stratum_id({2}), std::invalid_argument);
}

TEST(PartitionSpec, LevelsFromStratumRejectsOutOfBoundsStratum) {
  fims_popdy::PartitionSpec spec = fims_popdy::MakeDefaultSexPartitionSpec();
  EXPECT_THROW(spec.levels_from_stratum(2), std::invalid_argument);
}

TEST(PartitionSpec, ExpandGroupToStrataSupportsWildcard) {
  fims_popdy::PartitionSpec spec = fims_popdy::MakeDefaultSexPartitionSpec();
  fims_popdy::GroupSelector all_sexes;
  all_sexes.level = {fims_popdy::GroupSelector::kWildcard};

  EXPECT_EQ(spec.expand_group_to_strata(all_sexes),
            (std::vector<size_t>{0, 1}));
}

TEST(PartitionSpec, ExpandGroupToStrataSelectsSingleStratum) {
  fims_popdy::PartitionSpec spec = fims_popdy::MakeDefaultSexPartitionSpec();
  fims_popdy::GroupSelector female_only;
  female_only.level = {0};

  EXPECT_EQ(spec.expand_group_to_strata(female_only),
            (std::vector<size_t>{0}));
}

TEST(PartitionSpec, ExpandGroupToStrataRejectsMismatchedGroupSize) {
  fims_popdy::PartitionSpec spec = fims_popdy::MakeDefaultSexPartitionSpec();
  fims_popdy::GroupSelector group;
  group.level = {0, 1};
  EXPECT_THROW(spec.expand_group_to_strata(group), std::invalid_argument);
}

TEST(PartitionSpec, ExpandGroupToStrataRejectsOutOfBoundsLevel) {
  fims_popdy::PartitionSpec spec = fims_popdy::MakeDefaultSexPartitionSpec();
  fims_popdy::GroupSelector group;
  group.level = {2};
  EXPECT_THROW(spec.expand_group_to_strata(group), std::invalid_argument);
}

TEST(IndexLayout, FoldedIndicesMatchYearAgeAndStratum) {
  fims_popdy::IndexLayout layout;
  layout.n_strata = 2;
  layout.n_years = 3;
  layout.n_ages = 4;

  EXPECT_EQ(layout.i_age_year(1, 2), 6);
  EXPECT_EQ(layout.i_stratum_age_year(0, 1, 2), 6);
  EXPECT_EQ(layout.i_stratum_age_year(1, 1, 2), 18);
  EXPECT_EQ(layout.n_partitioned_age_year(), 24);
}

}  // namespace
