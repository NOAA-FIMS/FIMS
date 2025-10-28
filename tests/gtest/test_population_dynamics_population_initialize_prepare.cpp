#include "gtest/gtest.h"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{
   
    TEST_F(CAAInitializeTestFixture, input_data_are_specified)
    { 
        this->InitializeCAA();
        EXPECT_EQ(catch_at_age_model->populations[0]->id_g, id_g);
        EXPECT_EQ(catch_at_age_model->populations[0]->n_years, n_years);
        EXPECT_EQ(catch_at_age_model->populations[0]->n_ages, n_ages);
        EXPECT_EQ(catch_at_age_model->populations[0]->fleets.size(), n_fleets);
        EXPECT_EQ(catch_at_age_model->populations[0]->n_fleets, n_fleets);
    }

    TEST_F(CAAInitializeTestFixture, Initialize_works)
    {
        this->InitializeCAA();
        auto& dq = catch_at_age_model->GetPopulationDerivedQuantities(0);
        EXPECT_EQ(dq["mortality_F"].size(), n_years * n_ages);
        EXPECT_EQ(dq["mortality_Z"].size(), n_years * n_ages);
        EXPECT_EQ(dq["proportion_mature_at_age"].size(), (n_years+1) * n_ages);
       // EXPECT_EQ(dq["weight_at_age"].size(), n_ages);
        EXPECT_EQ(dq["unfished_numbers_at_age"].size(), (n_years + 1) * n_ages);
        EXPECT_EQ(dq["numbers_at_age"].size(), (n_years + 1) * n_ages);
        EXPECT_EQ(dq["total_landings_weight"].size(), n_years);
        EXPECT_EQ(dq["total_landings_numbers"].size(), n_years);
        EXPECT_EQ(dq["biomass"].size(), (n_years + 1));
        EXPECT_EQ(dq["unfished_biomass"].size(), (n_years + 1));
        EXPECT_EQ(dq["unfished_spawning_biomass"].size(), (n_years + 1));
        EXPECT_EQ(dq["spawning_biomass"].size(), n_years + 1);
        EXPECT_EQ(catch_at_age_model->populations[0]->proportion_female.size(), n_ages);
        EXPECT_EQ(catch_at_age_model->populations[0]->M.size(), n_years * n_ages);
        EXPECT_EQ(dq["expected_recruitment"].size(), n_years + 1);
        EXPECT_EQ(dq["sum_selectivity"].size(), n_years * n_ages);
    }

    TEST_F(CAAPrepareTestFixture, Prepare_works)
    {
        this->InitializeCAA();
        catch_at_age_model->Prepare();
        auto &dq = catch_at_age_model->GetPopulationDerivedQuantities(1);

        // vector size type = n_years and vector value = 0
        EXPECT_EQ(dq["total_landings_weight"],
            fims::Vector<double>(n_years, 0)
        );
        EXPECT_EQ(dq["total_landings_numbers"],
            fims::Vector<double>(n_years, 0)
        );
        EXPECT_EQ(dq["mortality_F"],
            fims::Vector<double>(n_years * n_ages, 0)
        );
        EXPECT_EQ(dq["mortality_Z"],
            fims::Vector<double>(n_years * n_ages, 0)
        );
        EXPECT_EQ(dq["numbers_at_age"],
            fims::Vector<double>((n_years + 1) * n_ages, 0)
        );
        EXPECT_EQ(dq["unfished_numbers_at_age"],
            fims::Vector<double>((n_years + 1) * n_ages, 0)
        );
        EXPECT_EQ(dq["biomass"],
            fims::Vector<double>(n_years + 1, 0) 
        );
        EXPECT_EQ(dq["spawning_biomass"],
            fims::Vector<double>(n_years + 1, 0) 
        );
        EXPECT_EQ(dq["unfished_biomass"],
            fims::Vector<double>(n_years + 1, 0) 
        );
        EXPECT_EQ(dq["unfished_spawning_biomass"],
            fims::Vector<double>(n_years + 1, 0) 
        );
        EXPECT_EQ(dq["proportion_mature_at_age"],
            fims::Vector<double>((n_years + 1) * n_ages, 0) 
        );
        EXPECT_EQ(dq["expected_recruitment"],
            fims::Vector<double>(n_years + 1, 0) 
        );
        EXPECT_EQ(dq["sum_selectivity"],
            fims::Vector<double>(n_years * n_ages, 0) 
        );
        
      

        // Test population.M
        fims::Vector<double> M(n_years * n_ages, 0);
        for (int i = 0; i < n_years * n_ages; i++)
        {
            M[i] = fims_math::exp(catch_at_age_model->populations[0]->log_M[i]);
            EXPECT_EQ(catch_at_age_model->populations[0]->M[i], M[i]);
        }
        EXPECT_EQ(catch_at_age_model->populations[0]->M.size(), n_years * n_ages);

        // Test population.proportion_female
        fims::Vector<double> p_female(n_ages, 0.5);
        for(int i = 0; i < n_ages; i++)
        {
            EXPECT_EQ(catch_at_age_model->populations[0]->proportion_female[i], p_female[i]);
        }
        
    }
} // namespace

