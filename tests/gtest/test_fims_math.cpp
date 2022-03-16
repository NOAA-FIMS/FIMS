#include "gtest/gtest.h"
#include "fims_math.hpp"

namespace
{

  // Test exp with double values
  // R code that generates true values for the test
  // exp(3.0): 20.08554
  // exp(-2.5): 0.082085
  TEST(exp, use_double_inputs)
  {
    EXPECT_NEAR(fims::exp(3.0), 20.08554, 0.0001);
    EXPECT_EQ(fims::exp(-2.5), std::exp(-2.5));
    EXPECT_NEAR(fims::exp(-2.5), 0.082085, 0.0001);
  }

  // Test exp with integer values  
  TEST(exp, use_integer_inputs)
  {
    EXPECT_NEAR(fims::exp(3), 20.08554, 0.0001);
  }

  // Test log with double values 
  // R code that generates true values for the test
  // log(3.0): 1.098612
  // log(-2.5): 0.082085 
  TEST(log, use_double_inputs)
  {
    EXPECT_NEAR(fims::log(3.0), 1.098612, 0.0001);
    EXPECT_EQ(fims::log(2.5), std::log(2.5));
    EXPECT_TRUE(std::isnan(fims::log(-2.5)));
  }
  
  // Test logistic with double values  
  TEST(logistic, function)
  {
    double mean = 3.0;
    double slope = 4.0;
    int x_int = 5;
    double x_double = 5.0;
    
    EXPECT_NEAR(fims::logistic(mean, slope, x_double), 0.6224593, 0.0001);
    //EXPECT_NEAR(fims::logistic(mean, slope, x_int), 0.6224593, 0.0001);
  }




}