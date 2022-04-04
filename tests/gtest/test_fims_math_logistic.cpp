#include "gtest/gtest.h"
#include "common/fims_math.hpp"

namespace
{

  // Test logistic using multiple input values
  TEST(logistic, use_multiple_input_values)
  {
    std::vector<double> median_value = {1.0, 10.0, 20.5};
    std::vector<double> slope_value = {0.0, 0.2, 0.05};
    std::vector<double> x_value = {2.0, 20.0, 40.5};

    // R code that generates true values for the test
    // 1.0/(1.0+exp(-(2.0-1.0)*0.0)) = 0.5
    // 1.0/(1.0+exp(-(20.0-10.0)*0.2)) = 0.8807971
    // 1.0/(1.0+exp(-(40.5-20.5)*0.05)) = 0.7310586
    std::vector<double> expect_value = {0.5, 0.8807971, 0.7310586};

    for (int i = 0; i < expect_value.size(); ++i)
    {
      EXPECT_NEAR(fims::logistic(median_value[i], slope_value[i], x_value[i]), expect_value[i], 0.0001);
    }
  }

}