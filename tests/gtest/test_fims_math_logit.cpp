#include "gtest/gtest.h"
#include "common/fims_math.hpp"

namespace
{

  // Test logit using multiple input values
  TEST(logit, use_multiple_input_values)
  {
    std::vector<double> max_value = {1.0, 10.0, 20.5};
    std::vector<double> min_value = {0.0, 0.2, 0.05};
    std::vector<double> x_value = {0.5, 2.0, 4.5};

    // R code that generates true values for the test
    // -log(max_value - x_value) + log(x_value - min_value)
    // =  0.000000 -1.491655 -1.279685
    std::vector<double> expect_value = {0.0, -1.491655, -1.279685};

  }

  TEST(inv_logit, use_multiple_input_values)
  {
    std::vector<double> max_value = {1.0, 10.0, 20.5};
    std::vector<double> min_value = {0.0, 0.2, 0.05};
    std::vector<double> logit_x_value = {0, -1.491655, -1.279685};

    // R code that generates true values for the test
    // max_value - (max_value - min_value)/(1+exp(-logit_x_value))
    // 
    std::vector<double> expect_value = {0.5, 2.0, 4.5};

    for (int i = 0; i < expect_value.size(); ++i)
    {
      EXPECT_NEAR(fims::inv_logit(min_value[i], max_value[i], logit_x_value[i]), 
      expect_value[i], 0.0001);
    }
  }

  TEST(inv_logit_logit, use_multiple_input_values)
  {
    
    std::vector<double> max_value = {1.0, 1.0};
    std::vector<double> min_value = {0.0, 0.0};
    std::vector<double> x_value = {0.0, 1.0};

    for (int i = 0; i < x_value.size(); ++i)
    {
      EXPECT_EQ(fims::inv_logit(min_value[i], max_value[i], 
      fims::logit(min_value[i], max_value[i], x_value[i])),
      x_value[i]); 
    }
  }

  TEST(logit_inv_logit, use_multiple_input_values)
  {
    
    std::vector<double> max_value = {1.0, 1.0};
    std::vector<double> min_value = {0.0, 0.0};
    std::vector<double> x_value = {-INFINITY, INFINITY};

    for (int i = 0; i < x_value.size(); ++i)
    {
      EXPECT_EQ(fims::logit(min_value[i], max_value[i], 
      fims::inv_logit(min_value[i], max_value[i], x_value[i])),
      x_value[i]); 
    }


  }

}