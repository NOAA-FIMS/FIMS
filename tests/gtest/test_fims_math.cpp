#include "gtest/gtest.h"
#include "fims_math.hpp"

namespace
{

  // Test exp using multiple input values and types
  // R code that generates true values for the test
  // exp(-2.5): 0.082085
  // exp(3.0): 20.08554
   
  // Not worth to write many tests when testing thin database wrappers, 
  // third-party libraries, or basic variable assignments.
  
  TEST(exp, use_multiple_input_values)
  {
    // Test exp using large negative input value
    EXPECT_EQ(fims::exp(-1000000.0), std::exp(-1000000.0));
    // Test exp using large positive input value
    EXPECT_EQ(fims::exp(1000000.0), std::exp(1000000.0));
    // Test exp using large positive integer value
    int input_value = 1000000;
    EXPECT_EQ(fims::exp(input_value), std::exp(input_value));
    // Test exp using double value 0.0
    EXPECT_EQ(fims::exp(0.0), std::exp(0.0));
    // Test exp using double value 1.0
    EXPECT_EQ(fims::exp(1.0), std::exp(1.0));
    // Test exp using double value 3.0
    EXPECT_NEAR(fims::exp(3.0), 20.08554, 0.0001);
    // Test exp using integer value 3
    // For fims::exp(3): the output value will be an integer if the input value is an integer
    // need to round the output value before using it as expected true value
    EXPECT_EQ(fims::exp(3), 20); 
    // Test exp using double value -2.5
    EXPECT_NEAR(fims::exp(-2.5), 0.082085, 0.0001);
  }

  // Test log using multiple input values and types 
  // R code that generates true values for the test
  // log(3.0): 1.098612
  // log(-2.5): NaN 
  TEST(log, use_multiple_input_values)
  {
    // Test log using large positive double value
    EXPECT_EQ(fims::log(1000000.0), std::log(1000000.0));
    // Test log using large positive integer value
    int input_value = 1000000;
    EXPECT_EQ(fims::log(input_value), std::log(input_value));
    // Test log using double value 3.0
    EXPECT_NEAR(fims::log(3.0), 1.098612, 0.0001);
    // Test log using negative value -2.5 and expect return of NaN
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