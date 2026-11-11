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
  adapter.mean_length_young_vector.resize(1);
  adapter.mean_length_old_vector.resize(1);
  adapter.growth_coefficient_vector.resize(1);
  adapter.reference_age_for_length_young_vector.resize(1);
  adapter.reference_age_for_length_old_vector.resize(1);
  adapter.length_weight_a_vector.resize(1);
  adapter.length_weight_b_vector.resize(1);
  adapter.length_at_age_sd_young_vector.resize(1);
  adapter.length_at_age_sd_old_vector.resize(1);

  // Adapter stores positive growth params on log scale.
  adapter.mean_length_young_vector[0] = fims_math::log(mean_length_young);
  // Estimated older length is stored as the log difference from younger length.
  adapter.mean_length_old_vector[0] =
      fims_math::log(mean_length_old - mean_length_young);
  adapter.growth_coefficient_vector[0] = fims_math::log(growth_coefficient);
  adapter.reference_age_for_length_young_vector[0] =
      reference_age_for_length_young;
  adapter.reference_age_for_length_old_vector[0] =
      reference_age_for_length_old;
  adapter.length_weight_a_vector[0] = fims_math::log(length_weight_a);
  adapter.length_weight_b_vector[0] = fims_math::log(length_weight_b);
  adapter.length_at_age_sd_young_vector[0] =
      fims_math::log(length_at_age_sd_at_reference_age_young);
  adapter.length_at_age_sd_old_vector[0] =
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
  const double denom =
      1.0 - std::exp(-growth_coefficient * (reference_age_for_length_old -
                                            reference_age_for_length_young));
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
  const double denom =
      1.0 - std::exp(-growth_coefficient * (reference_age_for_length_old -
                                            reference_age_for_length_young));
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
  const double denom =
      1.0 - std::exp(-growth_coefficient * (reference_age_for_length_old -
                                            reference_age_for_length_young));
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

TEST(VonBertalanffySchnuteGrowthModelAdapter, RejectsBothVariabilityPaths) {
  fims_popdy::VonBertalanffySchnuteGrowthModelAdapter<double> adapter;
  ConfigureAdapter(adapter, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0,
                   73.0);

  adapter.log_sd_mean_length_young_vector.resize(1);
  adapter.log_sd_mean_length_old_vector.resize(1);
  adapter.log_sd_growth_coefficient_vector.resize(1);
  adapter.mean_length_young_mean_length_old_logit_corr_vector.resize(1);
  adapter.mean_length_young_growth_coefficient_logit_corr_vector.resize(1);
  adapter.mean_length_old_growth_coefficient_logit_corr_vector.resize(1);

  adapter.log_sd_mean_length_young_vector[0] = fims_math::log(0.1);
  adapter.log_sd_mean_length_old_vector[0] = fims_math::log(0.1);
  adapter.log_sd_growth_coefficient_vector[0] = fims_math::log(0.1);
  adapter.mean_length_young_mean_length_old_logit_corr_vector[0] = 0.0;
  adapter.mean_length_young_growth_coefficient_logit_corr_vector[0] = 0.0;
  adapter.mean_length_old_growth_coefficient_logit_corr_vector[0] = 0.0;

  adapter.SetAgeOffset(1.0);

  try {
    adapter.Initialize(1, 12, 1);
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    EXPECT_NE(std::string(e.what()).find("exactly one supported path"),
              std::string::npos);
  }
}


TEST(VonBertalanffySchnuteGrowthModelAdapter,
     PreservesLengthOrderingAndHandlesTinyGaps) {
  for (bool young_is_constant : {false, true}) {
    SCOPED_TRACE(young_is_constant);
    fims_popdy::VonBertalanffySchnuteGrowthModelAdapter<double> adapter;
    ConfigureAdapter(adapter, 275.0, 725.0, 0.18, 1.0, 12.0,
                     2.5e-11, 3.0, 28.0, 73.0);

    if (young_is_constant) {
      adapter.UseConstantMeanLengthYoungWithEstimatedMeanLengthOld(275.0);
    } else {
      adapter.UseEstimatedReferenceMeanLengths();
    }

    adapter.SetAgeOffset(1.0);
    adapter.Initialize(1, 12, 1);

    // Change working values after initialization, as optimization would.
    adapter.MeanLengthYoungVector()[0] = fims_math::log(800.0);
    const double expected_young = young_is_constant ? 275.0 : 800.0;

    for (double gap : {1e-20, 0.001, 10.0, 450.0}) {
      SCOPED_TRACE(gap);
      adapter.MeanLengthOldVector()[0] = fims_math::log(gap);
      adapter.PrepareGrowthProducts();

      const auto* products = adapter.TryGetPreparedGrowthProducts();
      ASSERT_NE(products, nullptr);
      EXPECT_NEAR(products->MeanLAA(0, 0, 0), expected_young, 1e-8);
      if (gap == 1e-20) {
        // This gap is too small to distinguish the two physical lengths.
        EXPECT_EQ(products->MeanLAA(0, 11, 0),
                  products->MeanLAA(0, 0, 0));
      } else {
        EXPECT_GT(products->MeanLAA(0, 11, 0),
                  products->MeanLAA(0, 0, 0));
      }
      EXPECT_NEAR(products->MeanLAA(0, 11, 0),
                  expected_young + gap, 1e-8);
      EXPECT_NEAR(products->SdLAA(0, 0, 0), 28.0, 1e-4);
      EXPECT_NEAR(products->SdLAA(0, 11, 0), 73.0, 1e-4);
    }
  }
}

}  // namespace
