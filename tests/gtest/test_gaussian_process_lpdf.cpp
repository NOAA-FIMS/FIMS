#include <cmath>
#include <random>
#include "distributions/functors/gaussian_process_lpdf.hpp"
#include "gtest/gtest.h"
#include "test_stubs.hpp"

namespace {
TEST(GaussianProcessLPDF, StationaryFirstTermAndConditionalTerms) {
  // Independent scalar calculation for a two-state AR1 with a varying mean.
  fims_distributions::GaussianProcessLPDF<double> process;
  process.ar1 = true;
  process.log_sd = {std::log(0.4)};
  process.unconstrained_rho = {std::atanh(-0.6)};
  process.observed_values = {1.3, 0.4};
  process.expected_values = {1.0, 0.5};
  const double initial_sd = 0.5;
  const double innovation = -0.1 - (-0.6) * 0.3;
  const double expected = -std::log(2 * std::acos(-1.0)) -
                          std::log(initial_sd * 0.4) -
                          0.5 * (0.3 * 0.3 / (initial_sd * initial_sd) +
                                 innovation * innovation / (0.4 * 0.4));
  EXPECT_NEAR(process.evaluate(), expected, 1e-12);
  const auto before = process.observed_values;
  EXPECT_DOUBLE_EQ(process.evaluate(), process.evaluate());
  EXPECT_DOUBLE_EQ(process.observed_values[0], before[0]);
}

TEST(GaussianProcessLPDF, LinkedScalarMeanAndInvalidShapes) {
  fims_distributions::GaussianProcessLPDF<double> process;
  process.input_type = "random_effects";
  process.log_sd = {0.0};
  process.unconstrained_rho = {0.0};
  EXPECT_THROW(process.evaluate(), std::invalid_argument);
  fims::Vector<double> target = {2.0, 3.0};
  fims::Vector<double> mean = {1.0};
  process.re = &target;
  process.re_expected_values = &mean;
  EXPECT_NEAR(process.evaluate(), -std::log(2 * std::acos(-1.0)) - 2.5, 1e-12);
  mean.resize(3);
  EXPECT_THROW(process.evaluate(), std::invalid_argument);
}

TEST(GaussianProcessLPDF, SimulatesResidualsWithoutChangingTargets) {
  fims_distributions::GaussianProcessLPDF<double> process;
  process.ar1 = true;
  process.log_sd = {std::log(0.4)};
  process.unconstrained_rho = {std::atanh(0.6)};
  process.observed_values = {0.0, 0.0};
  process.expected_values = {1.0, 2.0};
  // Deterministic standard-normal draws expose initial and transition scales.
  const auto draws = process.Simulate([]() { return 1.0; });
  EXPECT_NEAR(draws[0], 1.5, 1e-12);
  EXPECT_NEAR(draws[1], 2.7, 1e-12);
  EXPECT_DOUBLE_EQ(process.observed_values[0], 0.0);
}
TEST(GaussianProcessLPDF, ContinuesFromFixedResidualHistory) {
  fims_distributions::GaussianProcessLPDF<double> process;
  process.ar1 = true;
  process.log_sd = {std::log(0.4)};
  process.unconstrained_rho = {std::atanh(0.6)};
  process.observed_values = {0.4, 99.0};
  process.expected_values = {0.0};
  const auto draws = process.SimulateFrom(1, []() { return 0.0; });
  EXPECT_DOUBLE_EQ(draws[0], 0.4);
  EXPECT_NEAR(draws[1], 0.24, 1e-12);
  EXPECT_THROW(process.SimulateFrom(3, []() { return 0.0; }),
               std::invalid_argument);
}

TEST(GaussianProcessLPDF, RandomWalkNormalAndFixedAnchor) {
  fims_distributions::GaussianProcessLPDF<double> process;
  process.rw1 = true;
  process.initial_mean = -0.5;
  process.log_initial_sd = std::log(0.4);
  process.log_sd = {std::log(0.2)};
  process.unconstrained_rho = {0.0};
  process.observed_values = {-0.5, -0.3};
  process.expected_values = {0.0};
  const double increment =
      -0.5 * std::log(2 * std::acos(-1.0)) - std::log(0.2) - 0.5;
  const double first = -0.5 * std::log(2 * std::acos(-1.0)) - std::log(0.4);
  EXPECT_NEAR(process.evaluate(), first + increment, 1e-12);
  process.initial_fixed = true;
  EXPECT_NEAR(process.evaluate(), increment, 1e-12);
  const auto mean_path = process.Simulate([]() { return 0.0; });
  EXPECT_DOUBLE_EQ(mean_path[0], -0.5);
  EXPECT_DOUBLE_EQ(mean_path[1], -0.5);
}

}  // namespace
