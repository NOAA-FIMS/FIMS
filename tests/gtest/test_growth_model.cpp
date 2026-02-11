#include <cmath>
#include "gtest/gtest.h"
#include "common/fims_math.hpp"
#include "population_dynamics/growth/growth_model.hpp"

namespace {

TEST(GrowthModel, CanConstructAndPrepare) {
  // years=1, ages=51 (0..50), sexes=1
  fims_popdy::GrowthModel<double> gm(1, 51, 1);

  gm.SetVonBertalanffyParameters(10.0, 100.0, 0.2, 0.0, 50.0);
  gm.SetLengthWeightParameters(1e-5, 3.0);
  gm.SetLengthSdParams(3.0, 7.0);

  gm.Prepare();

  const auto& p = gm.GetProducts();

  EXPECT_EQ(p.n_years, 1u);
  EXPECT_EQ(p.n_ages, 51u);
  EXPECT_EQ(p.n_sexes, 1u);

  // Check a couple of values exist and look sane
  const double age = 5.0;
  const double length_at_ref_age_1 = 10.0;
  const double length_at_ref_age_2 = 100.0;
  const double growth_coefficient_K = 0.2;
  const double reference_age_for_length_1 = 0.0;
  const double reference_age_for_length_2 = 50.0;
  const double denom_raw = 1.0 - std::exp(
      -growth_coefficient_K *
      (reference_age_for_length_2 - reference_age_for_length_1));
  const double denom = fims_math::ad_max(fims_math::ad_fabs(denom_raw), 1e-8);
  const double expected_length_at_age_0 =
      length_at_ref_age_1 +
      (length_at_ref_age_2 - length_at_ref_age_1) *
          (1.0 - std::exp(-growth_coefficient_K *
                          (0.0 - reference_age_for_length_1))) /
          denom;
  const double expected_length_at_age_5 =
      length_at_ref_age_1 +
      (length_at_ref_age_2 - length_at_ref_age_1) *
          (1.0 - std::exp(-growth_coefficient_K *
                          (age - reference_age_for_length_1))) /
          denom;
  const double expected_length_at_age_25 =
      length_at_ref_age_1 +
      (length_at_ref_age_2 - length_at_ref_age_1) *
          (1.0 - std::exp(-growth_coefficient_K *
                          (25.0 - reference_age_for_length_1))) /
          denom;
  const double expected_length_at_age_50 =
      length_at_ref_age_1 +
      (length_at_ref_age_2 - length_at_ref_age_1) *
          (1.0 - std::exp(-growth_coefficient_K *
                          (50.0 - reference_age_for_length_1))) /
          denom;
  const double observed_length_at_age_0 = p.MeanLAA(0, 0, 0);
  const double observed_length_at_age_50 = p.MeanLAA(0, 50, 0);

  EXPECT_NEAR(observed_length_at_age_0, expected_length_at_age_0, 1e-8);
  EXPECT_GT(expected_length_at_age_5, length_at_ref_age_1);
  EXPECT_LT(expected_length_at_age_5, length_at_ref_age_2);
  EXPECT_NEAR(observed_length_at_age_50, expected_length_at_age_50, 1e-8);
  EXPECT_NEAR(p.SdLAA(0, 0, 0), 3.0, 1e-8);
  EXPECT_NEAR(p.SdLAA(0, 50, 0), 7.0, 1e-8);
  // Length-based interpolation for SD
  const double expected_sd_mid =
      3.0 + (7.0 - 3.0) *
                (expected_length_at_age_25 - expected_length_at_age_0) /
                (expected_length_at_age_50 - expected_length_at_age_0);
  EXPECT_NEAR(p.SdLAA(0, 25, 0), expected_sd_mid, 1e-8);

  // weight exists in the contract (derived from length-weight)
  const double W5 = p.MeanWAA(0, 5, 0);
  EXPECT_GT(W5, 0.0);
  EXPECT_NEAR(W5, 1e-5 * std::pow(expected_length_at_age_5, 3.0), 1e-8);

}

TEST(GrowthModel, ZeroAgesThrows) {
  fims_popdy::GrowthModel<double> gm(1, 0, 1);
  gm.SetVonBertalanffyParameters(10.0, 50.0, 0.2, 0.0, 1.0);
  gm.SetLengthWeightParameters(1e-5, 3.0);
  gm.SetLengthSdParams(3.0, 7.0);

  EXPECT_THROW(gm.Prepare(), std::runtime_error);
}

TEST(GrowthModel, HandlesDegenerateLaaRangeWithoutThrow) {
  fims_popdy::GrowthModel<double> gm(1, 2, 1);
  gm.SetVonBertalanffyParameters(10.0, 10.0, 0.2, 0.0, 1.0);
  gm.SetLengthWeightParameters(1e-5, 3.0);
  gm.SetLengthSdParams(3.0, 7.0);

  EXPECT_NO_THROW(gm.Prepare());
  const auto& p = gm.GetProducts();
  EXPECT_TRUE(std::isfinite(p.MeanLAA(0, 0, 0)));
  EXPECT_TRUE(std::isfinite(p.SdLAA(0, 0, 0)));
  EXPECT_TRUE(std::isfinite(p.MeanWAA(0, 0, 0)));
}

TEST(GrowthModel, HandlesCoincidentReferenceAgesWithoutThrow) {
  fims_popdy::GrowthModel<double> gm(1, 3, 1);
  gm.SetVonBertalanffyParameters(10.0, 100.0, 0.2, 5.0, 5.0);
  gm.SetLengthWeightParameters(1e-5, 3.0);
  gm.SetLengthSdParams(3.0, 7.0);

  EXPECT_NO_THROW(gm.Prepare());
  const auto& p = gm.GetProducts();
  EXPECT_TRUE(std::isfinite(p.MeanLAA(0, 0, 0)));
  EXPECT_TRUE(std::isfinite(p.MeanWAA(0, 0, 0)));
}

TEST(GrowthModel, HandlesZeroLengthWeightAWithoutThrow) {
  fims_popdy::GrowthModel<double> gm(1, 2, 1);
  gm.SetVonBertalanffyParameters(10.0, 100.0, 0.2, 0.0, 1.0);
  gm.SetLengthWeightParameters(0.0, 3.0);
  gm.SetLengthSdParams(3.0, 7.0);

  EXPECT_NO_THROW(gm.Prepare());
  const auto& p = gm.GetProducts();
  EXPECT_NEAR(p.MeanWAA(0, 0, 0), 0.0, 1e-12);
}

TEST(GrowthModel, HandlesNegativeSdInputsWithoutThrow) {
  fims_popdy::GrowthModel<double> gm(1, 2, 1);
  gm.SetVonBertalanffyParameters(10.0, 100.0, 0.2, 0.0, 1.0);
  gm.SetLengthWeightParameters(1e-5, 3.0);
  gm.SetLengthSdParams(-1.0, 7.0);

  EXPECT_NO_THROW(gm.Prepare());
  const auto& p = gm.GetProducts();
  EXPECT_TRUE(std::isfinite(p.SdLAA(0, 0, 0)));
}

TEST(GrowthModel, SingleAgeAllowed) {
  fims_popdy::GrowthModel<double> gm(1, 1, 1);
  // allow equal reference ages and lengths when only one age
  gm.SetVonBertalanffyParameters(10.0, 10.0, 0.2, 0.0, 0.0);
  gm.SetLengthWeightParameters(1e-5, 3.0);
  gm.SetLengthSdParams(3.0, 7.0);

  EXPECT_NO_THROW(gm.Prepare());
  const auto& p = gm.GetProducts();
  EXPECT_EQ(p.n_ages, 1u);
  EXPECT_NEAR(p.MeanLAA(0, 0, 0), 10.0, 1e-12);
  EXPECT_NEAR(p.SdLAA(0, 0, 0), 3.0, 1e-12);
}

}  // namespace
