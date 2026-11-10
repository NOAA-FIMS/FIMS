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
  adapter.LengthAtAgeSdYoungVector().resize(1);
  adapter.LengthAtAgeSdOldVector().resize(1);

  // Adapter stores positive growth params on log scale.
  adapter.MeanLengthYoungVector()[0] = fims_math::log(mean_length_young);
  adapter.MeanLengthOldVector()[0] = fims_math::log(mean_length_old);
  adapter.GrowthCoefficientVector()[0] = fims_math::log(growth_coefficient);
  adapter.ReferenceAgeForLengthYoungVector()[0] =
      reference_age_for_length_young;
  adapter.ReferenceAgeForLengthOldVector()[0] = reference_age_for_length_old;
  adapter.LengthWeightAVector()[0] = fims_math::log(length_weight_a);
  adapter.LengthWeightBVector()[0] = fims_math::log(length_weight_b);
  adapter.LengthAtAgeSdYoungVector()[0] =
      fims_math::log(length_at_age_sd_at_reference_age_young);
  adapter.LengthAtAgeSdOldVector()[0] =
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
     TraditionalVonBertalanffyUsesDeltaMethodProducts) {
  fims_popdy::VonBertalanffyGrowthModelAdapter<double> adapter;
  adapter.UseVonBertalanffyTraditional();

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
  adapter.LogitCorrAsymptoticLengthGrowthCoefficientVector().resize(1);

  const double sd_log_asymptotic_length = 0.05;
  const double sd_log_growth_coefficient = 0.10;
  const double corr_linf_k = -0.5;

  adapter.LogSdAsymptoticLengthVector()[0] =
      fims_math::log(sd_log_asymptotic_length);
  adapter.LogSdGrowthCoefficientVector()[0] =
      fims_math::log(sd_log_growth_coefficient);
  adapter.LogitCorrAsymptoticLengthGrowthCoefficientVector()[0] =
      std::log((corr_linf_k + 1.0) / (1.0 - corr_linf_k));

  adapter.SetAgeOffset(1.0);
  adapter.Initialize(1, 5, 1);
  adapter.PrepareGrowthProducts();

  const auto* products = adapter.TryGetPreparedGrowthProducts();
  ASSERT_NE(products, nullptr);

  const double age = 3.0;
  const double age_delta = age - -0.5;
  const double survival_like = std::exp(-0.2 * age_delta);
  const double expected_length = 100.0 * (1.0 - survival_like);

  // Analytical derivatives of log length: 1 for log(Linf), and this for log(K).
  const double d_log_laa_d_log_growth_coefficient =
      0.2 * age_delta * survival_like / (1.0 - survival_like);

  const double var_log_asymptotic_length =
      sd_log_asymptotic_length * sd_log_asymptotic_length;
  const double var_log_growth_coefficient =
      sd_log_growth_coefficient * sd_log_growth_coefficient;
  const double cov_linf_k =
      corr_linf_k * sd_log_asymptotic_length * sd_log_growth_coefficient;

  // Fixed t0 contributes no variance or covariance terms.
  const double expected_log_var =
      var_log_asymptotic_length +
      d_log_laa_d_log_growth_coefficient *
          d_log_laa_d_log_growth_coefficient * var_log_growth_coefficient +
      2.0 * d_log_laa_d_log_growth_coefficient * cov_linf_k;

  const double expected_sd =
      expected_length * std::sqrt(expected_log_var);

  EXPECT_NEAR(products->MeanLAA(0, 2, 0), expected_length, 1e-8);
  EXPECT_NEAR(products->SdLAA(0, 2, 0), expected_sd, 1e-7);
}

}  // namespace
