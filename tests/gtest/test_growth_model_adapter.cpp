#include <cmath>
#include "gtest/gtest.h"
#include "common/fims_math.hpp"
#include "population_dynamics/growth/growth_model_adapter.hpp"

namespace {

void ConfigureAdapter(fims_popdy::VonBertalanffyGrowthModelAdapter<double>& adapter,
                      double length_at_ref_age_1, double length_at_ref_age_2, double growth_coefficient_K, double reference_age_for_length_1, double reference_age_for_length_2,
                      double length_weight_a, double length_weight_b,
                      double length_at_age_sd_at_reference_age_1,
                      double length_at_age_sd_at_reference_age_2) {
  adapter.LengthAtRefAge1Vector().resize(1);
  adapter.LengthAtRefAge2Vector().resize(1);
  adapter.GrowthCoefficientKVector().resize(1);
  adapter.ReferenceAgeForLength1Vector().resize(1);
  adapter.ReferenceAgeForLength2Vector().resize(1);
  adapter.LengthWeightAVector().resize(1);
  adapter.LengthWeightBVector().resize(1);
  adapter.LengthAtAgeSdAtRefAgesVector().resize(2);

  // Adapter stores positive growth params on log scale.
  adapter.LengthAtRefAge1Vector()[0] = fims_math::log(length_at_ref_age_1);
  adapter.LengthAtRefAge2Vector()[0] = fims_math::log(length_at_ref_age_2);
  adapter.GrowthCoefficientKVector()[0] = fims_math::log(growth_coefficient_K);
  adapter.ReferenceAgeForLength1Vector()[0] = reference_age_for_length_1;
  adapter.ReferenceAgeForLength2Vector()[0] = reference_age_for_length_2;
  adapter.LengthWeightAVector()[0] = fims_math::log(length_weight_a);
  adapter.LengthWeightBVector()[0] = fims_math::log(length_weight_b);
  adapter.LengthAtAgeSdAtRefAgesVector()[0] =
      fims_math::log(length_at_age_sd_at_reference_age_1);
  adapter.LengthAtAgeSdAtRefAgesVector()[1] =
      fims_math::log(length_at_age_sd_at_reference_age_2);
}

TEST(VonBertalanffyGrowthModelAdapter, UsesWaaFromLaa) {
  fims_popdy::VonBertalanffyGrowthModelAdapter<double> adapter;
  ConfigureAdapter(adapter, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0, 73.0);
  adapter.SetAgeOffset(1.0);
  adapter.Initialize(1, 12, 1);

  const double age = 5.0;
  const double length_at_ref_age_1 = 275.0;
  const double length_at_ref_age_2 = 725.0;
  const double growth_coefficient_K = 0.18;
  const double reference_age_for_length_1 = 1.0;
  const double reference_age_for_length_2 = 12.0;
  const double denom_raw = 1.0 - std::exp(-growth_coefficient_K * (reference_age_for_length_2 - reference_age_for_length_1));
  const double denom = fims_math::ad_max(fims_math::ad_fabs(denom_raw), 1e-8);
  const double L = length_at_ref_age_1 + (length_at_ref_age_2 - length_at_ref_age_1) * (1.0 - std::exp(-growth_coefficient_K * (age - reference_age_for_length_1))) / denom;
  const double expected = 2.5e-11 * std::pow(L, 3.0);
  const double W = adapter.evaluate(age);

  EXPECT_NEAR(W, expected, 1e-8);
}

TEST(VonBertalanffyGrowthModelAdapter, HonorsAgeOffset) {
  fims_popdy::VonBertalanffyGrowthModelAdapter<double> adapter;
  // ages 1..12 (n_ages = 12), set reference ages to match
  ConfigureAdapter(adapter, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0, 73.0);
  adapter.SetAgeOffset(1.0);
  adapter.Initialize(1, 12, 1);

  const double age = 1.0;
  const double length_at_ref_age_1 = 275.0;
  const double length_at_ref_age_2 = 725.0;
  const double growth_coefficient_K = 0.18;
  const double reference_age_for_length_1 = 1.0;
  const double reference_age_for_length_2 = 12.0;
  const double denom_raw = 1.0 - std::exp(-growth_coefficient_K * (reference_age_for_length_2 - reference_age_for_length_1));
  const double denom = fims_math::ad_max(fims_math::ad_fabs(denom_raw), 1e-8);
  const double L = length_at_ref_age_1 + (length_at_ref_age_2 - length_at_ref_age_1) * (1.0 - std::exp(-growth_coefficient_K * (age - reference_age_for_length_1))) / denom;
  const double expected = 2.5e-11 * std::pow(L, 3.0);
  const double W = adapter.evaluate(age);

  EXPECT_NEAR(W, expected, 1e-8);
}

TEST(VonBertalanffyGrowthModelAdapter, RejectsFractionalAge) {
  fims_popdy::VonBertalanffyGrowthModelAdapter<double> adapter;
  ConfigureAdapter(adapter, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0, 73.0);
  adapter.SetAgeOffset(1.0);
  adapter.Initialize(1, 12, 1);

  EXPECT_THROW(adapter.evaluate(5.5), std::runtime_error);
}

TEST(VonBertalanffyGrowthModelAdapter, RejectsNegativeAge) {
  fims_popdy::VonBertalanffyGrowthModelAdapter<double> adapter;
  ConfigureAdapter(adapter, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0, 73.0);
  adapter.SetAgeOffset(1.0);
  adapter.Initialize(1, 12, 1);

  EXPECT_THROW(adapter.evaluate(-1.0), std::runtime_error);
}

TEST(VonBertalanffyGrowthModelAdapter, ExtrapolatesAboveCachedAgeRange) {
  fims_popdy::VonBertalanffyGrowthModelAdapter<double> adapter;
  ConfigureAdapter(adapter, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0, 73.0);
  adapter.SetAgeOffset(1.0);
  adapter.Initialize(1, 12, 1);

  const double age = 13.0;
  const double length_at_ref_age_1 = 275.0;
  const double length_at_ref_age_2 = 725.0;
  const double growth_coefficient_K = 0.18;
  const double reference_age_for_length_1 = 1.0;
  const double reference_age_for_length_2 = 12.0;
  const double denom_raw = 1.0 - std::exp(-growth_coefficient_K * (reference_age_for_length_2 - reference_age_for_length_1));
  const double denom = fims_math::ad_max(fims_math::ad_fabs(denom_raw), 1e-8);
  const double L = length_at_ref_age_1 + (length_at_ref_age_2 - length_at_ref_age_1) * (1.0 - std::exp(-growth_coefficient_K * (age - reference_age_for_length_1))) / denom;
  const double expected = 2.5e-11 * std::pow(L, 3.0);

  const double W = adapter.evaluate(age);
  EXPECT_NEAR(W, expected, 1e-8);
}

}  // namespace
