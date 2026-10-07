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
  const double reference_age_for_length_young = 1.0;
  const double reference_age_for_length_old = 12.0;
  const double denom_raw =
      1.0 - std::exp(-growth_coefficient * (reference_age_for_length_old -
                                            reference_age_for_length_young));
  const double denom = fims_math::ad_max(fims_math::ad_fabs(denom_raw), 1e-8);
  const double expected_length_at_age_1 =
      mean_length_young +
      (mean_length_old - mean_length_young) *
          (1.0 - std::exp(-growth_coefficient *
                          (1.0 - reference_age_for_length_young))) /
          denom;
  const double expected_length_at_age_5 =
      mean_length_young +
      (mean_length_old - mean_length_young) *
          (1.0 - std::exp(-growth_coefficient *
                          (age - reference_age_for_length_young))) /
          denom;
  const double expected_length_at_age_8 =
      mean_length_young +
      (mean_length_old - mean_length_young) *
          (1.0 - std::exp(-growth_coefficient *
                          (8.0 - reference_age_for_length_young))) /
          denom;
  const double expected_length_at_age_12 =
      mean_length_young +
      (mean_length_old - mean_length_young) *
          (1.0 - std::exp(-growth_coefficient *
                          (12.0 - reference_age_for_length_young))) /
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

TEST(GrowthModel,
     TraditionalVonBertalanffyUsesSharedInterpolationProducts) {
  fims_popdy::GrowthModel<double> gm(1, 5, 1);

  const double asymptotic_length = 100.0;
  const double growth_coefficient = 0.2;
  const double age_at_zero_length = -0.5;

  gm.SetVonBertalanffyTraditionalParameters(
      asymptotic_length, growth_coefficient, age_at_zero_length);
  gm.SetLengthWeightParameters(1.0e-5, 3.0);
  gm.SetLengthSdReferenceAges(1.0, 5.0);
  gm.SetLengthSdParams(10.0, 20.0);
  gm.SetAgeOffset(1.0);

  gm.Prepare();

  const auto& p = gm.GetProducts();

  const auto expected_length = [&](double age) {
    return asymptotic_length *
           (1.0 -
            std::exp(-growth_coefficient * (age - age_at_zero_length)));
  };

  const double expected_length_age_1 = expected_length(1.0);
  const double expected_length_age_3 = expected_length(3.0);
  const double expected_length_age_5 = expected_length(5.0);

  EXPECT_NEAR(p.MeanLAA(0, 0, 0), expected_length_age_1, 1e-8);
  EXPECT_NEAR(p.MeanLAA(0, 2, 0), expected_length_age_3, 1e-8);
  EXPECT_NEAR(p.MeanLAA(0, 4, 0), expected_length_age_5, 1e-8);

  const double expected_sd_age_3 =
      10.0 + (20.0 - 10.0) *
                 (expected_length_age_3 - expected_length_age_1) /
                 (expected_length_age_5 - expected_length_age_1);

  EXPECT_NEAR(p.SdLAA(0, 0, 0), fims_math::ad_max(10.0, 1e-8), 1e-7);
  EXPECT_NEAR(p.SdLAA(0, 2, 0), fims_math::ad_max(expected_sd_age_3, 1e-8),
              1e-7);
  EXPECT_NEAR(p.SdLAA(0, 4, 0), fims_math::ad_max(20.0, 1e-8), 1e-7);

  EXPECT_NEAR(p.MeanWAA(0, 2, 0),
              1.0e-5 * std::pow(expected_length_age_3, 3.0), 1e-8);
}

TEST(GrowthModel, TraditionalDeltaMethodAllowsLowCv) {
  fims_popdy::GrowthModel<double> gm(1, 5, 1);

  gm.SetVonBertalanffyTraditionalParameters(100.0, 0.2, -0.5);
  gm.SetLengthWeightParameters(1.0e-5, 3.0);
  gm.SetTraditionalGrowthParameterCovariance(
      0.03 * 0.03,  // var(log(Linf))
      0.0, 0.0,
      0.0,  // var(log(K))
      0.0,
      0.0);  // var(t0)
  gm.SetAgeOffset(1.0);

  gm.Prepare();

  const auto& p = gm.GetProducts();
  const double cv_at_age_3 = p.SdLAA(0, 2, 0) / p.MeanLAA(0, 2, 0);

  EXPECT_NEAR(cv_at_age_3, 0.03, 1e-5);
  EXPECT_LT(cv_at_age_3, 0.04);
}

