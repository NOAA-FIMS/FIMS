#include <cmath>
#include "gtest/gtest.h"
#include "common/fims_math.hpp"
#include "population_dynamics/growth/functors/von_bertalanffy.hpp"

namespace
{
  TEST(VonBertalanffyEvaluate, BasicSanity)
  {
    fims_popdy::VonBertalanffyGrowth<double> vb;

    // set fixed parameters
    vb.length_at_ref_age_1 = 10.0;
    vb.length_at_ref_age_2 = 100.0;
    vb.growth_coefficient_K = 0.2;
    vb.reference_age_for_length_1 = 0.0;
    vb.reference_age_for_length_2 = 50.0;

    // expected values from the formula (guarded denominator)
    const double denom_raw = 1.0 - std::exp(
        -vb.growth_coefficient_K *
        (vb.reference_age_for_length_2 - vb.reference_age_for_length_1));
    const double denom = fims_math::ad_max(fims_math::ad_fabs(denom_raw), 1e-8);

    double L0 = vb.length_at_age(0.0);
    double L5 = vb.length_at_age(5.0);
    double L50 = vb.length_at_age(50.0);

    const double L0_expected =
        vb.length_at_ref_age_1 +
        (vb.length_at_ref_age_2 - vb.length_at_ref_age_1) *
            (1.0 - std::exp(-vb.growth_coefficient_K *
                            (0.0 - vb.reference_age_for_length_1))) /
            denom;
    const double L50_expected =
        vb.length_at_ref_age_1 +
        (vb.length_at_ref_age_2 - vb.length_at_ref_age_1) *
            (1.0 - std::exp(-vb.growth_coefficient_K *
                            (50.0 - vb.reference_age_for_length_1))) /
            denom;

    // sanity checks
    EXPECT_NEAR(L0, L0_expected, 1e-12);
    EXPECT_GT(L5, 10.0);
    EXPECT_LT(L5, 100.0);
    EXPECT_NEAR(L50, L50_expected, 1e-12);

    // weight uses length-weight params and evaluate() returns waa
    vb.length_weight_a = 1e-5;
    vb.length_weight_b = 3.0;
    double W5 = vb.evaluate(5.0);
    EXPECT_NEAR(W5, 1e-5 * std::pow(L5, 3.0), 1e-8);

    // match existing pattern: id should start at 0 for a fresh test TU
    EXPECT_EQ(vb.GetId(), 0);
  }
}
