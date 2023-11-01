#include "gtest/gtest.h"
#include "population_dynamics/selectivity/functors/double_logistic.hpp"

namespace
{

  TEST(double_logistic_selectivity, create_object)
  {
    
    fims_popdy::DoubleLogisticSelectivity<double> fishery_selectivity;
    fishery_selectivity.inflection_point_asc = 10.5;
    fishery_selectivity.slope_asc = 0.2;
    fishery_selectivity.inflection_point_desc = 15.0;
    fishery_selectivity.slope_desc = 0.05;
    double fishery_x = 34.5;
    // 1.0/(1.0+exp(-(34.5-10.5)*0.2)) * (1.0 - 1.0/(1.0+exp(-(34.5-15.0)*0.05))) = 0.2716494
    double expect_fishery = 0.2716494;
    EXPECT_NEAR(fishery_selectivity.evaluate(fishery_x), expect_fishery, 0.0001);

  }

}