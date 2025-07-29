#include "gtest/gtest.h"
#include "common/information.hpp"
#include "distributions/distributions.hpp"

namespace
{
  // Test random effects using variable map and double values
  TEST(SetupPriors, UseDoubleValues)
  {
    // Create pointer to information
    std::shared_ptr<fims_info::Information<double> > info = 
      fims_info::Information<double>::GetInstance();
    
    // Create new selectivity modules and set parameter values
    std::shared_ptr<fims_popdy::LogisticSelectivity<double> > selectivity1 =
      std::make_shared<fims_popdy::LogisticSelectivity<double> >();
    selectivity1->inflection_point.resize(1);
    selectivity1->slope.resize(2);
    selectivity1->inflection_point[0] = 19.7;
    selectivity1->slope[0] = 0.21;
    selectivity1->slope[1] = 0.25;

    std::shared_ptr<fims_popdy::LogisticSelectivity<double> > selectivity2 =
      std::make_shared<fims_popdy::LogisticSelectivity<double> >();
    selectivity2->inflection_point.resize(1);
    selectivity2->slope.resize(2);
    selectivity2->inflection_point[0] = 22.3;
    selectivity2->slope[0] = 0.18;
    selectivity2->slope[1] = 0.2;
    
    // Set up variable map to point to selectivity parameters
    info->variable_map[1] = &(selectivity1)->inflection_point;
    info->variable_map[2] = &(selectivity1)->slope;
    info->variable_map[3] = &(selectivity2)->inflection_point;
    info->variable_map[4] = &(selectivity2)->slope;
    
    //Create new normal distributions
    std::shared_ptr<fims_distributions::NormalLPDF<double> > normal_inflection_point =
    std::make_shared<fims_distributions::NormalLPDF<double> >();
    std::shared_ptr<fims_distributions::LogNormalLPDF<double> > lognormal_slope =
      std::make_shared<fims_distributions::LogNormalLPDF<double> >();
    // Set up pointers in information to point to each density module
    info->density_components[1] = normal_inflection_point;
    info->density_components[2] = lognormal_slope;
    // Set up inflection point key. Keys 1 and 3 will reference variable map 1 and 3
    normal_inflection_point->key.resize(2);
    normal_inflection_point->key[0] = 1;
    normal_inflection_point->key[1] = 3;
    normal_inflection_point->input_type = "prior";
    // Set up slope key. Keys 2 and 4 will reference variable map 2 adn 4
    lognormal_slope->key.resize(2);
    lognormal_slope->key[0] = 2;
    lognormal_slope->key[1] = 4;
    lognormal_slope->input_type = "prior";
    
    // Call function that links key ID to variable map pointers given variable map ID
    // This function will set the density component, priors, to the respective parameters
    info->SetupPriors();

    EXPECT_EQ((*normal_inflection_point->priors[0])[0], selectivity1->inflection_point[0]);
    EXPECT_EQ((*normal_inflection_point->priors[1])[0], selectivity2->inflection_point[0]);
    EXPECT_EQ((*lognormal_slope->priors[0])[0], selectivity1->slope[0]);
    EXPECT_EQ((*lognormal_slope->priors[0])[1], selectivity1->slope[1]);
    EXPECT_EQ((*lognormal_slope->priors[1])[0], selectivity2->slope[0]);
    EXPECT_EQ((*lognormal_slope->priors[1])[1], selectivity2->slope[1]);
    EXPECT_EQ((normal_inflection_point->get_observed(0)), selectivity1->inflection_point[0]);
    EXPECT_EQ((normal_inflection_point->get_observed(1)), selectivity2->inflection_point[0]);
    EXPECT_EQ((lognormal_slope->get_observed(0)), selectivity1->slope[0]);
    EXPECT_EQ((lognormal_slope->get_observed(1)), selectivity2->slope[0]);
    
    //update the value in normal to check if it is updated in selectivity
    (*normal_inflection_point->priors[0])[0] = 20.5;
    (*lognormal_slope->priors[0])[0] = 0.13;
    EXPECT_EQ(selectivity1->inflection_point[0], 20.5);
    EXPECT_EQ(selectivity1->slope[0], 0.13);
  }
}