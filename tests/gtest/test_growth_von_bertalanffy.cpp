#include <cmath>
#include "gtest/gtest.h"
#include "common/fims_math.hpp"
#include "population_dynamics/growth/functors/von_bertalanffy.hpp"

namespace
{
  fims_popdy::VonBertalanffyGrowth<double> MakeValidVonB() {
    fims_popdy::VonBertalanffyGrowth<double> vb;

    vb.length_at_ref_age_1 = 275.0;
    vb.length_at_ref_age_2 = 725.0;
    vb.growth_coefficient_K = 0.18;
    vb.reference_age_for_length_1 = 1.0;
    vb.reference_age_for_length_2 = 12.0;
    vb.length_weight_a = 2.5e-11;
    vb.length_weight_b = 3.0;

    return vb;
  }

  TEST(VonBertalanffyEvaluate, BasicSanity)
  {
    auto vb = MakeValidVonB();

    // expected values from the formula (guarded denominator)
    const double denom_raw = 1.0 - std::exp(
        -vb.growth_coefficient_K *
        (vb.reference_age_for_length_2 - vb.reference_age_for_length_1));
    const double denom = fims_math::ad_max(fims_math::ad_fabs(denom_raw), 1e-8);

    double L1 = vb.length_at_age(1.0);
    double L5 = vb.length_at_age(5.0);
    double L12 = vb.length_at_age(12.0);

    const double L1_expected =
        vb.length_at_ref_age_1 +
        (vb.length_at_ref_age_2 - vb.length_at_ref_age_1) *
            (1.0 - std::exp(-vb.growth_coefficient_K *
                            (1.0 - vb.reference_age_for_length_1))) /
            denom;
    const double L12_expected =
        vb.length_at_ref_age_1 +
        (vb.length_at_ref_age_2 - vb.length_at_ref_age_1) *
            (1.0 - std::exp(-vb.growth_coefficient_K *
                            (12.0 - vb.reference_age_for_length_1))) /
            denom;

    // sanity checks
    EXPECT_NEAR(L1, L1_expected, 1e-12);
    EXPECT_GT(L5, 275.0);
    EXPECT_LT(L5, 725.0);
    EXPECT_NEAR(L12, L12_expected, 1e-12);

    double W5 = vb.evaluate(5.0);
    EXPECT_NEAR(W5, 2.5e-11 * std::pow(L5, 3.0), 1e-10);

    // id should start at 0 for a fresh test TU
    EXPECT_EQ(vb.GetId(), 0);
  }

  TEST(VonBertalanffyEvaluate, RejectsNonPositiveGrowthCoefficientK)
  {
    auto vb = MakeValidVonB();

    // Make just this one parameter invalid.
    vb.growth_coefficient_K = 0.0;

    // The backend validator should now reject this.
    EXPECT_THROW(vb.length_at_age(1.0), std::runtime_error);
  }

  TEST(VonBertalanffyEvaluate, RejectsReversedReferenceAges)
  {
    auto vb = MakeValidVonB();

    // Make the second reference age come before the first.
    vb.reference_age_for_length_2 = vb.reference_age_for_length_1;

    EXPECT_THROW(vb.length_at_age(1.0), std::runtime_error);
  }

  TEST(VonBertalanffyEvaluate, RejectsNonIncreasingReferenceLengths)
  {
    auto vb = MakeValidVonB();

    // Make the second reference length no larger than the first.
    vb.length_at_ref_age_2 = vb.length_at_ref_age_1;

    EXPECT_THROW(vb.length_at_age(1.0), std::runtime_error);
  }
}
