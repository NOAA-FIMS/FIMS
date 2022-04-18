#include "gtest/gtest.h"
#include “population_dynamics/recruitment/functors/sr_beverton_holt.hpp”

// # R code that generates true values for testing
// BH_fcn <- function(R0, h, phi0, x) {
//  # R0 = unfished recruitment
//  # h = steepness
//  # phi0 = unfished spawners per recruit
//  # x = spawners
//  recruits <- (0.8 * R0 * h * x) / (0.2 * R0 * phi0 * (1.0 - h) + x * (h //  - 0.2))
//  return(recruits)
// }
//
// BH_fcn(R0 = 1000, h = 0.75, phi0 = 0.1, x = 30): 837.2093 
// BH_fcn(R0 = 1000, h = 0.99, phi0 = 0.1, x = 30): 994.1423
// BH_fcn(R0 = 1000, h = 0.75, phi0 = 0.3, x = 30): 679.2453
// BH_fcn(R0 = 1000, h = 0.99, phi0 = 0.3, x = 30): 985.8921
// BH_fcn(R0 = 1000, h = 0.75, phi0 = 0.2, x = 40): 750
// BH_fcn(R0 = 1000, h = 0.99, phi0 = 0.2, x = 40): 990

 
namespace
{
  TEST(sr_berverton_holt_evaluate, use_multiple_inputs)
  {
      EXPECT_EQ(1.0, 1.0);
      
      fims::SRBevertonHolt<double> recruit1;
      recruit1.steep = 0.7500;
      recruit1.rzero = 1000.000;
      recruit1.phizero = 0.100;
      double spawners = 30.000;
      // # R code that generates true values for testing
      // BH_fcn <- function(R0, h, phi0, x) {
      //  # R0 = unfished recruitment
      //  # h = steepness
      //  # phi0 = unfished spawners per recruit
      //  # x = spawners
      //  recruits <- (0.8 * R0 * h * x) / (0.2 * R0 * phi0 * (1.0 - h) + x * (h //  - 0.2))
      //  return(recruits)
      // }
      // (0.8 * 1000.0 * 0.75 * 30.0) / (0.2 * 1000.0 * 0.1 * (1.0 - 0.75) + 30.0 * (0.75 - 0.2)) = 837.2093
      double expect_recruit1 = 837.209300;
      EXPECT_NEAR(recruit1.evaluate(spawners), expect_recruit1, 0.0001);
      EXPECT_EQ(recruit1.GetId(), 0);
      
  }
}
