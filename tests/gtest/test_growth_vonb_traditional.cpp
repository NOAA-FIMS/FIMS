#include <cmath>
#include "gtest/gtest.h"
#include "common/fims_math.hpp"
#include "population_dynamics/growth/functors/vonb_traditional.hpp"

namespace {

fims_popdy::VonBertalanffyTraditionalGrowth<double> MakeValidTraditionalVonB() {
  fims_popdy::VonBertalanffyTraditionalGrowth<double> vb;

  vb.asymptotic_length = 100.0;
  vb.growth_coefficient = 0.2;
  vb.age_at_zero_length = -0.5;
  vb.length_weight_a = 1.0e-5;
  vb.length_weight_b = 3.0;

  return vb;
}

TEST(VonBertalanffyTraditionalEvaluate, BasicSanity) {
  auto vb = MakeValidTraditionalVonB();

  const double age = 5.0;
  const double expected_length =
      vb.asymptotic_length *
      (1.0 - std::exp(-vb.growth_coefficient *
                      (age - vb.age_at_zero_length)));

  const double length_at_age = vb.length_at_age(age);

  EXPECT_NEAR(length_at_age, expected_length, 1e-12);
  EXPECT_GT(length_at_age, vb.length_at_age(1.0));
  EXPECT_LT(length_at_age, vb.asymptotic_length);

  const double expected_weight =
      vb.length_weight_a * std::pow(length_at_age, vb.length_weight_b);

  EXPECT_NEAR(vb.weight_at_age(age), expected_weight, 1e-12);
  EXPECT_NEAR(vb.evaluate(0, age), expected_weight, 1e-12);
}

TEST(VonBertalanffyTraditionalEvaluate,
     WorkingScaleLogLengthGradientMatchesAnalyticalResult) {
  auto vb = MakeValidTraditionalVonB();

  const double age = 5.0;
  const double age_delta = age - vb.age_at_zero_length;
  const double survival_like =
      std::exp(-vb.growth_coefficient * age_delta);
  const double one_minus_survival_like = 1.0 - survival_like;

  double d_log_laa_d_log_asymptotic_length = 0.0;
  double d_log_laa_d_log_growth_coefficient = 0.0;
  double d_log_laa_d_age_at_zero_length = 0.0;

  vb.log_length_at_age_working_scale_gradient(
      age, d_log_laa_d_log_asymptotic_length,
      d_log_laa_d_log_growth_coefficient,
      d_log_laa_d_age_at_zero_length);

  const double length = vb.asymptotic_length * one_minus_survival_like;
  const double length_safe = fims_math::ad_max(length, 1e-8);

  EXPECT_NEAR(d_log_laa_d_log_asymptotic_length,
              vb.asymptotic_length * one_minus_survival_like / length_safe,
              1e-12);
  EXPECT_NEAR(d_log_laa_d_log_growth_coefficient,
              vb.asymptotic_length * vb.growth_coefficient * age_delta *
                  survival_like / length_safe,
              1e-12);
  EXPECT_NEAR(d_log_laa_d_age_at_zero_length,
              -vb.asymptotic_length * vb.growth_coefficient * survival_like /
                  length_safe,
              1e-12);
}

TEST(VonBertalanffyTraditionalEvaluate, RejectsAgeBelowT0) {
  auto vb = MakeValidTraditionalVonB();
  vb.age_at_zero_length = 1.0;

  EXPECT_ANY_THROW(vb.length_at_age(0.0));
  EXPECT_ANY_THROW(vb.weight_at_age(0.0));
  EXPECT_ANY_THROW(vb.evaluate(0, 0.0));
}

TEST(VonBertalanffyTraditionalEvaluate, AllowsPositiveT0AtAndAboveBoundary) {
  auto vb = MakeValidTraditionalVonB();
  vb.age_at_zero_length = 1.0;

  EXPECT_DOUBLE_EQ(vb.length_at_age(1.0), 0.0);
  EXPECT_DOUBLE_EQ(vb.weight_at_age(1.0), 0.0);

  const double expected_length = 100.0 * (1.0 - std::exp(-0.2));
  EXPECT_NEAR(vb.length_at_age(2.0), expected_length, 1e-12);
}

TEST(VonBertalanffyTraditionalEvaluate,
     NaturalLengthGradientMatchesFiniteDifferences) {
  auto vb = MakeValidTraditionalVonB();
  const double age = 5.0;
  const double step = 1e-5;

  double d_laa_d_log_asymptotic_length = 0.0;
  double d_laa_d_log_growth_coefficient = 0.0;
  double d_laa_d_age_at_zero_length = 0.0;
  vb.length_at_age_working_scale_gradient(
      age, d_laa_d_log_asymptotic_length,
      d_laa_d_log_growth_coefficient, d_laa_d_age_at_zero_length);

  auto plus = vb;
  auto minus = vb;
  plus.asymptotic_length *= std::exp(step);
  minus.asymptotic_length *= std::exp(-step);
  const double numerical_linf_derivative =
      (plus.length_at_age(age) - minus.length_at_age(age)) / (2.0 * step);

  plus = vb;
  minus = vb;
  plus.growth_coefficient *= std::exp(step);
  minus.growth_coefficient *= std::exp(-step);
  const double numerical_k_derivative =
      (plus.length_at_age(age) - minus.length_at_age(age)) / (2.0 * step);

  plus = vb;
  minus = vb;
  plus.age_at_zero_length += step;
  minus.age_at_zero_length -= step;
  const double numerical_t0_derivative =
      (plus.length_at_age(age) - minus.length_at_age(age)) / (2.0 * step);

  EXPECT_NEAR(d_laa_d_log_asymptotic_length, numerical_linf_derivative, 1e-7);
  EXPECT_NEAR(d_laa_d_log_growth_coefficient, numerical_k_derivative, 1e-7);
  EXPECT_NEAR(d_laa_d_age_at_zero_length, numerical_t0_derivative, 1e-7);
}

TEST(VonBertalanffyTraditionalEvaluate,
     NaturalLengthGradientHandlesAgeBoundary) {
  auto vb = MakeValidTraditionalVonB();
  vb.age_at_zero_length = 1.0;

  double d_laa_d_log_asymptotic_length = 123.0;
  double d_laa_d_log_growth_coefficient = 123.0;
  double d_laa_d_age_at_zero_length = 123.0;

  vb.length_at_age_working_scale_gradient(
      1.0, d_laa_d_log_asymptotic_length,
      d_laa_d_log_growth_coefficient, d_laa_d_age_at_zero_length);

  EXPECT_DOUBLE_EQ(d_laa_d_log_asymptotic_length, 0.0);
  EXPECT_DOUBLE_EQ(d_laa_d_log_growth_coefficient, 0.0);
  EXPECT_DOUBLE_EQ(d_laa_d_age_at_zero_length, -20.0);

  EXPECT_THROW(
      vb.length_at_age_working_scale_gradient(
          0.0, d_laa_d_log_asymptotic_length,
          d_laa_d_log_growth_coefficient, d_laa_d_age_at_zero_length),
      std::runtime_error);
}

}  // namespace
