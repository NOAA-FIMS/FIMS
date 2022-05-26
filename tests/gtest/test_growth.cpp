#include "gtest/gtest.h"
#include "population_dynamics/growth/functors/ewaa.hpp"

 
namespace
{
  TEST(growth_evaluate, integer_age_input)
  {
// empirical weight-at-age values from the model comparison project
// (via the Rdata object being used by the data group)
// ewaa(a = 0): 0.0
// ewaa(a = 1): 0.0005306555
// ewaa(a = 2): 0.0011963283 

      // not sure why this dummy test is here
      EXPECT_EQ(1.0, 1.0);
      
      // create a new ewaa singleton class
      fims::EWAAgrowth() ewaa1;
      // set the ewaa values 
      ewaa1.ewaa = [0.0, 0.005306555, 0.0011963283];
      // set the expected values
      double expect_ewaa0 = 0.0;
      double expect_ewaa1 = 0.005306555;
      double expect_ewaa2 = 0.0011963283;
      // test the values at ages 0, 1, and 2
      EXPECT_EQ(ewaa1.evaluate(0), expect_ewaa0);
      EXPECT_EQ(ewaa1.evaluate(1), expect_ewaa1);
      EXPECT_EQ(ewaa1.evaluate(2), expect_ewaa2);
      // test that the id of the singleton class is set correctly
      EXPECT_EQ(ewaa1.GetId(), 0);
  }

  TEST(growth_evaluate, double_age_input)
  {
      // create a new ewaa singleton class
      fims::EWAAgrowth() ewaa2;
      // set the ewaa values 
      ewaa2.ewaa = [0.0, 0.005306555, 0.0011963283];
      // test the values at ages 1.5, which isn't yet implemented
      // so should fail
      FAIL(ewaa2.evaluate(1.5));
      // test that the id of the singleton class is set correctly
      EXPECT_EQ(ewaa2.GetId(), 1);
  }
}
