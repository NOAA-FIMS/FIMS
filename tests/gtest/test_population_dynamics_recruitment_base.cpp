#include "gtest/gtest.h"
#include "population_dynamics/recruitment/functors/recruitment_base.hpp"
#include "population_dynamics/recruitment/functors/sr_beverton_holt.hpp"

namespace
{
  TEST(recruitment_deviations, constraint_works)
  {
    fims::SRBevertonHolt<double> recruit;
    recruit.recruit_deviations = {-1.0, 2.0, 3.0};

    // Test if constrain_deviations = false works
    recruit.constrain_deviations = false;
    recruit.PrepareConstrainedDeviations();

    std::vector<double> expected_deviations_false = {-1.0, 2.0, 3.0};
    for (int i = 0; i < recruit.recruit_deviations.size(); i++)
    {
      EXPECT_EQ(recruit.recruit_deviations[i],
                expected_deviations_false[i]);
    }

    // Test if constrain_deviations = true works
    recruit.constrain_deviations = true;
    recruit.PrepareConstrainedDeviations();
    // c(-1.0, 2.0, 3.0)-sum(c(-1.0, 2.0, 3.0))/3 = -2.3333333  0.6666667  1.6666667
    std::vector<double> expected_deviations_true = {-2.3333333, 0.6666667, 1.6666667};

    for (int i = 0; i < recruit.recruit_deviations.size(); i++)
    {
      EXPECT_NEAR(recruit.recruit_deviations[i],
                  expected_deviations_true[i], 0.0000001);
    }
  }

}
