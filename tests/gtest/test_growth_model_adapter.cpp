#include <cmath>
#include "gtest/gtest.h"
#include "common/fims_math.hpp"
#include "population_dynamics/growth/growth_model_adapter.hpp"

namespace {

void ConfigureAdapter(fims_popdy::VonBSchnuteGrowthModelAdapter<double>& adapter,
                      double mean_length_young, double mean_length_old, double von_bertalanffy_coefficient_K, double reference_age_for_length_1, double reference_age_for_length_2,
                      double length_weight_a, double length_weight_b,
                      double length_at_age_sd_at_reference_age_1,
                      double length_at_age_sd_at_reference_age_2) {
  adapter.MeanLengthYoungVector().resize(1);
  adapter.MeanLengthOldVector().resize(1);
  adapter.VonBertalanffyCoefficientKVector().resize(1);
  adapter.ReferenceAgeForLength1Vector().resize(1);
  adapter.ReferenceAgeForLength2Vector().resize(1);
  adapter.LengthWeightAVector().resize(1);
  adapter.LengthWeightBVector().resize(1);
  adapter.LengthAtAgeSdAtRefAgesVector().resize(2);

  // Adapter stores positive growth params on log scale.
  adapter.MeanLengthYoungVector()[0] = fims_math::log(mean_length_young);
  adapter.MeanLengthOldVector()[0] =
    fims_math::log(mean_length_old - mean_length_young);
  adapter.VonBertalanffyCoefficientKVector()[0] = fims_math::log(von_bertalanffy_coefficient_K);
  adapter.ReferenceAgeForLength1Vector()[0] = reference_age_for_length_1;
  adapter.ReferenceAgeForLength2Vector()[0] = reference_age_for_length_2;
  adapter.LengthWeightAVector()[0] = fims_math::log(length_weight_a);
  adapter.LengthWeightBVector()[0] = fims_math::log(length_weight_b);
  adapter.LengthAtAgeSdAtRefAgesVector()[0] =
      fims_math::log(length_at_age_sd_at_reference_age_1);
  adapter.LengthAtAgeSdAtRefAgesVector()[1] =
      fims_math::log(length_at_age_sd_at_reference_age_2);
}

TEST(VonBSchnuteGrowthModelAdapter, UsesWaaFromLaa) {
  fims_popdy::VonBSchnuteGrowthModelAdapter<double> adapter;
  ConfigureAdapter(adapter, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0, 73.0);
  adapter.SetAgeOffset(1.0);
  adapter.Initialize(1, 12, 1);

  const double age = 5.0;
  const double mean_length_young = 275.0;
  const double mean_length_old = 725.0;
  const double von_bertalanffy_coefficient_K = 0.18;
  const double reference_age_for_length_1 = 1.0;
  const double reference_age_for_length_2 = 12.0;
  const double denom_raw = 1.0 - std::exp(-von_bertalanffy_coefficient_K * (reference_age_for_length_2 - reference_age_for_length_1));
  const double denom = fims_math::ad_max(fims_math::ad_fabs(denom_raw), 1e-8);
  const double L = mean_length_young + (mean_length_old - mean_length_young) * (1.0 - std::exp(-von_bertalanffy_coefficient_K * (age - reference_age_for_length_1))) / denom;
  const double expected = 2.5e-11 * std::pow(L, 3.0);
  const double W = adapter.evaluate(0, age);

  EXPECT_NEAR(W, expected, 1e-8);
}

TEST(VonBSchnuteGrowthModelAdapter, HonorsAgeOffset) {
  fims_popdy::VonBSchnuteGrowthModelAdapter<double> adapter;
  // ages 1..12 (n_ages = 12), set reference ages to match
  ConfigureAdapter(adapter, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0, 73.0);
  adapter.SetAgeOffset(1.0);
  adapter.Initialize(1, 12, 1);

  const double age = 1.0;
  const double mean_length_young = 275.0;
  const double mean_length_old = 725.0;
  const double von_bertalanffy_coefficient_K = 0.18;
  const double reference_age_for_length_1 = 1.0;
  const double reference_age_for_length_2 = 12.0;
  const double denom_raw = 1.0 - std::exp(-von_bertalanffy_coefficient_K * (reference_age_for_length_2 - reference_age_for_length_1));
  const double denom = fims_math::ad_max(fims_math::ad_fabs(denom_raw), 1e-8);
  const double L = mean_length_young + (mean_length_old - mean_length_young) * (1.0 - std::exp(-von_bertalanffy_coefficient_K * (age - reference_age_for_length_1))) / denom;
  const double expected = 2.5e-11 * std::pow(L, 3.0);
  const double W = adapter.evaluate(0, age);

  EXPECT_NEAR(W, expected, 1e-8);
}

TEST(VonBSchnuteGrowthModelAdapter, RejectsFractionalAge) {
  fims_popdy::VonBSchnuteGrowthModelAdapter<double> adapter;
  ConfigureAdapter(adapter, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0, 73.0);
  adapter.SetAgeOffset(1.0);
  adapter.Initialize(1, 12, 1);

  EXPECT_THROW(adapter.evaluate(0, 5.5), std::runtime_error);
}

TEST(VonBSchnuteGrowthModelAdapter, RejectsNegativeAge) {
  fims_popdy::VonBSchnuteGrowthModelAdapter<double> adapter;
  ConfigureAdapter(adapter, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0, 73.0);
  adapter.SetAgeOffset(1.0);
  adapter.Initialize(1, 12, 1);

  EXPECT_THROW(adapter.evaluate(0, -1.0), std::runtime_error);
}

TEST(VonBSchnuteGrowthModelAdapter, ExtrapolatesAboveCachedAgeRange) {
  fims_popdy::VonBSchnuteGrowthModelAdapter<double> adapter;
  ConfigureAdapter(adapter, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0, 73.0);
  adapter.SetAgeOffset(1.0);
  adapter.Initialize(1, 12, 1);

  const double age = 13.0;
  const double mean_length_young = 275.0;
  const double mean_length_old = 725.0;
  const double von_bertalanffy_coefficient_K = 0.18;
  const double reference_age_for_length_1 = 1.0;
  const double reference_age_for_length_2 = 12.0;
  const double denom_raw = 1.0 - std::exp(-von_bertalanffy_coefficient_K * (reference_age_for_length_2 - reference_age_for_length_1));
  const double denom = fims_math::ad_max(fims_math::ad_fabs(denom_raw), 1e-8);
  const double L = mean_length_young + (mean_length_old - mean_length_young) * (1.0 - std::exp(-von_bertalanffy_coefficient_K * (age - reference_age_for_length_1))) / denom;
  const double expected = 2.5e-11 * std::pow(L, 3.0);

  const double W = adapter.evaluate(0, age);
  EXPECT_NEAR(W, expected, 1e-8);
}

TEST(VonBSchnuteGrowthModelAdapter, RejectsBothVariabilityPaths) {
  fims_popdy::VonBSchnuteGrowthModelAdapter<double> adapter;
  ConfigureAdapter(adapter, 275.0, 725.0, 0.18, 1.0, 12.0,
                   2.5e-11, 3.0, 28.0, 73.0);

  adapter.LogSdLengthAtRefAge1Vector().resize(1);
  adapter.LogSdLengthAtRefAge2Vector().resize(1);
  adapter.LogSdGrowthCoefficientKVector().resize(1);
  adapter.LogitCorrLengthAtRefAge1LengthAtRefAge2Vector().resize(1);
  adapter.LogitCorrLengthAtRefAge1KVector().resize(1);
  adapter.LogitCorrLengthAtRefAge2KVector().resize(1);

  adapter.LogSdLengthAtRefAge1Vector()[0] = fims_math::log(0.1);
  adapter.LogSdLengthAtRefAge2Vector()[0] = fims_math::log(0.1);
  adapter.LogSdGrowthCoefficientKVector()[0] = fims_math::log(0.1);
  adapter.LogitCorrLengthAtRefAge1LengthAtRefAge2Vector()[0] = 0.0;
  adapter.LogitCorrLengthAtRefAge1KVector()[0] = 0.0;
  adapter.LogitCorrLengthAtRefAge2KVector()[0] = 0.0;

  adapter.SetAgeOffset(1.0);

  try {
    adapter.Initialize(1, 12, 1);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_NE(std::string(e.what()).find("exactly one supported path"),
              std::string::npos);
  }
}

}  // namespace
