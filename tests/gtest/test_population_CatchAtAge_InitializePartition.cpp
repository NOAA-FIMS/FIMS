#include "gtest/gtest.h"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace {

TEST_F(CAAInitializeTestFixture, InitializeSetsDefaultSexPartition) {
  catch_at_age_model->Initialize();
  const auto &population = catch_at_age_model->populations[0];

  EXPECT_EQ(population->partition_spec.n_strata(), 2);
  EXPECT_EQ(population->partition_spec.axes[0].name, "sex");
  EXPECT_EQ(population->index_layout.n_years, n_years);
  EXPECT_EQ(population->index_layout.n_ages, n_ages);
  EXPECT_EQ(population->index_layout.n_strata, 2);
  EXPECT_EQ(population->index_layout.i_stratum_age_year(1, 0, 0),
            n_years * n_ages);

  EXPECT_TRUE(population->partition_demand.is_pooled());
  EXPECT_TRUE(fims_popdy::RequestedStrata(population->partition_spec,
                                          population->partition_demand)
                  .empty());
}

// Initialize must not wipe demand set before model init (e.g. from R via
// add_to_fims_tmb). Default remains pooled when unset.
TEST_F(CAAInitializeTestFixture, InitializePreservesPartitionDemand) {
  auto &population = catch_at_age_model->populations[0];
  population->partition_demand =
      fims_popdy::MakeSexPartitionDemand({"female"});

  catch_at_age_model->Initialize();

  EXPECT_FALSE(population->partition_demand.is_pooled());
  const std::vector<size_t> strata = fims_popdy::RequestedStrata(
      population->partition_spec, population->partition_demand);
  ASSERT_EQ(strata.size(), 1);
  EXPECT_EQ(strata[0], 0);
}

}  // namespace
