#include "gtest/gtest.h"
#include "interface/place_holder1.hpp"

// # R code that generates true values for the test
// dlnorm(1.0, 0.0, 1.0, TRUE): -0.9189385
// dlnorm(5.0, 10.0, 2.5, TRUE): -9.07679
// dlnorm(0.5, 0.0, 1.0, TRUE): -0.4660179
// dlnorm(c(1.0, 2.0), 0.0, 1.0, TRUE): -0.9189385 -1.8523122

namespace
{

  // Test dlognorm with double input values
  TEST(dlognorm, use_double_inputs)
  {
    // test value, expected value, tolerance
    EXPECT_NEAR(dlognorm(1.0, 0.0, 1.0), -0.9189385, 0.0001);
    EXPECT_NEAR(dlognorm(5.0, 10.0, 2.5), -9.07679, 0.0001);
  }

  // Test dlognorm with integer input values
  TEST(dlognorm, use_int_inputs)
  {
    // expect not equal (look up arguments)
    EXPECT_NE(dlognorm(1, 0, 1), -0.9189385);
  }

}
