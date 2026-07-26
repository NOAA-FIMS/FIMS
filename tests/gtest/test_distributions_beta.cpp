/**
 * @file test_distributions_beta.cpp
 * @brief GoogleTest unit tests for BetaLPDF distribution functor.
 */

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "distributions/functors/beta_lpdf.hpp"

TEST(BetaLPDF, StandardUniformBetaLogDensity) {
  // Beta(1, 1) on (0, 1) is Uniform(0, 1) -> pdf = 1, log_pdf = 0.
  fims_distributions::BetaLPDF<double> beta;
  beta.shape1.resize(1); beta.shape1[0] = 1.0;
  beta.shape2.resize(1); beta.shape2[0] = 1.0;

  beta.observed_values.resize(1);
  beta.observed_values[0] = 0.5;

  beta.expected_values.resize(1);
  beta.expected_values[0] = 0.0;

  beta.input_type = "prior";
  beta.priors.push_back(&(beta.observed_values));

  double lpdf = beta.evaluate();
  EXPECT_NEAR(lpdf, 0.0, 1e-6);
}

TEST(BetaLPDF, SymmetricBetaLogDensity) {
  // Beta(2, 2) on (0, 1): pdf(x) = 6 * x * (1 - x). At x = 0.5 -> pdf = 1.5 -> log_pdf = log(1.5)
  fims_distributions::BetaLPDF<double> beta;
  beta.shape1.resize(1); beta.shape1[0] = 2.0;
  beta.shape2.resize(1); beta.shape2[0] = 2.0;

  beta.observed_values.resize(1);
  beta.observed_values[0] = 0.5;

  beta.expected_values.resize(1);
  beta.expected_values[0] = 0.0;

  beta.input_type = "prior";
  beta.priors.push_back(&(beta.observed_values));

  double lpdf = beta.evaluate();
  double expected_lpdf = std::log(1.5);
  EXPECT_NEAR(lpdf, expected_lpdf, 1e-6);
}

TEST(BetaLPDF, ScaledBetaLogDensity) {
  // Beta(2, 2) on (0, max=10): at x = 5.0 -> u = 0.5 -> pdf(x) = 1.5 / 10 = 0.15 -> log_pdf = log(0.15)
  fims_distributions::BetaLPDF<double> beta;
  beta.shape1.resize(1); beta.shape1[0] = 2.0;
  beta.shape2.resize(1); beta.shape2[0] = 2.0;
  beta.scale.resize(1);  beta.scale[0]  = 10.0;

  beta.observed_values.resize(1);
  beta.observed_values[0] = 5.0;

  beta.expected_values.resize(1);
  beta.expected_values[0] = 0.0;

  beta.input_type = "prior";
  beta.priors.push_back(&(beta.observed_values));

  double lpdf = beta.evaluate();
  double expected_lpdf = std::log(0.15);
  EXPECT_NEAR(lpdf, expected_lpdf, 1e-6);
}
