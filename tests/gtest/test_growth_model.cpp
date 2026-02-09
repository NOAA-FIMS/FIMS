#include <cmath>
#include "gtest/gtest.h"
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
  const double L1 = 10.0;
  const double L2 = 100.0;
  const double K = 0.2;
  const double A1 = 0.0;
  const double A2 = 50.0;
  const double denom = 1.0 - std::exp(-K * (A2 - A1));
  const double L0 = L1 + (L2 - L1) * (1.0 - std::exp(-K * (0.0 - A1))) / denom;
  const double L5 = L1 + (L2 - L1) * (1.0 - std::exp(-K * (age - A1))) / denom;
  const double L25 =
      L1 + (L2 - L1) * (1.0 - std::exp(-K * (25.0 - A1))) / denom;
  const double L50 = L1 + (L2 - L1) * (1.0 - std::exp(-K * (50.0 - A1))) / denom;
  const double L0_obs = p.MeanLAA(0, 0, 0);
  const double L50_obs = p.MeanLAA(0, 50, 0);

  EXPECT_NEAR(L0_obs, L0, 1e-12);
  EXPECT_GT(L5, L1);
  EXPECT_LT(L5, L2);
  EXPECT_NEAR(L50_obs, L50, 1e-12);
  EXPECT_NEAR(p.SdLAA(0, 0, 0), 3.0, 1e-12);
  EXPECT_NEAR(p.SdLAA(0, 50, 0), 7.0, 1e-12);
  // Length-based interpolation for SD
  const double expected_sd_mid =
      3.0 + (7.0 - 3.0) * (L25 - L0) / (L50 - L0);
  EXPECT_NEAR(p.SdLAA(0, 25, 0), expected_sd_mid, 1e-12);

  // weight exists in the contract (derived from length-weight)
  const double W5 = p.MeanWAA(0, 5, 0);
  EXPECT_GT(W5, 0.0);
  EXPECT_NEAR(W5, 1e-5 * std::pow(L5, 3.0), 1e-8);

}

TEST(GrowthModel, NegativeLaaError) {
  fims_popdy::GrowthModel<double> gm(1, 2, 1);
  gm.SetVonBertalanffyParameters(-10.0, 50.0, 0.2, 0.0, 1.0);
  gm.SetLengthWeightParameters(1e-5, 3.0);
  gm.SetLengthSdParams(3.0, 7.0);

  EXPECT_THROW(gm.Prepare(), std::runtime_error);
}

TEST(GrowthModel, LaaRangeError) {
  fims_popdy::GrowthModel<double> gm(1, 3, 1);
  gm.SetVonBertalanffyParameters(10.0, 10.0, 0.0, 0.0, 2.0);  // flat LAA
  gm.SetLengthWeightParameters(1e-5, 3.0);
  gm.SetLengthSdParams(3.0, 7.0);

  EXPECT_THROW(gm.Prepare(), std::runtime_error);
}

TEST(GrowthModel, InvalidAgeOrder) {
  fims_popdy::GrowthModel<double> gm(1, 2, 1);
  gm.SetVonBertalanffyParameters(10.0, 100.0, 0.2, 5.0, 5.0);
  gm.SetLengthWeightParameters(1e-5, 3.0);
  gm.SetLengthSdParams(3.0, 7.0);

  EXPECT_THROW(gm.Prepare(), std::runtime_error);
}

TEST(GrowthModel, InvalidLengthWeightParams) {
  fims_popdy::GrowthModel<double> gm(1, 2, 1);
  gm.SetVonBertalanffyParameters(10.0, 100.0, 0.2, 0.0, 1.0);
  gm.SetLengthWeightParameters(0.0, 3.0);
  gm.SetLengthSdParams(3.0, 7.0);

  EXPECT_THROW(gm.Prepare(), std::runtime_error);
}

TEST(GrowthModel, InvalidSdParams) {
  fims_popdy::GrowthModel<double> gm(1, 2, 1);
  gm.SetVonBertalanffyParameters(10.0, 100.0, 0.2, 0.0, 1.0);
  gm.SetLengthWeightParameters(1e-5, 3.0);
  gm.SetLengthSdParams(-1.0, 7.0);

  EXPECT_THROW(gm.Prepare(), std::runtime_error);
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
