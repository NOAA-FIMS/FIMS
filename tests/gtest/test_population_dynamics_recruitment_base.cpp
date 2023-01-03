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

  TEST(recruitment_bias_adjustment, PrepareBiasAdjustment_works)
  {
    fims::SRBevertonHolt<double> recruit;
    recruit.log_sigma_recruit = fims::log(0.7);

    // Test if use_recruit_bias_adjustment = false works
    recruit.use_recruit_bias_adjustment = false;
    recruit.recruit_bias_adjustment = {0.0, 0.0, 0.0};
    recruit.recruit_bias_adjustment_fraction = {1.0, 1.0, 1.0};
    recruit.PrepareBiasAdjustment();

    std::vector<double> expected_bias_adjustment_false = {0.0, 0.0, 0.0};
    for (int i = 0; i < recruit.recruit_bias_adjustment.size(); i++)
    {
      EXPECT_EQ(recruit.recruit_bias_adjustment[i],
                expected_bias_adjustment_false[i]);
    }

    // Test if use_recruit_bias_adjustment = true works
    recruit.use_recruit_bias_adjustment = true;
    recruit.recruit_bias_adjustment = {0.0, 0.0, 0.0};
    recruit.recruit_bias_adjustment_fraction = {1.0, 1.0, 1.0};
    recruit.PrepareBiasAdjustment();

    // R code to generate true values: 0.5 * 0.7^2 * c(1.0, 1.0, 1.0)
    // 0.245 0.245 0.245
    std::vector<double> expected_bias_adjustment_true = {0.245, 0.245, 0.245};
    for (int i = 0; i < recruit.recruit_bias_adjustment.size(); i++)
    {
      EXPECT_NEAR(recruit.recruit_bias_adjustment[i],
                  expected_bias_adjustment_true[i], 0.001);
    }

    // Test if recruit_bias_adjustment_fraction will be converted to 1.0
    // when initial input value is 2.0
    recruit.recruit_bias_adjustment_fraction = {2.0, 2.0, 2.0};
    recruit.PrepareBiasAdjustment();

    // R code to generate true values: 0.5 * 0.7^2 * c(1.0, 1.0, 1.0)
    // 0.245 0.245 0.245
    std::vector<double> expected_bias_adjustment = {0.245, 0.245, 0.245};
    for (int i = 0; i < recruit.recruit_bias_adjustment.size(); i++)
    {
      EXPECT_NEAR(recruit.recruit_bias_adjustment[i],
                  expected_bias_adjustment[i], 0.001);
    }
  }

}
