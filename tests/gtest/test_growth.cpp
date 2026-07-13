#include "gtest/gtest.h"
#include "population_dynamics/growth/functors/ewaa.hpp"

namespace
{
  TEST(GrowthEvaluate, IntegerAgeInput)
  {
    // empirical weight-at-age values from the model comparison project
    // (via the Rdata object being used by the data group)
    // ewaa(a = 0): 0.0
    // ewaa(a = 1): 0.0005306555
    // ewaa(a = 2): 0.0011963283

    // create a new ewaa singleton class
    fims_popdy::EWAAGrowth<double> ewaa1;
    // set the ewaa values using an initializer list
    // std::pair is a class template that provides a way to store two heterogeneous objects as a single unit
    ewaa1.ewaa =
        std::map<double, double>{std::pair<double, double>(0.0, 0.0),
                                 std::pair<double, double>(1.0, 0.005306555),
                                 std::pair<double, double>(2.0, 0.0011963283)};
    // set the expected values
    std::map<double, double> expect_ewaa0 = std::map<double, double>{std::pair<double, double>(0.0, 0.0),
                                                                     std::pair<double, double>(1.0, 0.005306555),
                                                                     std::pair<double, double>(2.0, 0.0011963283)};
    // test the values at ages 0, 1, and 2
    EXPECT_EQ(ewaa1.evaluate(0), expect_ewaa0[0]);

    // test that the id of the singleton class is set correctly
    EXPECT_EQ(ewaa1.GetId(), 0);
  }

  TEST(GrowthEvaluate, DoubleAgeInput)
  {
    // create a new ewaa singleton class
    fims_popdy::EWAAGrowth<double> ewaa2;
    // set the ewaa values
    ewaa2.ewaa =
        std::map<double, double>{std::pair<double, double>(0.0, 0.0),
                                 std::pair<double, double>(1.0, 0.005306555),
                                 std::pair<double, double>(2.0, 0.0011963283)};

    std::map<double, double> expect_ewaa2 =
        {std::pair<double, double>(0.0, 0.0),
         std::pair<double, double>(1.0, 0.005306555),
         std::pair<double, double>(2.0, 0.0011963283)};
    // test the values at ages 1.5, which isn't yet implemented
    // so should fail
    EXPECT_EQ(ewaa2.evaluate(1.5), 0.0);
    // test that the id of the singleton class is set correctly
    // this is zero because we are running it in a different test case than above
    EXPECT_EQ(ewaa2.GetId(), 0);
  }
}
