/**
 * @file test_distributions_mvnorm.cpp
 * @brief GoogleTest unit tests for MVNormLPDF distribution functor.
 */

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "distributions/functors/mvnorm_lpdf.hpp"

TEST(MVNormLPDF, BivariateStandardNormalLogDensity) {
  // Test bivariate standard normal distribution with mean (0, 0) and identity covariance
  fims_distributions::MVNormLPDF<double> mvnorm;
  mvnorm.k_dim = 2;

  mvnorm.sigma_mat.resize(4);
  mvnorm.sigma_mat[0] = 1.0; mvnorm.sigma_mat[1] = 0.0;
  mvnorm.sigma_mat[2] = 0.0; mvnorm.sigma_mat[3] = 1.0;

  mvnorm.observed_values.resize(2);
  mvnorm.observed_values[0] = 0.0;
  mvnorm.observed_values[1] = 0.0;

  mvnorm.expected_values.resize(2);
  mvnorm.expected_values[0] = 0.0;
  mvnorm.expected_values[1] = 0.0;

  mvnorm.input_type = "prior";
  mvnorm.priors.push_back(&(mvnorm.observed_values));

  double lpdf = mvnorm.evaluate();

  // Analytical bivariate standard normal at (0, 0):
  // -k/2 * log(2*pi) - 0.5 * log(1) - 0.5 * 0 = -log(2*pi) ≈ -1.837877
  double expected_lpdf = -std::log(2.0 * M_PI);
  EXPECT_NEAR(lpdf, expected_lpdf, 1e-6);
}

TEST(MVNormLPDF, CorrelatedBivariateNormalLogDensity) {
  // Test bivariate normal with covariance Sigma = [[2, 1], [1, 2]]
  // det(Sigma) = 3
  fims_distributions::MVNormLPDF<double> mvnorm;
  mvnorm.k_dim = 2;

  mvnorm.sigma_mat.resize(4);
  mvnorm.sigma_mat[0] = 2.0; mvnorm.sigma_mat[1] = 1.0;
  mvnorm.sigma_mat[2] = 1.0; mvnorm.sigma_mat[3] = 2.0;

  mvnorm.observed_values.resize(2);
  mvnorm.observed_values[0] = 1.0;
  mvnorm.observed_values[1] = 1.0;

  mvnorm.expected_values.resize(2);
  mvnorm.expected_values[0] = 0.0;
  mvnorm.expected_values[1] = 0.0;

  mvnorm.input_type = "prior";
  mvnorm.priors.push_back(&(mvnorm.observed_values));

  double lpdf = mvnorm.evaluate();

  // Quad form: x = [1, 1], Sigma^-1 = 1/3 * [[2, -1], [-1, 2]]
  // x^T Sigma^-1 x = 1/3 * (2 - 1 - 1 + 2) = 2/3
  // Expected lpdf = -0.5 * (2/3 + log(3) + 2*log(2*pi))
  double expected_lpdf = -0.5 * (2.0 / 3.0 + std::log(3.0) + 2.0 * std::log(2.0 * M_PI));
  EXPECT_NEAR(lpdf, expected_lpdf, 1e-5);
}
