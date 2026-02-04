#include "gtest/gtest.h"
#include "population_dynamics/recruitment/functors/sr_beverton_holt.hpp"

 
namespace
{
  TEST(SrBevertonHoltEvaluate, UseMultipleInputs)
  {
// BH_fcn(R0 = 1000, h = 0.75, phi0 = 0.1, x = 30): 837.2093 
// BH_fcn(R0 = 1000, h = 0.99, phi0 = 0.1, x = 30): 994.1423
// BH_fcn(R0 = 1000, h = 0.75, phi0 = 0.3, x = 30): 679.2453
// BH_fcn(R0 = 1000, h = 0.99, phi0 = 0.3, x = 30): 985.8921
// BH_fcn(R0 = 1000, h = 0.2, phi0 = 0.2, x = 40): 200
// BH_fcn(R0 = 1000, h = 0.99, phi0 = 0.2, x = 40): 990

      fims_popdy::SRBevertonHolt<double> recruit1;
      recruit1.logit_steep.resize(1);
      recruit1.logit_steep[0] = fims_math::logit(0.2, 1.0, 0.7500);
      // The R0 value (1 thousand) here is for this unit test.
      // It is different than the Model Comparison Project value (1 million). 
      recruit1.log_rzero.resize(1);
      recruit1.log_rzero[0] = std::log(1000.000);
      double spawners = 30.000;
      double phi_0 = 0.1;
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
      EXPECT_NEAR(recruit1.evaluate_mean(spawners,phi_0), expect_recruit1, 0.0001);
      EXPECT_EQ(recruit1.GetId(), 0);
      EXPECT_EQ(recruit1.evaluate_process(0), 0);
      
      fims_popdy::SRBevertonHolt<double> recruit2;
      recruit2.logit_steep.resize(1);
      recruit2.logit_steep[0] = fims_math::logit(0.2, 1.0, 0.200);
      recruit2.log_rzero.resize(1);
      recruit2.log_rzero[0] = std::log(1000.000);
      double spawners2 = 40.000;
      double phi_02 = 0.2;
      // # R code that generates true values for testing
      // BH_fcn <- function(R0, h, phi0, x) {
      //  # R0 = unfished recruitment
      //  # h = steepness
      //  # phi0 = unfished spawners per recruit
      //  # x = spawners
      //  recruits <- (0.8 * R0 * h * x) / (0.2 * ssb0 * (1.0 - h) + x * (h //  - 0.2))
      //  return(recruits)
      // }
      
      double expect_recruit2 = 200.0;
      EXPECT_NEAR(recruit2.evaluate_mean(spawners2, phi_02), expect_recruit2, 0.0001);
      EXPECT_EQ(recruit2.GetId(), 1);
      EXPECT_EQ(recruit2.evaluate_process(0), 0);
  }

}
