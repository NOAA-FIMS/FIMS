#include "gtest/gtest.h"
#include "population_dynamics/recruitment/functors/sr_beverton_holt.hpp"

 
namespace
{
  TEST(sr_beverton_holt_evaluate, use_multiple_inputs)
  {
// BH_fcn(R0 = 1000, h = 0.75, phi0 = 0.1, x = 30): 837.2093 
// BH_fcn(R0 = 1000, h = 0.99, phi0 = 0.1, x = 30): 994.1423
// BH_fcn(R0 = 1000, h = 0.75, phi0 = 0.3, x = 30): 679.2453
// BH_fcn(R0 = 1000, h = 0.99, phi0 = 0.3, x = 30): 985.8921
// BH_fcn(R0 = 1000, h = 0.2, phi0 = 0.2, x = 40): 200
// BH_fcn(R0 = 1000, h = 0.99, phi0 = 0.2, x = 40): 990

      EXPECT_EQ(1.0, 1.0);
      
      fims::SRBevertonHolt<double> recruit1;
      recruit1.steep = 0.7500;
      recruit1.rzero = 1000.000;
      double ssbzero =  100.000;
      double spawners = 30.000;
      // # R code that generates true values for testing
      // BH_fcn <- function(R0, h, phi0, x) {
      //  # R0 = unfished recruitment
      //  # h = steepness
      //  # phi0 = unfished spawners per recruit
      //  # x = spawners
      //  recruits <- (0.8 * R0 * h * x) / (0.2 * 100.0 * (1.0 - h) + x * (h //  - 0.2))
      //  return(recruits)
      // }
      // (0.8 * 1000.0 * 0.75 * 30.0) / (0.2 * 100.0 * (1.0 - 0.75) + 30.0 * (0.75 - 0.2)) = 837.2093
      double expect_recruit1 = 837.209300;
      EXPECT_NEAR(recruit1.evaluate(spawners,ssbzero), expect_recruit1, 0.0001);
      EXPECT_EQ(recruit1.GetId(), 0);
      
      fims::SRBevertonHolt<double> recruit2;
      recruit2.steep = 0.200;
      recruit2.rzero = 1000.000;
      double spawners2 = 30.000;
      double ssbzero2 = 100.000;
      // # R code that generates true values for testing
      // BH_fcn <- function(R0, h, phi0, x) {
      //  # R0 = unfished recruitment
      //  # h = steepness
      //  # phi0 = unfished spawners per recruit
      //  # x = spawners
      //  recruits <- (0.8 * R0 * h * x) / (0.2 * ssb0 * (1.0 - h) + x * (h //  - 0.2))
      //  return(recruits)
      // }
      // (0.8 * 1000.0 * 0.2 * 30.0) / (0.2 * 100.0 * (1.0 - 0.2) + 30.0 * (0.2 - 0.2)) = 300
      double expect_recruit2 = 300.0;
      EXPECT_NEAR(recruit2.evaluate(spawners2, ssbzero2), expect_recruit2, 0.0001);
      EXPECT_EQ(recruit2.GetId(), 1);
  }

}
