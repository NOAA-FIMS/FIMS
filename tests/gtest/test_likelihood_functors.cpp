#include <cmath>

#include "gtest/gtest.h"
#include "likelihood/likelihood.hpp"

namespace {

const double kLogTwoPi = 1.8378770664093453;

TEST(LikelihoodFunctors, NormalEvaluatesNegativeLogLikelihood) {
  fims_likelihood::NormalLikelihood<double> likelihood;
  likelihood.observed_values = {1.0, 2.0};
  likelihood.expected_values = {0.5, 1.5};
  likelihood.log_sd = {std::log(2.0)};

  double actual = likelihood.Evaluate();
  double expected = 0.0;
  for (size_t i = 0; i < 2; i++) {
    double residual =
        likelihood.observed_values[i] - likelihood.expected_values[i];
    expected += 0.5 * (kLogTwoPi + 2.0 * std::log(2.0) +
                       (residual * residual) / 4.0);
  }

  EXPECT_NEAR(actual, expected, 1e-10);
  EXPECT_EQ(likelihood.nll_components.size(), 2);
}

TEST(LikelihoodFunctors, NormalUsesLinkedInput) {
  fims::Vector<double> input = {1.0};
  fims_likelihood::NormalLikelihood<double> likelihood;
  likelihood.SetInput(&input, fims_likelihood::LikelihoodRole::Prior);
  likelihood.expected_values = {0.0};
  likelihood.log_sd = {0.0};

  EXPECT_NEAR(likelihood.Evaluate(), 0.5 * kLogTwoPi + 0.5, 1e-10);
}

TEST(LikelihoodFunctors, LognormalEvaluatesNegativeLogLikelihood) {
  fims_likelihood::LognormalLikelihood<double> likelihood;
  likelihood.observed_values = {std::exp(1.0)};
  likelihood.expected_values = {0.5};
  likelihood.log_sd = {std::log(2.0)};

  double residual = 1.0 - 0.5;
  double expected = 0.5 * (kLogTwoPi + 2.0 * std::log(2.0) +
                           (residual * residual) / 4.0) +
                    1.0;

  EXPECT_NEAR(likelihood.Evaluate(), expected, 1e-10);
}

TEST(LikelihoodFunctors, GammaEvaluatesNegativeLogLikelihood) {
  fims_likelihood::GammaLikelihood<double> likelihood;
  likelihood.observed_values = {4.0};
  likelihood.expected_values = {3.0};
  likelihood.log_sd = {std::log(1.5)};

  double mean = 3.0;
  double sd = 1.5;
  double shape = (mean / sd) * (mean / sd);
  double scale = (sd * sd) / mean;
  double log_density = (shape - 1.0) * std::log(4.0) - 4.0 / scale -
                       std::lgamma(shape) - shape * std::log(scale);

  EXPECT_NEAR(likelihood.Evaluate(), -log_density, 1e-10);
}

TEST(LikelihoodFunctors, InvGammaEvaluatesNegativeLogLikelihood) {
  fims_likelihood::InvGammaLikelihood<double> likelihood;
  likelihood.observed_values = {2.0};
  likelihood.log_shape = {std::log(3.0)};
  likelihood.log_scale = {std::log(4.0)};

  double shape = 3.0;
  double scale = 4.0;
  double log_density = -shape * std::log(scale) - std::lgamma(shape) -
                       (shape + 1.0) * std::log(2.0) - 1.0 / (scale * 2.0);

  EXPECT_NEAR(likelihood.Evaluate(), -log_density, 1e-10);
}

TEST(LikelihoodFunctors, MultinomialEvaluatesNegativeLogLikelihood) {
  fims_likelihood::MultinomialLikelihood<double> likelihood;
  likelihood.observed_values = {2.0, 1.0, 1.0};
  likelihood.expected_values = {0.5, 0.25, 0.25};
  likelihood.dims = {1, 3};

  double log_density = std::lgamma(5.0) - std::lgamma(3.0) -
                       std::lgamma(2.0) - std::lgamma(2.0) +
                       2.0 * std::log(0.5) + std::log(0.25) +
                       std::log(0.25);

  EXPECT_NEAR(likelihood.Evaluate(), -log_density, 1e-10);
}

}  // namespace
