#include <cmath>
#include "gtest/gtest.h"
#include "common/fims_math.hpp"
#include "population_dynamics/growth/growth_model_adapter.hpp"

namespace {

void ConfigureAdapter(fims_popdy::VonBertalanffyGrowthModelAdapter<double>& adapter,
                      double L1, double L2, double K, double A1, double A2,
                      double a_wl, double b_wl, double sd1, double sdA) {
  adapter.L1Vector().resize(1);
  adapter.L2Vector().resize(1);
  adapter.KVector().resize(1);
  adapter.AgeL1Vector().resize(1);
  adapter.AgeL2Vector().resize(1);
  adapter.AwlVector().resize(1);
  adapter.BwlVector().resize(1);
  adapter.SDgrowthVector().resize(2);

  // Adapter stores positive growth params on log scale.
  adapter.L1Vector()[0] = fims_math::log(L1);
  adapter.L2Vector()[0] = fims_math::log(L2);
  adapter.KVector()[0] = fims_math::log(K);
  adapter.AgeL1Vector()[0] = A1;
  adapter.AgeL2Vector()[0] = A2;
  adapter.AwlVector()[0] = fims_math::log(a_wl);
  adapter.BwlVector()[0] = fims_math::log(b_wl);
  adapter.SDgrowthVector()[0] = fims_math::log(sd1);
  adapter.SDgrowthVector()[1] = fims_math::log(sdA);
}

TEST(VonBertalanffyGrowthModelAdapter, UsesWaaFromLaa) {
  fims_popdy::VonBertalanffyGrowthModelAdapter<double> adapter;
  ConfigureAdapter(adapter, 10.0, 100.0, 0.2, 0.0, 50.0, 1e-5, 3.0, 3.0, 7.0);
  adapter.Initialize(1, 51, 1);

  const double age = 5.0;
  const double L1 = 10.0;
  const double L2 = 100.0;
  const double K = 0.2;
  const double A1 = 0.0;
  const double A2 = 50.0;
  const double denom = 1.0 - std::exp(-K * (A2 - A1));
  const double L = L1 + (L2 - L1) * (1.0 - std::exp(-K * (age - A1))) / denom;
  const double expected = 1e-5 * std::pow(L, 3.0);
  const double W = adapter.evaluate(age);

  EXPECT_NEAR(W, expected, 1e-8);
}

TEST(VonBertalanffyGrowthModelAdapter, HonorsAgeOffset) {
  fims_popdy::VonBertalanffyGrowthModelAdapter<double> adapter;
  // ages 1..51 (n_ages = 51), set reference ages to match
  ConfigureAdapter(adapter, 10.0, 100.0, 0.2, 1.0, 51.0, 1e-5, 3.0, 3.0, 7.0);
  adapter.SetAgeOffset(1.0);
  adapter.Initialize(1, 51, 1);

  const double age = 1.0;
  const double L1 = 10.0;
  const double L2 = 100.0;
  const double K = 0.2;
  const double A1 = 1.0;
  const double A2 = 51.0;
  const double denom = 1.0 - std::exp(-K * (A2 - A1));
  const double L = L1 + (L2 - L1) * (1.0 - std::exp(-K * (age - A1))) / denom;
  const double expected = 1e-5 * std::pow(L, 3.0);
  const double W = adapter.evaluate(age);

  EXPECT_NEAR(W, expected, 1e-8);
}

TEST(VonBertalanffyGrowthModelAdapter, RejectsFractionalAge) {
  fims_popdy::VonBertalanffyGrowthModelAdapter<double> adapter;
  ConfigureAdapter(adapter, 10.0, 100.0, 0.2, 0.0, 50.0, 1e-5, 3.0, 3.0, 7.0);
  adapter.Initialize(1, 51, 1);

  EXPECT_THROW(adapter.evaluate(5.5), std::runtime_error);
}

TEST(VonBertalanffyGrowthModelAdapter, RejectsNegativeAge) {
  fims_popdy::VonBertalanffyGrowthModelAdapter<double> adapter;
  ConfigureAdapter(adapter, 10.0, 100.0, 0.2, 0.0, 50.0, 1e-5, 3.0, 3.0, 7.0);
  adapter.Initialize(1, 51, 1);

  EXPECT_THROW(adapter.evaluate(-1.0), std::runtime_error);
}

TEST(VonBertalanffyGrowthModelAdapter, RejectsOutOfRangeAge) {
  fims_popdy::VonBertalanffyGrowthModelAdapter<double> adapter;
  ConfigureAdapter(adapter, 10.0, 100.0, 0.2, 0.0, 50.0, 1e-5, 3.0, 3.0, 7.0);
  adapter.Initialize(1, 51, 1);

  EXPECT_THROW(adapter.evaluate(51.0), std::runtime_error);
}

}  // namespace
