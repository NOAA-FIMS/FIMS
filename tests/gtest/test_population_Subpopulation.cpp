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

TEST(PartitionSpec, StratumSplitFactorLooksUpPrecomputedFactors) {
  fims_popdy::PartitionSpec spec = fims_popdy::MakeDefaultSexPartitionSpec();
  const std::vector<double> split_factors = {0.3, 0.7};

  EXPECT_DOUBLE_EQ(spec.stratum_split_factor(0, split_factors), 0.3);
  EXPECT_DOUBLE_EQ(spec.stratum_split_factor(1, split_factors), 0.7);
}

TEST(PartitionSpec, StratumSplitFactorRejectsMismatchedFactorSize) {
  fims_popdy::PartitionSpec spec = fims_popdy::MakeDefaultSexPartitionSpec();
  const std::vector<double> split_factors = {0.5};
  EXPECT_THROW(spec.stratum_split_factor(0, split_factors), std::invalid_argument);
}

TEST(PartitionSpec, StratumSplitFactorRejectsOutOfBoundsStratum) {
  fims_popdy::PartitionSpec spec = fims_popdy::MakeDefaultSexPartitionSpec();
  const std::vector<double> split_factors = {0.5, 0.5};
  EXPECT_THROW(spec.stratum_split_factor(2, split_factors),
               std::invalid_argument);
}

TEST(SexStratumSplitFactors, DefaultSexPartition) {
  fims_popdy::PartitionSpec spec = fims_popdy::MakeDefaultSexPartitionSpec();
  const double p_female = 0.3;
  const std::vector<double> split_factors =
      fims_popdy::SexStratumSplitFactors(spec, p_female);

  ASSERT_EQ(split_factors.size(), 2);
  EXPECT_DOUBLE_EQ(split_factors[0], p_female);
  EXPECT_DOUBLE_EQ(split_factors[1], 1.0 - p_female);
  EXPECT_DOUBLE_EQ(spec.stratum_split_factor(0, split_factors), p_female);
  EXPECT_DOUBLE_EQ(spec.stratum_split_factor(1, split_factors),
                   1.0 - p_female);
}

// Verifies sex level is read from each encoded stratum. Not a production
// split recipe for multi-axis specs; area allocation is user-defined.
TEST(SexStratumSplitFactors, ReadsSexLevelFromMultiAxisSpec) {
  fims_popdy::PartitionSpec spec;
  fims_popdy::Axis sex_axis;
  sex_axis.name = "sex";
  sex_axis.levels = {"female", "male"};
  fims_popdy::Axis area_axis;
  area_axis.name = "area";
  area_axis.levels = {"north", "east", "south", "west"};
  spec.axes.push_back(std::move(sex_axis));
  spec.axes.push_back(std::move(area_axis));

  const double p_female = 0.4;
  const std::vector<double> split_factors =
      fims_popdy::SexStratumSplitFactors(spec, p_female);

  ASSERT_EQ(split_factors.size(), 8);
  for (size_t stratum = 0; stratum < 4; ++stratum) {
    EXPECT_DOUBLE_EQ(split_factors[stratum], p_female);
  }
  for (size_t stratum = 4; stratum < 8; ++stratum) {
    EXPECT_DOUBLE_EQ(split_factors[stratum], 1.0 - p_female);
  }
}

TEST(SexStratumSplitFactors, RejectsMissingSexAxis) {
  fims_popdy::PartitionSpec spec;
  fims_popdy::Axis area_axis;
  area_axis.name = "area";
  area_axis.levels = {"north", "south"};
  spec.axes.push_back(std::move(area_axis));

  EXPECT_THROW(fims_popdy::SexStratumSplitFactors(spec, 0.5),
               std::invalid_argument);
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
