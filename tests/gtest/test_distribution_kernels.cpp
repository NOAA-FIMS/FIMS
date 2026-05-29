#include "gtest/gtest.h"

#include <cmath>

#include "distributions/kernels/distribution_kernels.hpp"
#include "likelihood/likelihood.hpp"

namespace {

TEST(DistributionKernels, NormalLogDensityMatchesKnownValue) {
  double value =
      fims_distributions::kernels::Normal<double>::log_density(1.0, 0.0, 2.0);

  EXPECT_NEAR(value, -1.737085713764618, 1e-12);
}

TEST(DistributionKernels, LognormalLogDensityIncludesJacobian) {
  double value = fims_distributions::kernels::LogNormal<double>::log_density(
      2.0, 0.0, 0.5);

  EXPECT_NEAR(value, -1.8798445610410757, 1e-12);
}

TEST(DistributionKernels, LognormalLogScaleDensityOmitsJacobian) {
  double value =
      fims_distributions::kernels::LogNormal<double>::log_density_log_scale(
          2.0, 0.0, 0.5);

  EXPECT_NEAR(value, -1.1866973804811303, 1e-12);
}

TEST(DistributionKernels, NormalKernelWorksWithLikelihoodTerm) {
  fims::Vector<double> observed{1.0, 2.0, 3.0};
  fims::Vector<double> expected{2.0};
  fims_likelihood::Parameter<double> sigma(
      "sigma", fims::Vector<double>{std::log(1.0)},
      fims_likelihood::Transform::Log);

  fims_likelihood::LikelihoodTerm<double> term(
      fims_likelihood::LikelihoodTermType::Data, "normal_data",
      fims_likelihood::vector_ref(observed),
      fims_likelihood::vector_ref(expected),
      fims_likelihood::parameter_ref(sigma),
      fims_distributions::kernels::Normal<double>::log_density);

  EXPECT_NEAR(term.evaluate(), -3.756815599614018, 1e-12);
}

}  // namespace
