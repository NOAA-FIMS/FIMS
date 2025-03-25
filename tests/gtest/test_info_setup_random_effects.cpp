#include "gtest/gtest.h"
#include "common/information.hpp"
#include "distributions/distributions.hpp"

namespace
{
  // Test random effects using variable map and double values
  TEST(SetupRandomEffects, UseDoubleValues)
  {
    std::shared_ptr<fims_info::Information<double> > info = 
      fims_info::Information<double>::GetInstance();
    
    // Create a new recruitment module
    std::shared_ptr<fims_popdy::SRBevertonHolt<double> > recruitment =
      std::make_shared<fims_popdy::SRBevertonHolt<double> >();
    recruitment->log_r.resize(2);
    recruitment->log_r[0] = 3.2;
    recruitment->log_r[1] = 4.1;
    recruitment->log_expected_recruitment.resize(2);
    recruitment->log_expected_recruitment[0] = 2.1;
    recruitment->log_expected_recruitment[1] = -1.7;
    info->variable_map[1] = &(recruitment)->log_r;
    info->variable_map[2] = &(recruitment)->log_expected_recruitment;
    
    //Create a new normal distribution
    std::shared_ptr<fims_distributions::NormalLPDF<double> > normal =
      std::make_shared<fims_distributions::NormalLPDF<double> >();
    info->density_components[1] = normal;
    normal->key.resize(2);
    normal->key[0] = 1;
    normal->key[1] = 2;
    normal->input_type = "random_effects";
    
    info->SetupRandomEffects();

    for(size_t i=0; i<recruitment->log_r.size(); i++){
      EXPECT_EQ((*normal->re)[i], recruitment->log_r[i]);
      EXPECT_EQ((*normal->re_expected_values)[i], recruitment->log_expected_recruitment[i]);
    }

    //update the value in normal to check if it is updated in recruitment
    (*normal->re)[0] = 4.3;
    (*normal->re_expected_values)[0] = 5.6;
    EXPECT_EQ(recruitment->log_r[0], 4.3);
    EXPECT_EQ(recruitment->log_expected_recruitment[0], 5.6);
  }
}