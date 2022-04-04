#include "gtest/gtest.h"
#include "common/fims_math.hpp"

namespace
{

  // Test log using multiple input values and types
  // Not worth to write many tests when testing thin database wrappers,
  // third-party libraries, or basic variable assignments.

  TEST(log, use_multiple_double_values)
  {
    // Test log using large double value
    EXPECT_EQ(fims::log(1000000.0), std::log(1000000.0));

    // Test log using double value 3.0
    // R code that generates true values for the test
    // log(3.0): 1.098612
    EXPECT_NEAR(fims::log(3.0), 1.098612, 0.0001);
  }

  TEST(log, use_integer_values)
  {
    // Test log using large positive integer value
    int large_int = 1000000;
    EXPECT_EQ(fims::log<double>(large_int), std::log(large_int));

    // log(3.0): 1.098612
    int small_int = 3;
    EXPECT_EQ(fims::log<double>(small_int), std::log(small_int));
  }

  // log(-2.5): NaN
  TEST(log, use_negative_double_values)
  {
    // Test log using negative value -2.5 and expect return of NaN
    EXPECT_TRUE(std::isnan(fims::log(-2.5)));
  }

}
