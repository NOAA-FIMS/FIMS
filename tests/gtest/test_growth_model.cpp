#include <cmath>
#include <limits>
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

TEST(GrowthModel, RejectsNonFiniteTraditionalCovarianceInputs) {
  const double invalid_values[] = {
      std::numeric_limits<double>::quiet_NaN(),
      std::numeric_limits<double>::infinity(),
      -std::numeric_limits<double>::infinity()};

  for (int index = 0; index < 6; ++index) {
    for (const double invalid_value : invalid_values) {
      fims_popdy::GrowthModel<double> gm(1, 5, 1);
      double covariance_inputs[] = {
          0.01, 0.0, 0.0, 0.01, 0.0, 0.01};
      covariance_inputs[index] = invalid_value;

      SCOPED_TRACE(index);
      SCOPED_TRACE(invalid_value);

      EXPECT_THROW(
          gm.SetTraditionalGrowthParameterCovariance(
              covariance_inputs[0], covariance_inputs[1],
              covariance_inputs[2], covariance_inputs[3],
              covariance_inputs[4], covariance_inputs[5]),
          std::runtime_error);
    }
  }
}

TEST(GrowthModel, RejectsNonFiniteCalculatedLengthVariance) {
  fims_popdy::GrowthModel<double> gm(1, 5, 1);

  gm.SetVonBertalanffyTraditionalParameters(1e200, 0.2, -0.5);
  gm.SetLengthWeightParameters(1.0, 1.0);
  gm.SetTraditionalGrowthParameterCovariance(
      0.01, 0.0, 0.0, 0.01, 0.0, 0.0);
  gm.SetAgeOffset(1.0);

  // Finite inputs can still overflow when length derivatives are squared.
  try {
    gm.Prepare();
    FAIL() << "Expected rejection of non-finite calculated length variance";
  } catch (const std::runtime_error& error) {
    EXPECT_STREQ(
        error.what(),
        "Traditional growth calculated length variance must be finite "
        "and nonnegative");
  }
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

TEST(GrowthModel, TraditionalDeltaMethodHandlesNearPerfectCorrelations) {
  const double asymptotic_length = 100.0;
  const double growth_coefficient = 0.2;
  const double age_at_zero_length = -0.5;
  const double age = 3.0;
  const double scaled_age =
      growth_coefficient * (age - age_at_zero_length);
  const double survival_like = std::exp(-scaled_age);

  const double length =
      asymptotic_length * (1.0 - survival_like);
  const double derivative_log_k =
      asymptotic_length * scaled_age * survival_like;

  const double sd_log_k = 0.1;
  const double sd_log_linf = derivative_log_k * sd_log_k / length;
  const double contribution = derivative_log_k * sd_log_k;

  // Equal contributions nearly cancel when their correlation approaches -1.
  for (const double correlation : {-0.999999, 0.0, 0.999999}) {
    SCOPED_TRACE(correlation);
    fims_popdy::GrowthModel<double> gm(1, 1, 1);

    gm.SetVonBertalanffyTraditionalParameters(
        asymptotic_length, growth_coefficient, age_at_zero_length);
    gm.SetLengthWeightParameters(1e-5, 3.0);
    gm.SetTraditionalGrowthParameterCovariance(
        sd_log_linf * sd_log_linf,
        correlation * sd_log_linf * sd_log_k,
        0.0,
        sd_log_k * sd_log_k,
        0.0,
        0.0);
    gm.SetAgeOffset(age);

    ASSERT_NO_THROW(gm.Prepare());

    const double actual_sd = gm.GetProducts().SdLAA(0, 0, 0);
    const double expected_sd =
        contribution * std::sqrt(2.0 * (1.0 + correlation));

    ASSERT_TRUE(std::isfinite(actual_sd));
    EXPECT_GT(actual_sd, 0.0);
    EXPECT_NEAR(actual_sd, expected_sd, expected_sd * 1e-8);
  }
}

TEST(GrowthModel, TraditionalDeltaMethodHasZeroSdAtFixedT0) {
  fims_popdy::GrowthModel<double> gm(1, 5, 1);

  gm.SetVonBertalanffyTraditionalParameters(100.0, 0.2, 1.0);
  gm.SetLengthWeightParameters(1.0e-5, 3.0);
  gm.SetTraditionalGrowthParameterCovariance(
      0.05 * 0.05,
      -0.5 * 0.05 * 0.10,
      0.0,
      0.10 * 0.10,
      0.0,
      0.0);
  gm.SetAgeOffset(1.0);

  gm.Prepare();

  const auto& products = gm.GetProducts();

  // All individual curves have zero length at their shared, fixed t0.
  EXPECT_DOUBLE_EQ(products.MeanLAA(0, 0, 0), 0.0);
  EXPECT_DOUBLE_EQ(products.SdLAA(0, 0, 0), 0.0);

  EXPECT_GT(products.MeanLAA(0, 1, 0), 0.0);
  EXPECT_GT(products.SdLAA(0, 1, 0), 0.0);
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

  const double age_delta = age - age_at_zero_length;
  const double survival_like = std::exp(-growth_coefficient * age_delta);
  const double expected_length =
      asymptotic_length * (1.0 - survival_like);

  const double d_length_d_log_linf = expected_length;
  const double d_length_d_log_k =
      asymptotic_length * growth_coefficient * age_delta * survival_like;
  const double d_length_d_t0 =
      -asymptotic_length * growth_coefficient * survival_like;

  const double expected_variance =
      d_length_d_log_linf * d_length_d_log_linf *
          var_log_asymptotic_length +
      d_length_d_log_k * d_length_d_log_k *
          var_log_growth_coefficient +
      d_length_d_t0 * d_length_d_t0 * var_age_at_zero_length +
      2.0 * d_length_d_log_linf * d_length_d_log_k * cov_linf_k +
      2.0 * d_length_d_log_linf * d_length_d_t0 * cov_linf_t0 +
      2.0 * d_length_d_log_k * d_length_d_t0 * cov_k_t0;

  const double expected_sd = std::sqrt(expected_variance);

  EXPECT_NEAR(p.SdLAA(0, 2, 0), expected_sd, 1e-7);
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
