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
}

}  // namespace
