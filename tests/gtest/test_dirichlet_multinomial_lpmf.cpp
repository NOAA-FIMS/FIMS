#include <cmath>
#include "distributions/functors/dirichlet_multinomial_lpmf.hpp"
#include "gtest/gtest.h"
#include "test_stubs.hpp"

TEST(DirichletMultinomial, MissingRowsAndNormalizedContributions) {
  fims_distributions::DirichletMultinomialLPMF<double> d;
  d.input_type = "data";
  d.log_concentration = {std::log(10.0)};
  d.data_observed_values =
      std::make_shared<fims_data_object::DataObject<double>>(2, 2);
  d.data_observed_values->data = {2.0, 3.0, -999.0, 4.0};
  fims::Vector<double> expected = {.4, .6, .4, .6};
  d.data_expected_values = &expected;
  const double beta_ratio = std::lgamma(6.0) + std::lgamma(9.0) -
                            std::lgamma(15.0) - std::lgamma(4.0) -
                            std::lgamma(6.0) + std::lgamma(10.0);
  const double expected_log = std::log(10.0) + beta_ratio;
  EXPECT_NEAR(d.evaluate(), expected_log, 1e-12);
  EXPECT_NEAR(d.lpdf_vec[0] + d.lpdf_vec[1], d.lpdf, 1e-12);
  EXPECT_EQ(d.lpdf_vec[2], 0.0);
  EXPECT_EQ(d.lpdf_vec[3], 0.0);
}
