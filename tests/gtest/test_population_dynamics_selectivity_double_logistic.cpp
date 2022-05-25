#include "gtest/gtest.h"
#include "population_dynamics/selectivity/functors/double_logistic.hpp"

namespace
{


  TEST(double_logistic_selectivity, create_object)
  {
    
    fims::DoubleLogisticSelectivity<double> fishery_selectivity;
    fishery_selectivity.median1 = 10.5;
    fishery_selectivity.slope1 = 0.2;
    fishery_selectivity.median2 = 15.0;
    fishery_selectivity.slope2 = 0.05;
    double fishery_x = 34.5;
    // 1.0/(1.0+exp(-(34.5-10.5)*2.0)) * (1.0 - 1.0/(1.0+exp(-(34.5-15.0)*0.05))) = 0.273885
    double expect_fishery = 0.273885;
    EXPECT_NEAR(fishery_selectivity.evaluate(fishery_x), expect_fishery, 0.0001);

  }

}