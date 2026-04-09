#include "gtest/gtest.h"
#include "common/fims_math.hpp"

namespace
{

  // R code that generates expected values for the test:
  // dinvgamma <- function(x, shape, scale, logscale = TRUE) {
  //   ret <- shape * log(scale) - lgamma(shape) - (shape + 1) * log(x) -
  //          scale / x
  //   if(logscale) { return(ret) } else { return(exp(ret)) }
  // }

  TEST(Dinvgamma, UseMultipleDoubleValues)
  {
    // Test dinvgamma with log scale (default)
    // R: dinvgamma(1.0, 2.0, 3.0) = 2*log(3) - lgamma(2) - 3*log(1) - 3/1
    //                              = 2.197225 - 0 - 0 - 3 = -0.8027755
    EXPECT_NEAR(fims_math::dinvgamma(1.0, 2.0, 3.0), -0.8027755, 1e-6);

    // R: dinvgamma(2.0, 3.0, 4.0) = 3*log(4) - lgamma(3) - 4*log(2) - 4/2
    //                              = 4.158883 - 0.6931472 - 2.772589 - 2 = -1.306853
    EXPECT_NEAR(fims_math::dinvgamma(2.0, 3.0, 4.0), -1.306853, 1e-6);

    // Test dinvgamma with logscale = false (probability density)
    // R: dinvgamma(1.0, 2.0, 3.0, logscale = FALSE) = exp(-0.8027755) = 0.4480836
    EXPECT_NEAR(fims_math::dinvgamma(1.0, 2.0, 3.0, false), 0.4480836, 1e-6);
  }

  TEST(Dinvgamma, UseScalarShapeOne)
  {
    // Test with shape = 1 (exponential distribution case for inverse)
    // R: dinvgamma(0.5, 1.0, 1.0) = 1*log(1) - lgamma(1) - 2*log(0.5) - 1/0.5
    //                              = 0 - 0 - (-1.386294) - 2 = -0.6137056
    EXPECT_NEAR(fims_math::dinvgamma(0.5, 1.0, 1.0), -0.6137056, 1e-6);
  }

  TEST(Dinvgamma, UseSmallXValue)
  {
    // Test with small positive x
    // R: dinvgamma(0.1, 2.0, 1.0) = 2*log(1) - lgamma(2) - 3*log(0.1) - 1/0.1
    //                              = 0 - 0 - (-6.907755) - 10 = -3.092245
    EXPECT_NEAR(fims_math::dinvgamma(0.1, 2.0, 1.0), -3.092245, 1e-5);
  }

  TEST(Dinvgamma, UseLargeXValue)
  {
    // Test with large x
    // R: dinvgamma(100.0, 2.0, 1.0) = 2*log(1) - lgamma(2) - 3*log(100) -
    //                                  1/100
    //                              = 0 - 0 - 13.81551 - 0.01 = -13.82551
    EXPECT_NEAR(fims_math::dinvgamma(100.0, 2.0, 1.0), -13.82551, 1e-4);
  }

}
