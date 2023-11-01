#include "gtest/gtest.h"
#include "common/fims_math.hpp"

namespace
{

  // Test ad_fabs
  TEST(AdFabs, UseDoubleValues)
  {
    // Expected value from R: x=2; sqrt(x*x+1e-5) = 2.000002
    EXPECT_NEAR(fims_math::ad_fabs(2.0), 2.000002, 0.000001);
    EXPECT_NE(fims_math::ad_fabs(2.0), 1.0);

    // Expected value from R: x=2; sqrt(x*x+1e-4) = 2.000025
    EXPECT_NEAR(fims_math::ad_fabs(2.0, 1e-4), 2.000025, 0.000001);
    
  }

  // Test ad_min
  TEST(AdMin, UseDoubleValues)
  {
    // Expected value from R: a=2.0; b=1.0; c=1e-5; 
    // (a+b-sqrt((a-b)^2+c))*0.5 = 0.9999975
    EXPECT_NEAR(fims_math::ad_min(2.0, 1.0), 0.9999975, 0.0000001);
    EXPECT_NE(fims_math::ad_min(2.0, 1.0), 2.0);

    // Expected value from R: a=2.0; b=1.0; c=1e-4; 
    // (a+b-sqrt((a-b)^2+c))*0.5 = 0.999975
    EXPECT_NEAR(fims_math::ad_min(2.0, 1.0, 1e-4), 0.999975, 0.000001);
  }

  // Test ad_max
  TEST(AdMax, UseDoubleValues)
  {
    // Expected value from R: a=2.0; b=1.0; c=1e-5; 
    // (a+b+sqrt((a-b)^2+c))*0.5 = 2.000002
    EXPECT_NEAR(fims_math::ad_max(2.0, 1.0), 2.000002, 0.000001);
    EXPECT_NE(fims_math::ad_max(2.0, 1.0), 1.0);

    // Expected value from R: a=2.0; b=1.0; c=1e-4; 
    // (a+b+sqrt((a-b)^2+c))*0.5 = 2.000025
    EXPECT_NEAR(fims_math::ad_max(2.0, 1.0, 1e-4), 2.000025, 0.000001);

  }

}
