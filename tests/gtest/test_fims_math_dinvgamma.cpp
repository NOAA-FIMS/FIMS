#include "gtest/gtest.h"
#include "common/fims_math.hpp"

namespace {

// Reference implementation of the inverse gamma log-density matching the
// R formulation in the issue:
//   dinvgamma <- function(x, shape, scale, logscale = TRUE) {
//     ret <- shape * log(scale) - lgamma(shape) - (shape + 1) * log(x) -
//            scale / x
//     ...
//   }
double dinvgamma_r(double x, double shape, double scale,
                   bool logscale = true) {
  double ret = shape * std::log(scale) - std::lgamma(shape) -
               (shape + 1.0) * std::log(x) - scale / x;
  return logscale ? ret : std::exp(ret);
}

TEST(FimsMathDinvgamma, LogScaleBasicValues) {
  // Test that fims_math::dinvgamma matches the reference formula in log scale
  EXPECT_NEAR(fims_math::dinvgamma(1.0, 2.0, 1.0),
              dinvgamma_r(1.0, 2.0, 1.0), 1e-10);
  EXPECT_NEAR(fims_math::dinvgamma(2.0, 3.0, 5.0),
              dinvgamma_r(2.0, 3.0, 5.0), 1e-10);
  EXPECT_NEAR(fims_math::dinvgamma(0.5, 1.0, 2.0),
              dinvgamma_r(0.5, 1.0, 2.0), 1e-10);
  EXPECT_NEAR(fims_math::dinvgamma(10.0, 4.0, 2.0),
              dinvgamma_r(10.0, 4.0, 2.0), 1e-10);
}

TEST(FimsMathDinvgamma, NonLogScaleBasicValues) {
  // Test that fims_math::dinvgamma returns the density (not log) when
  // logscale = false
  EXPECT_NEAR(fims_math::dinvgamma(1.0, 2.0, 1.0, false),
              dinvgamma_r(1.0, 2.0, 1.0, false), 1e-10);
  EXPECT_NEAR(fims_math::dinvgamma(2.0, 3.0, 5.0, false),
              dinvgamma_r(2.0, 3.0, 5.0, false), 1e-10);
}

TEST(FimsMathDinvgamma, SmallXValues) {
  // Test with small positive x values
  EXPECT_NEAR(fims_math::dinvgamma(1e-4, 2.0, 1.0),
              dinvgamma_r(1e-4, 2.0, 1.0), 1e-6);
}

TEST(FimsMathDinvgamma, LargeXValues) {
  // Test with large x values
  EXPECT_NEAR(fims_math::dinvgamma(1e4, 2.0, 1.0),
              dinvgamma_r(1e4, 2.0, 1.0), 1e-6);
}

TEST(FimsMathDinvgamma, LargeShapeValues) {
  // Test with large shape parameter
  EXPECT_NEAR(fims_math::dinvgamma(1.0, 100.0, 50.0),
              dinvgamma_r(1.0, 100.0, 50.0), 1e-6);
}

}  // namespace
