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

    double W5 = vb.evaluate(0, 5.0);
    EXPECT_NEAR(W5, 2.5e-11 * std::pow(L5, 3.0), 1e-10);

    fims_popdy::VonBertalanffyGrowth<double> vb2;
    EXPECT_EQ(vb2.GetId(), vb.GetId() + 1);
  }

  TEST(VonBertalanffyEvaluate, UsesLinearRampBelowFirstReferenceAge)
  {
    auto vb = MakeValidVonB();

    // This setup reproduces the age-0 back-calculation problem:
    // A1 is above the youngest model age, so age 0 is below the first
    // reference age.
    vb.length_at_ref_age_1 = 6.0;
    vb.length_at_ref_age_2 = 67.1;
    vb.growth_coefficient_K = 0.242;
    vb.reference_age_for_length_1 = 1.0;
    vb.reference_age_for_length_2 = 21.0;

    // Use a non-integer length-weight exponent because negative length raised
    // to a non-integer power is the pathway that can produce NaN.
    vb.length_weight_a = 1.75e-8;
    vb.length_weight_b = 2.99;

    // Ages below A1 should now use the early-age linear ramp rather than
    // back-extrapolating the curved growth equation.
    EXPECT_NEAR(vb.length_at_age(0.0), 0.0, 1e-12);
    EXPECT_NEAR(vb.length_at_age(0.5), 3.0, 1e-12);
    EXPECT_NEAR(vb.length_at_age(1.0), 6.0, 1e-12);
    EXPECT_GT(vb.length_at_age(2.0), vb.length_at_ref_age_1);

    // The key failure mode was NaN weight-at-age at age 0. This should now be
    // finite and non-negative.
    const double W0 = vb.weight_at_age(0.0);
    const double W_half = vb.weight_at_age(0.5);

    EXPECT_TRUE(std::isfinite(W0));
    EXPECT_TRUE(std::isfinite(W_half));
    EXPECT_GE(W0, 0.0);
    EXPECT_GE(W_half, 0.0);
    EXPECT_NEAR(
        W_half, vb.length_weight_a * std::pow(3.0, vb.length_weight_b), 1e-12);

    // The log-length gradient follows the early-age ramp below A1.
    // At age 0.5, mean length is 3 and dL/dL1 is 0.5. The implementation uses
    // the same smooth safety floor as other log-length calculations, so compute
    // the expected value with that same safe mean length.
    double d_log_laa_d_l1 = 0.0;
    double d_log_laa_d_l2 = 0.0;
    double d_log_laa_d_k = 0.0;

    vb.log_length_at_age_gradient(
        0.5, d_log_laa_d_l1, d_log_laa_d_l2, d_log_laa_d_k);

    const double ratio_half = 0.5 / vb.reference_age_for_length_1;
    const double mean_length_half = vb.length_at_ref_age_1 * ratio_half;
    const double mean_length_half_safe =
        fims_math::ad_max(mean_length_half, 1e-8);

    EXPECT_NEAR(d_log_laa_d_l1, ratio_half / mean_length_half_safe, 1e-12);
    EXPECT_NEAR(d_log_laa_d_l2, 0.0, 1e-12);
    EXPECT_NEAR(d_log_laa_d_k, 0.0, 1e-12);

    // On the log scale, multiply the natural-scale derivative by L1.
    double d_log_laa_d_log_l1 = 0.0;
    double d_log_laa_d_log_l2 = 0.0;
    double d_log_laa_d_log_k = 0.0;

    vb.log_length_at_age_logscale_gradient(
        0.5,
        d_log_laa_d_log_l1,
        d_log_laa_d_log_l2,
        d_log_laa_d_log_k);

    EXPECT_NEAR(
        d_log_laa_d_log_l1,
        vb.length_at_ref_age_1 * ratio_half / mean_length_half_safe,
        1e-12);
    EXPECT_NEAR(d_log_laa_d_log_l2, 0.0, 1e-12);
    EXPECT_NEAR(d_log_laa_d_log_k, 0.0, 1e-12);
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
