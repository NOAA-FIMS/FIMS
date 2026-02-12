#include <cmath>
#include "gtest/gtest.h"
#include "common/fims_math.hpp"
#include "population_dynamics/growth/growth_model.hpp"

namespace {

TEST(GrowthModel, CanConstructAndPrepare) {
  // years=1, ages=12 (1..12), sexes=1
  fims_popdy::GrowthModel<double> gm(1, 12, 1);

  gm.SetVonBertalanffyParameters(275.0, 725.0, 0.18, 1.0, 12.0);
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
  const double length_at_ref_age_1 = 275.0;
  const double length_at_ref_age_2 = 725.0;
  const double growth_coefficient_K = 0.18;
  const double reference_age_for_length_1 = 1.0;
  const double reference_age_for_length_2 = 12.0;
  const double denom_raw = 1.0 - std::exp(
      -growth_coefficient_K *
      (reference_age_for_length_2 - reference_age_for_length_1));
  const double denom = fims_math::ad_max(fims_math::ad_fabs(denom_raw), 1e-8);
  const double expected_length_at_age_1 =
      length_at_ref_age_1 +
      (length_at_ref_age_2 - length_at_ref_age_1) *
          (1.0 - std::exp(-growth_coefficient_K *
                          (1.0 - reference_age_for_length_1))) /
          denom;
  const double expected_length_at_age_5 =
      length_at_ref_age_1 +
      (length_at_ref_age_2 - length_at_ref_age_1) *
          (1.0 - std::exp(-growth_coefficient_K *
                          (age - reference_age_for_length_1))) /
          denom;
  const double expected_length_at_age_8 =
      length_at_ref_age_1 +
      (length_at_ref_age_2 - length_at_ref_age_1) *
          (1.0 - std::exp(-growth_coefficient_K *
                          (8.0 - reference_age_for_length_1))) /
          denom;
  const double expected_length_at_age_12 =
      length_at_ref_age_1 +
      (length_at_ref_age_2 - length_at_ref_age_1) *
          (1.0 - std::exp(-growth_coefficient_K *
                          (12.0 - reference_age_for_length_1))) /
          denom;
  const double observed_length_at_age_1 = p.MeanLAA(0, 0, 0);
  const double observed_length_at_age_12 = p.MeanLAA(0, 11, 0);

  EXPECT_NEAR(observed_length_at_age_1, expected_length_at_age_1, 1e-8);
  EXPECT_GT(expected_length_at_age_5, length_at_ref_age_1);
  EXPECT_LT(expected_length_at_age_5, length_at_ref_age_2);
  EXPECT_NEAR(observed_length_at_age_12, expected_length_at_age_12, 1e-8);
  EXPECT_NEAR(p.SdLAA(0, 0, 0), 28.0, 1e-8);
  EXPECT_NEAR(p.SdLAA(0, 11, 0), 73.0, 1e-8);
  // Length-based interpolation for SD
  const double expected_sd_mid =
      28.0 + (73.0 - 28.0) *
                (expected_length_at_age_8 - expected_length_at_age_1) /
                (expected_length_at_age_12 - expected_length_at_age_1);
  EXPECT_NEAR(p.SdLAA(0, 7, 0), expected_sd_mid, 1e-8);

  // weight exists in the contract (derived from length-weight)
  const double W5 = p.MeanWAA(0, 4, 0);
  EXPECT_GT(W5, 0.0);
  EXPECT_NEAR(W5, 2.5e-11 * std::pow(expected_length_at_age_5, 3.0), 1e-8);

}

TEST(GrowthModel, ZeroAgesThrows) {
  fims_popdy::GrowthModel<double> gm(1, 0, 1);
  gm.SetVonBertalanffyParameters(275.0, 725.0, 0.18, 1.0, 12.0);
  gm.SetLengthWeightParameters(2.5e-11, 3.0);
  gm.SetLengthSdParams(28.0, 73.0);

  EXPECT_THROW(gm.Prepare(), std::runtime_error);
}

TEST(GrowthModel, HandlesDegenerateLaaRangeWithoutThrow) {
  fims_popdy::GrowthModel<double> gm(1, 2, 1);
  gm.SetVonBertalanffyParameters(275.0, 275.0, 0.18, 1.0, 12.0);
  gm.SetLengthWeightParameters(2.5e-11, 3.0);
  gm.SetLengthSdParams(28.0, 73.0);

  EXPECT_NO_THROW(gm.Prepare());
  const auto& p = gm.GetProducts();
  EXPECT_TRUE(std::isfinite(p.MeanLAA(0, 0, 0)));
  EXPECT_TRUE(std::isfinite(p.SdLAA(0, 0, 0)));
  EXPECT_TRUE(std::isfinite(p.MeanWAA(0, 0, 0)));
}

TEST(GrowthModel, RejectsCoincidentReferenceAges) {
  fims_popdy::GrowthModel<double> gm(1, 3, 1);
  gm.SetVonBertalanffyParameters(275.0, 725.0, 0.18, 5.0, 5.0);
  gm.SetLengthWeightParameters(2.5e-11, 3.0);
  gm.SetLengthSdParams(28.0, 73.0);

  EXPECT_THROW(gm.Prepare(), std::runtime_error);
}

TEST(GrowthModel, HandlesZeroLengthWeightAWithoutThrow) {
  fims_popdy::GrowthModel<double> gm(1, 2, 1);
  gm.SetVonBertalanffyParameters(275.0, 725.0, 0.18, 1.0, 12.0);
  gm.SetLengthWeightParameters(0.0, 3.0);
  gm.SetLengthSdParams(28.0, 73.0);

  EXPECT_NO_THROW(gm.Prepare());
  const auto& p = gm.GetProducts();
  EXPECT_NEAR(p.MeanWAA(0, 0, 0), 0.0, 1e-12);
}

TEST(GrowthModel, HandlesNegativeSdInputsWithoutThrow) {
  fims_popdy::GrowthModel<double> gm(1, 2, 1);
  gm.SetVonBertalanffyParameters(275.0, 725.0, 0.18, 1.0, 12.0);
  gm.SetLengthWeightParameters(2.5e-11, 3.0);
  gm.SetLengthSdParams(-1.0, 73.0);

  EXPECT_NO_THROW(gm.Prepare());
  const auto& p = gm.GetProducts();
  EXPECT_TRUE(std::isfinite(p.SdLAA(0, 0, 0)));
}

TEST(GrowthModel, SingleAgeRejectsCoincidentReferenceAges) {
  fims_popdy::GrowthModel<double> gm(1, 1, 1);
  // Coincident reference ages are invalid even with one modeled age.
  gm.SetVonBertalanffyParameters(275.0, 275.0, 0.18, 1.0, 1.0);
  gm.SetLengthWeightParameters(2.5e-11, 3.0);
  gm.SetLengthSdParams(28.0, 73.0);

  EXPECT_THROW(gm.Prepare(), std::runtime_error);
}

}  // namespace
