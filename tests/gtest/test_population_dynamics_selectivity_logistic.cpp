#include "gtest/gtest.h"
#include "population_dynamics/selectivity/functors/logistic.hpp"

namespace
{

  
  TEST(LogisticSelectivity, CreateObject)
  {
    
    fims_popdy::LogisticSelectivity<double> fishery_selectivity;
    fishery_selectivity.inflection_point.resize(1);
    fishery_selectivity.slope.resize(1);
    fishery_selectivity.inflection_point[0] = 20.5;
    fishery_selectivity.slope[0] = 0.2;
    double fishery_x = 40.5;
    // 1.0/(1.0+exp(-(40.5-20.5)*0.2)) = 0.9820138
    double expect_fishery = 0.9820138;
    EXPECT_NEAR(fishery_selectivity.evaluate(fishery_x), expect_fishery, 0.0001);


  }

}