#include "gtest/gtest.h"
#include "population_dynamics/fleet/fleet.hpp"
#include "distributions/distributions.hpp"

namespace
{

  //this demostrates how to set up to test the structure of fleets.
  // It won't work in gtest because the Dlnorm functions are only available if
  // TMB is also available. This test instead should be done as a testthat rcpp
  // test rather than gtest. However, we are leaving this file to provide an example
  // of the approach.
  TEST(fleet, create_object)
  {
    //set up selectivity
    fims::LogisticSelectivity<double> fishery_selectivity;
    fishery_selectivity.median = 20.5;
    fishery_selectivity.slope = 0.2;
    double fishery_x = 40.5;
    // 1.0/(1.0+exp(-(40.5-20.5)*0.2)) = 0.9820138
    double expect_fishery = 0.9820138;

    //set up index likelihood
    fims::Dlnorm<T> nll_dlnorm;
    nll_dlnorm.logy = 2.356247;
    nll_dlnorm.meanlog = 0;
    nll_dlnorm.sdlog = 1;

    //set up agecomp likelihood
    fims::Dmultinom<T> nll_dmultinom;
    nll_dmultinom.x = {2, 3, 7};
    nll_dmultinom.p = {.1, .3, .6};
    
    fims::Fleet<T> f;
    f.selectivity = fishery_selectivity
    f.observed_index_data = 2.356247;
    f.observed_agecomp_data = {2,3,7}
    f.index_likelihood = nll_dlnorm.evaluate(log=true);
    f.agecomp_likelihood = nll_dmultinom.evaluate(log=true);
    f.evaluate();

    EXPECT_EQ(1, 1);
  }

}