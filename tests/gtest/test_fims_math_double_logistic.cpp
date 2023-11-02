#include "gtest/gtest.h"
#include "common/fims_math.hpp"

namespace
{
  // Test double logistic using multiple input values
  TEST(DoubleLogistic, UseMultipleInputValues)
  {
    std::vector<double> inflection_point_asc_value = {1.0, 10.0, 20.5};
    std::vector<double> slope_asc_value = {0.0, 0.2, 0.05};
    std::vector<double> inflection_point_desc_value = {6.0, 15.0, 23.5};
    std::vector<double> slope_desc_value = {0.0, 0.2, 0.05};
    std::vector<double> x_value = {2.0, 20.0, 40.5};

    // R code that generates true values for the test
    // 1.0/(1.0+exp(-(2.0-1.0)*0.0)) * (1.0 - 1.0/(1.0+exp(-(2.0-6.0)*0.0))) = 0.25
    // 1.0/(1.0+exp(-(20.0-10.0)*0.2)) * (1.0 - 1.0/(1.0+exp(-(20.0-15.0)*0.2))) = 0.2368828
    // 1.0/(1.0+exp(-(40.5-20.5)*0.05)) * (1.0 - 1.0/(1.0+exp(-(40.5-23.5)*0.05))) = 0.218903

    std::vector<double> expect_value = {0.25, 0.2368828, 0.218903};

    for (int i = 0; i < expect_value.size(); ++i)
    {
      EXPECT_NEAR(fims_math::double_logistic(inflection_point_asc_value[i], slope_asc_value[i], inflection_point_desc_value[i], slope_desc_value[i], x_value[i]), expect_value[i], 0.0001);
    }
  }

}