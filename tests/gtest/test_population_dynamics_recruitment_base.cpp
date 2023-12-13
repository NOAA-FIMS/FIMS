#include "gtest/gtest.h"
#include "population_dynamics/recruitment/functors/recruitment_base.hpp"
#include "population_dynamics/recruitment/functors/sr_beverton_holt.hpp"

namespace
{
  TEST(RecruitmentDeviations, ConstraintWorks)
  {
    fims_popdy::SRBevertonHolt<double> recruit;
    recruit.log_recruit_devs.resize(3);
    recruit.log_recruit_devs[0] = -1.0;
    recruit.log_recruit_devs[1] = 2.0;
    recruit.log_recruit_devs[2] = 3.0;

    // Test if constrain_deviations = false works
    recruit.constrain_deviations = false;
    recruit.PrepareConstrainedDeviations();

    fims::Vector<double> expected_deviations_false(3, 0);
    expected_deviations_false[0] = -1.0;
    expected_deviations_false[1] = 2.0;
    expected_deviations_false[2] = 3.0;
    for (int i = 0; i < recruit.log_recruit_devs.size(); i++)
    {
      EXPECT_EQ(recruit.log_recruit_devs[i],
                expected_deviations_false[i]);
    }

    // Test if constrain_deviations = true works
    recruit.constrain_deviations = true;
    recruit.PrepareConstrainedDeviations();
    // c(-1.0, 2.0, 3.0)-sum(c(-1.0, 2.0, 3.0))/3 = -2.3333333  0.6666667  1.6666667
    std::vector<double> expected_deviations_true = {-2.3333333, 0.6666667, 1.6666667};

    for (int i = 0; i < recruit.log_recruit_devs.size(); i++)
    {
      EXPECT_NEAR(recruit.log_recruit_devs[i],
                  expected_deviations_true[i], 0.0000001);
    }
  }

}
