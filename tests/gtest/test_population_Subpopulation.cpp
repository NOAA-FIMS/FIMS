#include "gtest/gtest.h"
#include "population_dynamics/population/subpopulation.hpp"
#include <limits>

TEST(Subpopulation, ConstructsIndependentTwoSexConfiguration) {
  auto groups = fims_popdy::MakeTwoSexSubpopulations(
      fims::Vector<double>{1.0, 2.0}, fims::Vector<double>{3.0, 4.0}, 0.4);
  EXPECT_EQ(groups[0].name, "female");
  EXPECT_EQ(groups[1].name, "male");
  EXPECT_DOUBLE_EQ(groups[0].recruitment_fraction, 0.4);
  EXPECT_DOUBLE_EQ(groups[1].recruitment_fraction, 0.6);
  EXPECT_DOUBLE_EQ(groups[0].reproductive_contribution, 1.0);
  EXPECT_DOUBLE_EQ(groups[1].reproductive_contribution, 0.0);
  EXPECT_DOUBLE_EQ(groups[1].log_init_naa[1], 4.0);
  groups[0].log_init_naa[0] = 9.0;
  EXPECT_DOUBLE_EQ(groups[1].log_init_naa[0], 3.0);
  EXPECT_EQ(groups[0].growth_id, -999);
  EXPECT_EQ(groups[0].maturity_id, -999);
  EXPECT_TRUE(groups[0].selectivity_ids.empty());
}

TEST(Subpopulation, ValidatesNamesAndConservation) {
  std::vector<fims_popdy::Subpopulation<double>> empty;
  EXPECT_NO_THROW(fims_popdy::ValidateSubpopulations(empty));
  auto groups = fims_popdy::MakeTwoSexSubpopulations(
      fims::Vector<double>{0.0}, fims::Vector<double>{0.0});
  groups[1].name = "female";
  EXPECT_THROW(fims_popdy::ValidateSubpopulations(groups), std::invalid_argument);
  groups[1].name = "";
  EXPECT_THROW(fims_popdy::ValidateSubpopulations(groups), std::invalid_argument);
  groups[1].name = "male";
  groups[1].recruitment_fraction = 0.4;
  EXPECT_THROW(fims_popdy::ValidateSubpopulations(groups), std::invalid_argument);
  groups[1].recruitment_fraction = 0.5;
  groups[0].reproductive_contribution = 0.0;
  EXPECT_THROW(fims_popdy::ValidateSubpopulations(groups), std::invalid_argument);
}

TEST(Subpopulation, RejectsInvalidFractions) {
  for (double invalid : {-0.1, 1.1,
                         std::numeric_limits<double>::infinity(),
                         std::numeric_limits<double>::quiet_NaN()}) {
    auto groups = fims_popdy::MakeTwoSexSubpopulations(
        fims::Vector<double>{0.0}, fims::Vector<double>{0.0});
    groups[0].recruitment_fraction = invalid;
    EXPECT_THROW(fims_popdy::ValidateSubpopulations(groups), std::invalid_argument);
    groups[0].recruitment_fraction = 0.5;
    groups[0].reproductive_contribution = invalid;
    EXPECT_THROW(fims_popdy::ValidateSubpopulations(groups), std::invalid_argument);
  }
}

TEST(Subpopulation, SupportsAdditionalGroupsAndTypedAbundance) {
  std::vector<fims_popdy::Subpopulation<float>> groups{
      {"a", fims::Vector<float>{1.0f}, 0.2, 1.0},
      {"b", fims::Vector<float>{2.0f}, 0.3, 0.5},
      {"c", fims::Vector<float>{3.0f}, 0.5, 0.0}};
  EXPECT_NO_THROW(fims_popdy::ValidateSubpopulations(groups));
  EXPECT_FLOAT_EQ(groups[2].log_init_naa[0], 3.0f);
}

TEST(Subpopulation, LayoutIsContiguousWithAgeFastest) {
  const fims_popdy::SubpopulationLayout layout{3, 4};
  size_t expected = 0;
  for (size_t group = 0; group < 2; ++group)
    for (size_t year = 0; year < 3; ++year)
      for (size_t age = 0; age < 4; ++age)
        EXPECT_EQ(layout.index(group, year, age), expected++);
}
