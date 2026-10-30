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
     LogLengthGradientsMatchFiniteDifferences) {
  // Small lengths exercise the smoothing; ordinary lengths check its limit.
  for (const double asymptotic_length : {100.0, 0.001}) {
    SCOPED_TRACE(asymptotic_length);
    auto vb = MakeValidTraditionalVonB();
    vb.asymptotic_length = asymptotic_length;

    const double age = 5.0;
    const double relative_step = 1e-5;
    const double linf_step = asymptotic_length * relative_step;
    const double k_step = vb.growth_coefficient * relative_step;
    const double t0_step = 1e-5;

    double natural_linf = 0.0;
    double natural_k = 0.0;
    double natural_t0 = 0.0;
    vb.log_length_at_age_gradient(
        age, natural_linf, natural_k, natural_t0);

    auto plus = vb;
    auto minus = vb;
    plus.asymptotic_length += linf_step;
    minus.asymptotic_length -= linf_step;
    const double numerical_linf =
        (plus.log_length_at_age(age) - minus.log_length_at_age(age)) /
        (2.0 * linf_step);

    plus = vb;
    minus = vb;
    plus.growth_coefficient += k_step;
    minus.growth_coefficient -= k_step;
    const double numerical_k =
        (plus.log_length_at_age(age) - minus.log_length_at_age(age)) /
        (2.0 * k_step);

    plus = vb;
    minus = vb;
    plus.age_at_zero_length += t0_step;
    minus.age_at_zero_length -= t0_step;
    const double numerical_t0 =
        (plus.log_length_at_age(age) - minus.log_length_at_age(age)) /
        (2.0 * t0_step);

    EXPECT_NEAR(natural_linf, numerical_linf,
                1e-6 * (1.0 + std::abs(numerical_linf)));
    EXPECT_NEAR(natural_k, numerical_k,
                1e-6 * (1.0 + std::abs(numerical_k)));
    EXPECT_NEAR(natural_t0, numerical_t0,
                1e-6 * (1.0 + std::abs(numerical_t0)));

    double working_linf = 0.0;
    double working_k = 0.0;
    double working_t0 = 0.0;
    vb.log_length_at_age_working_scale_gradient(
        age, working_linf, working_k, working_t0);

    EXPECT_NEAR(working_linf,
                numerical_linf * vb.asymptotic_length, 1e-6);
    EXPECT_NEAR(working_k,
                numerical_k * vb.growth_coefficient, 1e-6);
    EXPECT_NEAR(working_t0, numerical_t0, 1e-6);
  }
}

TEST(VonBertalanffyTraditionalEvaluate, RejectsAgeBelowT0) {
  auto vb = MakeValidTraditionalVonB();
  vb.age_at_zero_length = 1.0;

  EXPECT_THROW(vb.length_at_age(0.0), std::runtime_error);
  EXPECT_THROW(vb.weight_at_age(0.0), std::runtime_error);
  EXPECT_THROW(vb.evaluate(0, 0.0), std::runtime_error);
  EXPECT_THROW(vb.log_length_at_age(0.0), std::runtime_error);

  double derivative_linf = 0.0;
  double derivative_k = 0.0;
  double derivative_t0 = 0.0;

  EXPECT_THROW(
      vb.log_length_at_age_gradient(
          0.0, derivative_linf, derivative_k, derivative_t0),
      std::runtime_error);
  EXPECT_THROW(
      vb.log_length_at_age_working_scale_gradient(
          0.0, derivative_linf, derivative_k, derivative_t0),
      std::runtime_error);
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
