#include <cmath>
#include "gtest/gtest.h"
#include "common/fims_math.hpp"
#include "population_dynamics/growth/growth_model.hpp"

namespace {

TEST(GrowthModel, CanConstructAndPrepare) {
  // years=1, ages=12 (1..12), sexes=1
  fims_popdy::GrowthModel<double> gm(1, 12, 1);

  gm.SetVonBertalanffySchnuteParameters(275.0, 725.0, 0.18, 1.0, 12.0);
  gm.SetLengthWeightParameters(2.5e-11, 3.0);
  gm.SetLengthSdParams(28.0, 73.0);
  gm.SetAgeOffset(1.0);

  gm.Prepare();

  const auto& p = gm.GetProducts();

  EXPECT_EQ(p.n_years, 1u);
  EXPECT_EQ(p.n_ages, 12u);
  EXPECT_EQ(p.n_sexes, 1u);

  // Check a couple of values exist and look sane
  const double age = 5.0;
  const double mean_length_young = 275.0;
  const double mean_length_old = 725.0;
  const double growth_coefficient = 0.18;
  const double reference_age_for_length_1 = 1.0;
  const double reference_age_for_length_2 = 12.0;
  const double denom_raw = 1.0 - std::exp(
      -growth_coefficient *
      (reference_age_for_length_2 - reference_age_for_length_1));
  const double denom = fims_math::ad_max(fims_math::ad_fabs(denom_raw), 1e-8);
  const double expected_length_at_age_1 =
      mean_length_young +
      (mean_length_old - mean_length_young) *
          (1.0 - std::exp(-growth_coefficient *
                          (1.0 - reference_age_for_length_1))) /
          denom;
  const double expected_length_at_age_5 =
      mean_length_young +
      (mean_length_old - mean_length_young) *
          (1.0 - std::exp(-growth_coefficient *
                          (age - reference_age_for_length_1))) /
          denom;
  const double expected_length_at_age_8 =
      mean_length_young +
      (mean_length_old - mean_length_young) *
          (1.0 - std::exp(-growth_coefficient *
                          (8.0 - reference_age_for_length_1))) /
          denom;
  const double expected_length_at_age_12 =
      mean_length_young +
      (mean_length_old - mean_length_young) *
          (1.0 - std::exp(-growth_coefficient *
                          (12.0 - reference_age_for_length_1))) /
          denom;
  const double observed_length_at_age_1 = p.MeanLAA(0, 0, 0);
  const double observed_length_at_age_12 = p.MeanLAA(0, 11, 0);

  EXPECT_NEAR(observed_length_at_age_1, expected_length_at_age_1, 1e-8);
  EXPECT_GT(expected_length_at_age_5, mean_length_young);
  EXPECT_LT(expected_length_at_age_5, mean_length_old);
  EXPECT_NEAR(observed_length_at_age_12, expected_length_at_age_12, 1e-8);
  EXPECT_NEAR(p.SdLAA(0, 0, 0), 28.0, 1e-7);
  EXPECT_NEAR(p.SdLAA(0, 11, 0), 73.0, 1e-7);
  // Length-based interpolation for SD
  const double expected_sd_mid =
      28.0 + (73.0 - 28.0) *
                (expected_length_at_age_8 - expected_length_at_age_1) /
                (expected_length_at_age_12 - expected_length_at_age_1);
  EXPECT_NEAR(p.SdLAA(0, 7, 0), expected_sd_mid, 1e-7);

  // weight exists in the contract (derived from length-weight)
  const double W5 = p.MeanWAA(0, 4, 0);
  EXPECT_GT(W5, 0.0);
  EXPECT_NEAR(W5, 2.5e-11 * std::pow(expected_length_at_age_5, 3.0), 1e-8);

}

TEST(GrowthModel, UsesDeltaMethodVariabilityAtReferenceAges) {
  fims_popdy::GrowthModel<double> gm(1, 12, 1);

  const double mean_length_young = 275.0;
  const double mean_length_old = 725.0;
  const double growth_coefficient = 0.18;
  const double reference_age_for_length_1 = 1.0;
  const double reference_age_for_length_2 = 12.0;

  gm.SetVonBertalanffySchnuteParameters(
      mean_length_young, mean_length_old, growth_coefficient,
      reference_age_for_length_1, reference_age_for_length_2);
  gm.SetLengthWeightParameters(2.5e-11, 3.0);
  gm.SetGrowthParameterCovariance(
      0.01,  // var(log(mean_length_young))) = 0.1^2
      0.0,
      0.0,
      0.01,  // var(log(mean_length_old))) = 0.1^2
      0.0,
      0.01); // var(log(growth_coefficient))) = 0.1^2
  gm.SetAgeOffset(1.0);

  gm.Prepare();

  const auto& p = gm.GetProducts();

  fims_popdy::VonBertalanffySchnuteGrowth<double> vb;
  vb.mean_length_young = mean_length_young;
  vb.mean_length_old = mean_length_old;
  vb.growth_coefficient = growth_coefficient;
  vb.reference_age_for_length_1 = reference_age_for_length_1;
  vb.reference_age_for_length_2 = reference_age_for_length_2;

  const double age_1 = 1.0;
  const double age_12 = 12.0;

  const double expected_length_at_age_1 = vb.length_at_age(age_1);
  const double expected_length_at_age_12 = vb.length_at_age(age_12);

  EXPECT_NEAR(p.MeanLAA(0, 0, 0), expected_length_at_age_1, 1e-8);
  EXPECT_NEAR(p.MeanLAA(0, 11, 0), expected_length_at_age_12, 1e-8);

  double d_log_laa_d_log_l1 = 0.0;
  double d_log_laa_d_log_l2 = 0.0;
  double d_log_laa_d_log_k = 0.0;

  vb.log_length_at_age_logscale_gradient(
      age_1, d_log_laa_d_log_l1, d_log_laa_d_log_l2, d_log_laa_d_log_k);
  const double expected_log_var_age_1 =
      d_log_laa_d_log_l1 * d_log_laa_d_log_l1 * 0.01 +
      d_log_laa_d_log_l2 * d_log_laa_d_log_l2 * 0.01 +
      d_log_laa_d_log_k * d_log_laa_d_log_k * 0.01;
  const double expected_log_var_age_1_safe =
      fims_math::ad_max(expected_log_var_age_1, 0.0);
  const double expected_sd_age_1 =
      expected_length_at_age_1 * std::sqrt(expected_log_var_age_1_safe);

  vb.log_length_at_age_logscale_gradient(
      age_12, d_log_laa_d_log_l1, d_log_laa_d_log_l2, d_log_laa_d_log_k);
  const double expected_log_var_age_12 =
      d_log_laa_d_log_l1 * d_log_laa_d_log_l1 * 0.01 +
      d_log_laa_d_log_l2 * d_log_laa_d_log_l2 * 0.01 +
      d_log_laa_d_log_k * d_log_laa_d_log_k * 0.01;
  const double expected_log_var_age_12_safe =
      fims_math::ad_max(expected_log_var_age_12, 0.0);
  const double expected_sd_age_12 =
      expected_length_at_age_12 * std::sqrt(expected_log_var_age_12_safe);

// Allow tiny floating-point differences from guarded AD-safe math.
  EXPECT_NEAR(p.SdLAA(0, 0, 0), expected_sd_age_1, 1e-7);
  EXPECT_NEAR(p.SdLAA(0, 11, 0), expected_sd_age_12, 1e-7);

  // Mid-curve SD should also remain finite and positive.
  EXPECT_TRUE(std::isfinite(p.SdLAA(0, 7, 0)));
  EXPECT_GT(p.SdLAA(0, 7, 0), 0.0);
}

TEST(GrowthModel, ZeroAgesThrows) {
  fims_popdy::GrowthModel<double> gm(1, 0, 1);
  gm.SetVonBertalanffySchnuteParameters(275.0, 725.0, 0.18, 1.0, 12.0);
  gm.SetLengthWeightParameters(2.5e-11, 3.0);
  gm.SetLengthSdParams(28.0, 73.0);

  EXPECT_THROW(gm.Prepare(), std::runtime_error);
}

TEST(GrowthModel, RejectsCoincidentReferenceAges) {
  fims_popdy::GrowthModel<double> gm(1, 3, 1);
  gm.SetVonBertalanffySchnuteParameters(275.0, 725.0, 0.18, 5.0, 5.0);
  gm.SetLengthWeightParameters(2.5e-11, 3.0);
  gm.SetLengthSdParams(28.0, 73.0);

  EXPECT_THROW(gm.Prepare(), std::runtime_error);
}

TEST(GrowthModel, HandlesNegativeSdInputsWithoutThrow) {
  fims_popdy::GrowthModel<double> gm(1, 2, 1);
  gm.SetVonBertalanffySchnuteParameters(275.0, 725.0, 0.18, 1.0, 12.0);
  gm.SetLengthWeightParameters(2.5e-11, 3.0);
  gm.SetLengthSdParams(-1.0, 73.0);

  EXPECT_NO_THROW(gm.Prepare());
  const auto& p = gm.GetProducts();
  EXPECT_TRUE(std::isfinite(p.SdLAA(0, 0, 0)));
}

TEST(GrowthModel, SingleAgeRejectsCoincidentReferenceAges) {
  fims_popdy::GrowthModel<double> gm(1, 1, 1);
  // Coincident reference ages are invalid even with one modeled age.
  gm.SetVonBertalanffySchnuteParameters(275.0, 725.0, 0.18, 1.0, 1.0);
  gm.SetLengthWeightParameters(2.5e-11, 3.0);
  gm.SetLengthSdParams(28.0, 73.0);

  EXPECT_THROW(gm.Prepare(), std::runtime_error);
}

}  // namespace
