#include "gtest/gtest.h"
#include "likelihoods/functors/tmb_likelihoods.hpp"

namespace
{

  // Test dnorm using multiple input values
  TEST(dnorm, use_multiple_input_values)
  {
    //EXPECT_EQ(1.0, 1.0);
    std::vector<double> observed = {0, -10.0, 10.0};
    std::vector<double> expected = {0, 0, 5};
    std::vector<double> sd = {1, 1, 3};

    // R code that generates true values for the test
    // dnorm(0,0,1, TRUE) = -0.9189385
    // dnorm(-10,0,1, TRUE) = -50.91894
    // dnorm(10,0,1,TRUE) = -3.40644
    std::vector<double> expect_value = {-0.9189385, -50.91894,-3.40644};

    for (int i = 0; i < expect_value.size(); ++i)
    {
      EXPECT_NEAR(fims::dnorm(observed[i], expected[i], sd[i], 1), expect_value[i], 0.0001);
    }
  }

}