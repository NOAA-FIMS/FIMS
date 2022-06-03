#include "gtest/gtest.h"
#include "population_dynamics/recruitment/functors/recruitment_base.hpp"
#include "population_dynamics/recruitment/functors/sr_beverton_holt.hpp"

 
namespace
{
  TEST(recruitment_deviations, constraint_works)
  {
    fims::SRBevertonHolt<double> recruit;
    recruit.rec_deviations = {-1.0, 2.0, 3.0};

    // Test that the constrain_deviations = false works
    recruit.constrain_deviations = false;
    recruit.PrepareConstrainedDeviations();

    for (int i = 0; i < recruit.rec_deviations.size(); i++)
    {
      EXPECT_EQ(recruit.rec_deviations[i],
                recruit.rec_deviations[i]);
    }

    // Test that the constrain_deviations = true works
    recruit.constrain_deviations = true;
    recruit.PrepareConstrainedDeviations();
    // c(-1.0, 2.0, 3.0)-sum(c(-1.0, 2.0, 3.0))/3 = -2.3333333  0.6666667  1.6666667
    std::vector<double> expected_deviations = {-2.3333333, 0.6666667, 1.6666667};

    for (int i = 0; i < recruit.rec_deviations.size(); i++)
    {
      EXPECT_NEAR(recruit.rec_deviations[i],
                expected_deviations[i], 0.0000001);
    }
  }

}