TEST(GrowthModel, TraditionalDeltaMethodIncludesT0Covariances) {
  fims_popdy::GrowthModel<double> gm(1, 5, 1);

  const double asymptotic_length = 100.0;
  const double growth_coefficient = 0.2;
  const double age_at_zero_length = -0.5;

  const double sd_log_asymptotic_length = 0.05;
  const double sd_log_growth_coefficient = 0.10;
  const double sd_age_at_zero_length = 0.20;

  const double corr_linf_k = -0.5;
  const double corr_linf_t0 = 0.25;
  const double corr_k_t0 = -0.3;

  const double var_log_asymptotic_length =
      sd_log_asymptotic_length * sd_log_asymptotic_length;
  const double var_log_growth_coefficient =
      sd_log_growth_coefficient * sd_log_growth_coefficient;
  const double var_age_at_zero_length =
      sd_age_at_zero_length * sd_age_at_zero_length;

  const double cov_linf_k =
      corr_linf_k * sd_log_asymptotic_length * sd_log_growth_coefficient;
  const double cov_linf_t0 =
      corr_linf_t0 * sd_log_asymptotic_length * sd_age_at_zero_length;
  const double cov_k_t0 =
      corr_k_t0 * sd_log_growth_coefficient * sd_age_at_zero_length;

  gm.SetVonBertalanffyTraditionalParameters(
      asymptotic_length, growth_coefficient, age_at_zero_length);
  gm.SetLengthWeightParameters(1.0e-5, 3.0);
  gm.SetTraditionalGrowthParameterCovariance(
      var_log_asymptotic_length, cov_linf_k, cov_linf_t0,
      var_log_growth_coefficient, cov_k_t0, var_age_at_zero_length);
  gm.SetAgeOffset(1.0);

  gm.Prepare();

  const auto& p = gm.GetProducts();

  const double age = 3.0;

  fims_popdy::VonBertalanffyTraditionalGrowth<double> vb;
  vb.asymptotic_length = asymptotic_length;
  vb.growth_coefficient = growth_coefficient;
  vb.age_at_zero_length = age_at_zero_length;

  double d_log_laa_d_log_asymptotic_length = 0.0;
  double d_log_laa_d_log_growth_coefficient = 0.0;
  double d_log_laa_d_age_at_zero_length = 0.0;

  vb.log_length_at_age_working_scale_gradient(
      age, d_log_laa_d_log_asymptotic_length,
      d_log_laa_d_log_growth_coefficient,
      d_log_laa_d_age_at_zero_length);

  const double expected_log_var =
      d_log_laa_d_log_asymptotic_length *
          d_log_laa_d_log_asymptotic_length * var_log_asymptotic_length +
      2.0 * d_log_laa_d_log_asymptotic_length *
          d_log_laa_d_log_growth_coefficient * cov_linf_k +
      2.0 * d_log_laa_d_log_asymptotic_length *
          d_log_laa_d_age_at_zero_length * cov_linf_t0 +
      d_log_laa_d_log_growth_coefficient *
          d_log_laa_d_log_growth_coefficient * var_log_growth_coefficient +
      2.0 * d_log_laa_d_log_growth_coefficient *
          d_log_laa_d_age_at_zero_length * cov_k_t0 +
      d_log_laa_d_age_at_zero_length * d_log_laa_d_age_at_zero_length *
          var_age_at_zero_length;

  const double expected_sd_raw =
      vb.length_at_age(age) *
      std::sqrt(fims_math::ad_max(expected_log_var, 0.0, 1e-12));
  const double expected_sd = fims_math::ad_max(expected_sd_raw, 1e-8);

  EXPECT_NEAR(p.SdLAA(0, 2, 0), expected_sd, 1e-7);
}

TEST(GrowthModel, UsesDeltaMethodVariabilityAtReferenceAges) {
  fims_popdy::GrowthModel<double> gm(1, 12, 1);

  const double mean_length_young = 275.0;
  const double mean_length_old = 725.0;
  const double growth_coefficient = 0.18;
  const double reference_age_for_length_young = 1.0;
  const double reference_age_for_length_old = 12.0;

  gm.SetVonBertalanffySchnuteParameters(
      mean_length_young, mean_length_old, growth_coefficient,
      reference_age_for_length_young, reference_age_for_length_old);
  gm.SetLengthWeightParameters(2.5e-11, 3.0);
  gm.SetGrowthParameterCovariance(
      0.01,  // var(log(mean_length_young))) = 0.1^2
      0.0, 0.0,
      0.01,  // var(log(mean_length_old))) = 0.1^2
      0.0,
      0.01);  // var(log(growth_coefficient))) = 0.1^2
  gm.SetAgeOffset(1.0);

  gm.Prepare();

  const auto& p = gm.GetProducts();

  fims_popdy::VonBertalanffySchnuteGrowth<double> vb;
  vb.mean_length_young = mean_length_young;
  vb.mean_length_old = mean_length_old;
  vb.growth_coefficient = growth_coefficient;
  vb.reference_age_for_length_young = reference_age_for_length_young;
  vb.reference_age_for_length_old = reference_age_for_length_old;

  const double age_1 = 1.0;
  const double age_12 = 12.0;

  const double expected_length_at_age_1 = vb.length_at_age(age_1);
  const double expected_length_at_age_12 = vb.length_at_age(age_12);

  EXPECT_NEAR(p.MeanLAA(0, 0, 0), expected_length_at_age_1, 1e-8);
  EXPECT_NEAR(p.MeanLAA(0, 11, 0), expected_length_at_age_12, 1e-8);

  double d_log_laa_d_log_length_young = 0.0;
  double d_log_laa_d_log_length_old = 0.0;
  double d_log_laa_d_log_k = 0.0;

  vb.log_length_at_age_logscale_gradient(age_1, d_log_laa_d_log_length_young,
                                         d_log_laa_d_log_length_old,
                                         d_log_laa_d_log_k);
  const double expected_log_var_age_1 =
      d_log_laa_d_log_length_young * d_log_laa_d_log_length_young * 0.01 +
      d_log_laa_d_log_length_old * d_log_laa_d_log_length_old * 0.01 +
      d_log_laa_d_log_k * d_log_laa_d_log_k * 0.01;
  const double expected_log_var_age_1_safe =
      fims_math::ad_max(expected_log_var_age_1, 0.0);
  const double expected_sd_age_1 =
      expected_length_at_age_1 * std::sqrt(expected_log_var_age_1_safe);

  vb.log_length_at_age_logscale_gradient(age_12, d_log_laa_d_log_length_young,
                                         d_log_laa_d_log_length_old,
                                         d_log_laa_d_log_k);
  const double expected_log_var_age_12 =
      d_log_laa_d_log_length_young * d_log_laa_d_log_length_young * 0.01 +
      d_log_laa_d_log_length_old * d_log_laa_d_log_length_old * 0.01 +
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
