#include "gtest/gtest.h"
#include "common/information.hpp"
#include "distributions/distributions.hpp"

namespace
{
  // Test random effects using variable map and double values
  TEST(SetupPriors, UseDoubleValues)
  {
    std::shared_ptr<fims_info::Information<double> > info = 
      fims_info::Information<double>::GetInstance();
    
    // Create new selectivity modules
    std::shared_ptr<fims_popdy::LogisticSelectivity<double> > selectivity1 =
      std::make_shared<fims_popdy::LogisticSelectivity<double> >();
    selectivity1->inflection_point.resize(1);
    selectivity1->slope.resize(1);
    selectivity1->inflection_point[0] = 19.7;
    selectivity1->slope[0] = 0.21;

    std::shared_ptr<fims_popdy::LogisticSelectivity<double> > selectivity2 =
      std::make_shared<fims_popdy::LogisticSelectivity<double> >();
    selectivity2->inflection_point.resize(1);
    selectivity2->slope.resize(1);
    selectivity2->inflection_point[0] = 22.3;
    selectivity2->slope[0] = 0.18;
    
    info->variable_map[1] = &(selectivity1)->inflection_point;
    info->variable_map[2] = &(selectivity1)->slope;
    info->variable_map[3] = &(selectivity2)->inflection_point;
    info->variable_map[4] = &(selectivity2)->slope;
    
    //Create new normal distributions
    std::shared_ptr<fims_distributions::NormalLPDF<double> > normal_inflection_point =
    std::make_shared<fims_distributions::NormalLPDF<double> >();
    std::shared_ptr<fims_distributions::NormalLPDF<double> > normal_slope =
      std::make_shared<fims_distributions::NormalLPDF<double> >();
    info->density_components[1] = normal_inflection_point;
    info->density_components[2] = normal_slope;
    normal_inflection_point->key.resize(2);
    normal_inflection_point->key[0] = 1;
    normal_inflection_point->key[1] = 3;
    normal_inflection_point->input_type = "prior";
    normal_slope->key.resize(2);
    normal_slope->key[0] = 2;
    normal_slope->key[1] = 4;
    normal_slope->input_type = "prior";
    
    info->SetupPriors();

    EXPECT_EQ((*normal_inflection_point->priors[0])[0], selectivity1->inflection_point[0]);
    EXPECT_EQ((*normal_inflection_point->priors[1])[0], selectivity2->inflection_point[0]);
    EXPECT_EQ((*normal_slope->priors[0])[0], selectivity1->slope[0]);
    EXPECT_EQ((*normal_slope->priors[1])[0], selectivity2->slope[0]);
    EXPECT_EQ((normal_inflection_point->get_observed(0)), selectivity1->inflection_point[0]);
    EXPECT_EQ((normal_inflection_point->get_observed(1)), selectivity2->inflection_point[0]);
    EXPECT_EQ((normal_slope->get_observed(0)), selectivity1->slope[0]);
    EXPECT_EQ((normal_slope->get_observed(1)), selectivity2->slope[0]);
    
    //update the value in normal to check if it is updated in selectivity
    (*normal_inflection_point->priors[0])[0] = 20.5;
    (*normal_slope->priors[0])[0] = 0.13;
    EXPECT_EQ(selectivity1->inflection_point[0], 20.5);
    EXPECT_EQ(selectivity1->slope[0], 0.13);
  }
}