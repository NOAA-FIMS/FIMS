#include "gtest/gtest.h"
#include "population_dynamics/maturity/functors/logistic.hpp"

namespace
{

  
  TEST(LogisticMaturity, CreateObject)
  {
    
    fims_popdy::LogisticMaturity<double> maturity;
    maturity.inflection_point = 20.5;
    maturity.slope = 0.15;
    double maturity_x = 40.5;
    // 1.0/(1.0+exp(-(40.5-20.5)*0.15)) = 0.9525741
    double expect_maturity = 0.9525741;
    EXPECT_NEAR(maturity.evaluate(maturity_x), expect_maturity, 0.0001);


  }

}