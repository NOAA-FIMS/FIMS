#include "gtest/gtest.h"
#include "common/fims_math.hpp"
#include <Rmath.h>

namespace {

constexpr double kTol = 1e-12;

// Reference implementation of dinvgamma in R for testing
double dinvgamma_r(double x, double shape, double scale, int logscale = 0) {
  // get log density of the inverse gamma distribution using the relationship to the gamma distribution
  double ret = dgamma(1/x, shape, scale, 1) - 2 * fims_math::log(x);
  return logscale ? ret : std::exp(ret);
}

TEST(FimsMathDinvgamma, LogScaleBasicValues) {
  // Test that fims_math::dinvgamma returns the log-density when
  // logscale = true
  EXPECT_NEAR(fims_math::dinvgamma(1.0, 2.0, 1.0, true),
              dinvgamma_r(1.0, 2.0, 1.0, true), kTol);
  EXPECT_NEAR(fims_math::dinvgamma(2.0, 3.0, 5.0, true),
              dinvgamma_r(2.0, 3.0, 5.0, true), kTol);
  EXPECT_NEAR(fims_math::dinvgamma(0.5, 1.0, 2.0, true),
              dinvgamma_r(0.5, 1.0, 2.0, true), kTol);
  EXPECT_NEAR(fims_math::dinvgamma(10.0, 4.0, 2.0, true),
              dinvgamma_r(10.0, 4.0, 2.0, true), kTol);
  EXPECT_NEAR(fims_math::dinvgamma(3.0, 0.5, 3.0, true),
              dinvgamma_r(3.0, 0.5, 3.0, true), kTol);
}

TEST(FimsMathDinvgamma, NonLogScaleBasicValues) {
  // Test that fims_math::dinvgamma returns the density (not log) when
  // logscale = false (default)
  EXPECT_NEAR(fims_math::dinvgamma(1.0, 2.0, 1.0),
              dinvgamma_r(1.0, 2.0, 1.0), kTol);
  EXPECT_NEAR(fims_math::dinvgamma(2.0, 3.0, 5.0),
              dinvgamma_r(2.0, 3.0, 5.0), kTol);
}

TEST(FimsMathDinvgamma, SmallXValues) {
  // Test with small positive x values
  EXPECT_NEAR(fims_math::dinvgamma(1e-4, 2.0, 1.0, true),
              dinvgamma_r(1e-4, 2.0, 1.0, true), kTol);
}

TEST(FimsMathDinvgamma, LargeXValues) {
  // Test with large x values
  EXPECT_NEAR(fims_math::dinvgamma(1e4, 2.0, 1.0, true),
              dinvgamma_r(1e4, 2.0, 1.0, true), kTol);
}

TEST(FimsMathDinvgamma, LargeShapeValues) {
  // Test with large shape parameter
  EXPECT_NEAR(fims_math::dinvgamma(1.0, 100.0, 50.0, true),
              dinvgamma_r(1.0, 100.0, 50.0, true), kTol);
}

}  // namespace