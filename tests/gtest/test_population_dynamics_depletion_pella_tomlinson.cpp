#include "gtest/gtest.h"
#include "population_dynamics/depletion/functors/pella_tomlinson.hpp"
#include "common/fims_math.hpp"

namespace
{
  TEST(DepletionEvaluate, DoubleInput){
    fims_popdy::PellaTomlinsonDepletion<double> pella_tomlinson1;
    pella_tomlinson1.r.resize(1);
    pella_tomlinson1.K.resize(1);
    pella_tomlinson1.m.resize(1);
    pella_tomlinson1.r[0] = 0.2;
    pella_tomlinson1.K[0] = 645.0;
    pella_tomlinson1.m[0] = 2.0;

    // Test values from the jabba comparison
    // (via the Rdata object being used by the data group)
    // depletion(t-1) = 0.88
    // catch(t-1) = 10
    // r = 0.2
    // K = 645
    // m = 2.0
    // expected depletion(t) = \f[ d_{t-1} + \frac{r}{m - 1.0} * d_{t-1} *  (1.0 - d_{t-1}^{m - 1.0} - Catch_{t-1} /
    // K \f]
    // = 0.88 + (0.2 / (2.0 - 1.0)) * 0.88 * (1.0 - 0.88^(2.0 - 1.0)) - 10 / 645
    // expected depletion(t) = 0.8856161

    double depletion_ym1 = 0.88;
    double catch_ym1 = 10.0;
    double expect_depletion_t = 0.8856161;

    EXPECT_NEAR(pella_tomlinson1.evaluate_mean(depletion_ym1, catch_ym1),
                expect_depletion_t, 1e-7);
    // test that the id of the singleton class is set correctly
    EXPECT_EQ(pella_tomlinson1.GetId(), 0);
  }
}