#include "gtest/gtest.h"

#include <cmath>
#include <memory>

#include "common/information.hpp"
#include "distributions/kernels/distribution_kernels.hpp"

namespace {

TEST(InformationLikelihoodTerms, StoresAndClearsLikelihoodTerms) {
  std::shared_ptr<fims_info::Information<double>> info =
      fims_info::Information<double>::GetInstance();
  info->Clear();

  fims::Vector<double> observed{1.0, 2.0, 3.0};
  fims_likelihood::Parameter<double> sigma(
      "sigma", fims::Vector<double>{std::log(1.0)},
      fims_likelihood::Transform::Log);

  std::shared_ptr<fims_likelihood::LikelihoodTerm<double>> term =
      std::make_shared<fims_likelihood::LikelihoodTerm<double>>(
          fims_likelihood::LikelihoodTermType::Data, "normal_data",
          fims_likelihood::vector_ref(observed),
          fims_likelihood::constant_ref(2.0),
          fims_likelihood::parameter_ref(sigma),
          fims_distributions::kernels::Normal<double>::log_density);

  info->likelihood_terms.push_back(term);

  EXPECT_EQ(info->likelihood_terms.size(), 1);
  EXPECT_NEAR(info->likelihood_terms[0]->evaluate(), -3.756815599614018,
              1e-12);

  info->Clear();

  EXPECT_EQ(info->likelihood_terms.size(), 0);
}

}  // namespace
