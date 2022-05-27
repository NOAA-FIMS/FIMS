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
      fims::EWAAgrowth<double> ewaa1;
      // set the ewaa values 
      ewaa1.ewaa = std::vector<double> {0.0, 0.005306555, 0.0011963283};
      // set the expected values
      std::vector<double> expect_ewaa0 = {0.0, 0.005306555,
       0.0011963283};
      // test the values at ages 0, 1, and 2
      EXPECT_EQ(ewaa1.evaluate(0), expect_ewaa0[0]);

      // test that the id of the singleton class is set correctly
      EXPECT_EQ(ewaa1.GetId(), 0);
  }

  TEST(growth_evaluate, double_age_input)
  {
      // create a new ewaa singleton class
      fims::EWAAgrowth<double> ewaa2;
      // set the ewaa values 
      ewaa2.ewaa = std::vector<double> {0.0, 0.005306555, 0.0011963283};
      std::vector<double> expect_ewaa2 = {0.0, 0.005306555,
       0.0011963283};
      // test the values at ages 1.5, which isn't yet implemented
      // so should fail
      EXPECT_EQ(ewaa2.evaluate(1.5), expect_ewaa2[1]);
      // test that the id of the singleton class is set correctly
      //this is zero because we are running it in a different test case than above
      EXPECT_EQ(ewaa2.GetId(), 0);
  }
}
