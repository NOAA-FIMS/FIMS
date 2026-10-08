#include <cmath>
#include "gtest/gtest.h"
#include "common/fims_math.hpp"
#include "population_dynamics/growth/growth_model_adapter.hpp"

namespace {

void ConfigureAdapter(
    fims_popdy::VonBertalanffySchnuteGrowthModelAdapter<double>& adapter,
    double mean_length_young, double mean_length_old, double growth_coefficient,
    double reference_age_for_length_young, double reference_age_for_length_old,
    double length_weight_a, double length_weight_b,
    double length_at_age_sd_at_reference_age_young,
    double length_at_age_sd_at_reference_age_old) {
  adapter.MeanLengthYoungVector().resize(1);
  adapter.MeanLengthOldVector().resize(1);
  adapter.GrowthCoefficientVector().resize(1);
  adapter.ReferenceAgeForLengthYoungVector().resize(1);
  adapter.ReferenceAgeForLengthOldVector().resize(1);
  adapter.LengthWeightAVector().resize(1);
  adapter.LengthWeightBVector().resize(1);
  adapter.LengthAtAgeSdAtRefAgesVector().resize(2);

  // Adapter stores positive growth params on log scale.
  adapter.MeanLengthYoungVector()[0] = fims_math::log(mean_length_young);
  adapter.MeanLengthOldVector()[0] = fims_math::log(mean_length_old);
  adapter.GrowthCoefficientVector()[0] = fims_math::log(growth_coefficient);
  adapter.ReferenceAgeForLengthYoungVector()[0] =
      reference_age_for_length_young;
  adapter.ReferenceAgeForLengthOldVector()[0] = reference_age_for_length_old;
  adapter.LengthWeightAVector()[0] = fims_math::log(length_weight_a);
  adapter.LengthWeightBVector()[0] = fims_math::log(length_weight_b);
  adapter.LengthAtAgeSdAtRefAgesVector()[0] =
      fims_math::log(length_at_age_sd_at_reference_age_young);
  adapter.LengthAtAgeSdAtRefAgesVector()[1] =
      fims_math::log(length_at_age_sd_at_reference_age_old);
}

void ConfigureTraditionalAdapter(
    fims_popdy::VonBertalanffyGrowthModelAdapter<double>& adapter,
    double asymptotic_length, double growth_coefficient,
    double age_at_zero_length, double reference_age_for_length_young,
    double reference_age_for_length_old, double length_weight_a,
    double length_weight_b, double length_at_age_sd_at_reference_age_young,
    double length_at_age_sd_at_reference_age_old) {
  adapter.UseTraditionalVonBertalanffy();
  adapter.AsymptoticLengthVector().resize(1);
  adapter.GrowthCoefficientVector().resize(1);
  adapter.AgeAtZeroLengthVector().resize(1);
  adapter.ReferenceAgeForLengthYoungVector().resize(1);
  adapter.ReferenceAgeForLengthOldVector().resize(1);
  adapter.LengthWeightAVector().resize(1);
  adapter.LengthWeightBVector().resize(1);
  adapter.LengthAtAgeSdAtRefAgesVector().resize(2);

  adapter.AsymptoticLengthVector()[0] = fims_math::log(asymptotic_length);
  adapter.GrowthCoefficientVector()[0] = fims_math::log(growth_coefficient);
  adapter.AgeAtZeroLengthVector()[0] = age_at_zero_length;
  adapter.ReferenceAgeForLengthYoungVector()[0] =
      reference_age_for_length_young;
  adapter.ReferenceAgeForLengthOldVector()[0] = reference_age_for_length_old;
  adapter.LengthWeightAVector()[0] = fims_math::log(length_weight_a);
  adapter.LengthWeightBVector()[0] = fims_math::log(length_weight_b);
  adapter.LengthAtAgeSdAtRefAgesVector()[0] =
      fims_math::log(length_at_age_sd_at_reference_age_young);
  adapter.LengthAtAgeSdAtRefAgesVector()[1] =
      fims_math::log(length_at_age_sd_at_reference_age_old);
}

TEST(VonBertalanffySchnuteGrowthModelAdapter, UsesWaaFromLaa) {
  fims_popdy::VonBertalanffySchnuteGrowthModelAdapter<double> adapter;
  ConfigureAdapter(adapter, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0,
                   73.0);
  adapter.SetAgeOffset(1.0);
  adapter.Initialize(1, 12, 1);

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
  const double L =
      mean_length_young +
      (mean_length_old - mean_length_young) *
          (1.0 - std::exp(-growth_coefficient *
                          (age - reference_age_for_length_young))) /
          denom;
  const double expected = 2.5e-11 * std::pow(L, 3.0);
  const double W = adapter.evaluate(0, age);

  EXPECT_NEAR(W, expected, 1e-8);
}

TEST(VonBertalanffySchnuteGrowthModelAdapter, HonorsAgeOffset) {
  fims_popdy::VonBertalanffySchnuteGrowthModelAdapter<double> adapter;
  // ages 1..12 (n_ages = 12), set reference ages to match
  ConfigureAdapter(adapter, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0,
                   73.0);
  adapter.SetAgeOffset(1.0);
  adapter.Initialize(1, 12, 1);

  const double age = 1.0;
  const double mean_length_young = 275.0;
  const double mean_length_old = 725.0;
  const double growth_coefficient = 0.18;
  const double reference_age_for_length_young = 1.0;
  const double reference_age_for_length_old = 12.0;
  const double denom_raw =
      1.0 - std::exp(-growth_coefficient * (reference_age_for_length_old -
                                            reference_age_for_length_young));
  const double denom = fims_math::ad_max(fims_math::ad_fabs(denom_raw), 1e-8);
  const double L =
      mean_length_young +
      (mean_length_old - mean_length_young) *
          (1.0 - std::exp(-growth_coefficient *
                          (age - reference_age_for_length_young))) /
          denom;
  const double expected = 2.5e-11 * std::pow(L, 3.0);
  const double W = adapter.evaluate(0, age);

  EXPECT_NEAR(W, expected, 1e-8);
}

TEST(VonBertalanffySchnuteGrowthModelAdapter, RejectsFractionalAge) {
  fims_popdy::VonBertalanffySchnuteGrowthModelAdapter<double> adapter;
  ConfigureAdapter(adapter, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0,
                   73.0);
  adapter.SetAgeOffset(1.0);
  adapter.Initialize(1, 12, 1);

  EXPECT_THROW(adapter.evaluate(0, 5.5), std::runtime_error);
}

TEST(VonBertalanffySchnuteGrowthModelAdapter, RejectsNegativeAge) {
  fims_popdy::VonBertalanffySchnuteGrowthModelAdapter<double> adapter;
  ConfigureAdapter(adapter, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0,
                   73.0);
  adapter.SetAgeOffset(1.0);
  adapter.Initialize(1, 12, 1);

  EXPECT_THROW(adapter.evaluate(0, -1.0), std::runtime_error);
}

TEST(VonBertalanffySchnuteGrowthModelAdapter, ExtrapolatesAboveCachedAgeRange) {
  fims_popdy::VonBertalanffySchnuteGrowthModelAdapter<double> adapter;
  ConfigureAdapter(adapter, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0,
                   73.0);
  adapter.SetAgeOffset(1.0);
  adapter.Initialize(1, 12, 1);

  const double age = 13.0;
  const double mean_length_young = 275.0;
  const double mean_length_old = 725.0;
  const double growth_coefficient = 0.18;
  const double reference_age_for_length_young = 1.0;
  const double reference_age_for_length_old = 12.0;
  const double denom_raw =
      1.0 - std::exp(-growth_coefficient * (reference_age_for_length_old -
                                            reference_age_for_length_young));
  const double denom = fims_math::ad_max(fims_math::ad_fabs(denom_raw), 1e-8);
  const double L =
      mean_length_young +
      (mean_length_old - mean_length_young) *
          (1.0 - std::exp(-growth_coefficient *
                          (age - reference_age_for_length_young))) /
          denom;
  const double expected = 2.5e-11 * std::pow(L, 3.0);

  const double W = adapter.evaluate(0, age);
  EXPECT_NEAR(W, expected, 1e-8);
}

TEST(VonBertalanffyGrowthModelAdapter,
     TraditionalVonBertalanffyUsesInterpolationProducts) {
  fims_popdy::VonBertalanffyGrowthModelAdapter<double> adapter;
  ConfigureTraditionalAdapter(adapter, 100.0, 0.2, -0.5, 1.0, 5.0, 1.0e-5, 3.0,
                              10.0, 20.0);
  adapter.SetAgeOffset(1.0);
  adapter.Initialize(1, 5, 1);
  adapter.PrepareGrowthProducts();

  const double age = 3.0;
  const double expected_length =
      100.0 * (1.0 - std::exp(-0.2 * (age - -0.5)));
  const double expected_weight = 1.0e-5 * std::pow(expected_length, 3.0);

  EXPECT_NEAR(adapter.evaluate(0, age), expected_weight, 1e-8);

  const auto* products = adapter.TryGetPreparedGrowthProducts();
  ASSERT_NE(products, nullptr);
  EXPECT_NEAR(products->MeanLAA(0, 2, 0), expected_length, 1e-8);
  EXPECT_NEAR(products->MeanWAA(0, 2, 0), expected_weight, 1e-8);
}

TEST(VonBertalanffyGrowthModelAdapter,
     TraditionalVonBertalanffyUsesDeltaMethodProducts) {
  fims_popdy::VonBertalanffyGrowthModelAdapter<double> adapter;
  adapter.UseTraditionalVonBertalanffy();

  adapter.AsymptoticLengthVector().resize(1);
  adapter.GrowthCoefficientVector().resize(1);
  adapter.AgeAtZeroLengthVector().resize(1);
  adapter.LengthWeightAVector().resize(1);
  adapter.LengthWeightBVector().resize(1);

  adapter.AsymptoticLengthVector()[0] = fims_math::log(100.0);
  adapter.GrowthCoefficientVector()[0] = fims_math::log(0.2);
  adapter.AgeAtZeroLengthVector()[0] = -0.5;
  adapter.LengthWeightAVector()[0] = fims_math::log(1.0e-5);
  adapter.LengthWeightBVector()[0] = fims_math::log(3.0);

  adapter.LogSdAsymptoticLengthVector().resize(1);
  adapter.LogSdGrowthCoefficientVector().resize(1);
  adapter.LogSdAgeAtZeroLengthVector().resize(1);
  adapter.LogitCorrAsymptoticLengthGrowthCoefficientVector().resize(1);
  adapter.LogitCorrAsymptoticLengthAgeAtZeroLengthVector().resize(1);
  adapter.LogitCorrGrowthCoefficientAgeAtZeroLengthVector().resize(1);

  const double sd_log_asymptotic_length = 0.05;
  const double sd_log_growth_coefficient = 0.10;
  const double sd_age_at_zero_length = 0.20;
  const double corr_linf_k = -0.5;
  const double corr_linf_t0 = 0.25;
  const double corr_k_t0 = -0.3;
  const auto corr_to_logit = [](double corr) {
    return std::log((corr + 1.0) / (1.0 - corr));
  };

  adapter.LogSdAsymptoticLengthVector()[0] =
      fims_math::log(sd_log_asymptotic_length);
  adapter.LogSdGrowthCoefficientVector()[0] =
      fims_math::log(sd_log_growth_coefficient);
  adapter.LogSdAgeAtZeroLengthVector()[0] =
      fims_math::log(sd_age_at_zero_length);
  adapter.LogitCorrAsymptoticLengthGrowthCoefficientVector()[0] =
      corr_to_logit(corr_linf_k);
  adapter.LogitCorrAsymptoticLengthAgeAtZeroLengthVector()[0] =
      corr_to_logit(corr_linf_t0);
  adapter.LogitCorrGrowthCoefficientAgeAtZeroLengthVector()[0] =
      corr_to_logit(corr_k_t0);

  adapter.SetAgeOffset(1.0);
  adapter.Initialize(1, 5, 1);
  adapter.PrepareGrowthProducts();

  const auto* products = adapter.TryGetPreparedGrowthProducts();
  ASSERT_NE(products, nullptr);

  const double age = 3.0;
  fims_popdy::VonBertalanffyTraditionalGrowth<double> vb;
  vb.asymptotic_length = 100.0;
  vb.growth_coefficient = 0.2;
  vb.age_at_zero_length = -0.5;

  double d_log_laa_d_log_asymptotic_length = 0.0;
  double d_log_laa_d_log_growth_coefficient = 0.0;
  double d_log_laa_d_age_at_zero_length = 0.0;

  vb.log_length_at_age_working_scale_gradient(
      age, d_log_laa_d_log_asymptotic_length,
      d_log_laa_d_log_growth_coefficient,
      d_log_laa_d_age_at_zero_length);

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

  EXPECT_NEAR(products->SdLAA(0, 2, 0), expected_sd, 1e-7);
}

}  // namespace
