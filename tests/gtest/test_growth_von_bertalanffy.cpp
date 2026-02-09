#include <cmath>
#include "gtest/gtest.h"
#include "population_dynamics/growth/functors/von_bertalanffy.hpp"

namespace
{
  TEST(VonBertalanffyEvaluate, BasicSanity)
  {
    fims_popdy::VonBertalanffyGrowth<double> vb;

    // set fixed parameters
    vb.L1 = 10.0;
    vb.L2 = 100.0;
    vb.K = 0.2;
    vb.age_L1 = 0.0;
    vb.age_L2 = 50.0;

    // expected values from the formula:
    // L(a) = L1 + (L2 - L1) * (1 - exp(-K * (a - A1))) /
    //        (1 - exp(-K * (A2 - A1)))
    double L0 = vb.length_at_age(0.0);
    double L5 = vb.length_at_age(5.0);
    double L50 = vb.length_at_age(50.0);

    // sanity checks
    EXPECT_NEAR(L0, 10.0, 1e-12);     // at A1, should equal L1
    EXPECT_GT(L5, 10.0);
    EXPECT_LT(L5, 100.0);
    EXPECT_NEAR(L50, 100.0, 1e-12);   // at A2, should equal L2

    // weight uses length-weight params and evaluate() returns waa
    vb.a_wl = 1e-5;
    vb.b_wl = 3.0;
    double W5 = vb.evaluate(5.0);
    EXPECT_NEAR(W5, 1e-5 * std::pow(L5, 3.0), 1e-8);

    // match existing pattern: id should start at 0 for a fresh test TU
    EXPECT_EQ(vb.GetId(), 0);
  }
}
