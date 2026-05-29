#include "gtest/gtest.h"

#include <cmath>

#include "likelihood/likelihood.hpp"

namespace {

TEST(LikelihoodParameter, AppliesLogTransform) {
  fims_likelihood::Parameter<double> sigma(
      "sigma", fims::Vector<double>{std::log(0.4)},
      fims_likelihood::Transform::Log,
      fims_likelihood::EstimationRole::FixedEffect);

  EXPECT_NEAR(sigma.value(), 0.4, 1e-12);
  EXPECT_NEAR(sigma.log_jacobian(), std::log(0.4), 1e-12);
}

TEST(LikelihoodParameter, AppliesBoundedLogitTransform) {
  double lower = 0.2;
  double upper = 1.0;
  double raw = fims_math::logit(lower, upper, 0.75);

  fims_likelihood::Parameter<double> steepness(
      "steepness", fims::Vector<double>{raw},
      fims_likelihood::Transform::Logit,
      fims_likelihood::EstimationRole::FixedEffect);
  steepness.lower_bound = lower;
  steepness.upper_bound = upper;

  EXPECT_NEAR(steepness.value(), 0.75, 1e-12);
}

TEST(LikelihoodValueRef, ReadsVectorsScalarsAndParameters) {
  fims::Vector<double> observed{1.0, 2.0, 3.0};
  fims::Vector<double> scalar_mean{2.0};
  fims_likelihood::Parameter<double> sigma(
      "sigma", fims::Vector<double>{std::log(0.5)},
      fims_likelihood::Transform::Log);

  fims_likelihood::ValueRef<double> x =
      fims_likelihood::vector_ref(observed);
  fims_likelihood::ValueRef<double> mean =
      fims_likelihood::vector_ref(scalar_mean);
  fims_likelihood::ValueRef<double> sd =
      fims_likelihood::parameter_ref(sigma);

  EXPECT_EQ(x.size(), 3);
  EXPECT_EQ(mean[2], 2.0);
  EXPECT_NEAR(sd[0], 0.5, 1e-12);
}

TEST(LikelihoodTerm, EvaluatesNormalLikeDensity) {
  fims::Vector<double> observed{1.0, 2.0, 3.0};
  fims::Vector<double> expected{2.0};
  fims_likelihood::Parameter<double> sigma(
      "sigma", fims::Vector<double>{std::log(1.0)},
      fims_likelihood::Transform::Log);

  auto normal_log_density = [](double x, double mean, double sd) {
    const double pi = 3.14159265358979323846;
    double z = (x - mean) / sd;
    return -0.5 * std::log(2.0 * pi) - std::log(sd) - 0.5 * z * z;
  };

  fims_likelihood::LikelihoodTerm<double> term(
      fims_likelihood::LikelihoodTermType::Data, "normal_data",
      fims_likelihood::vector_ref(observed),
      fims_likelihood::vector_ref(expected),
      fims_likelihood::parameter_ref(sigma), normal_log_density);

  double total = term.evaluate();

  EXPECT_EQ(term.log_density_values.size(), 3);
  EXPECT_NEAR(total, -4.256815599614018, 1e-12);
}

}  // namespace
