#include "gtest/gtest.h"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{
   
    TEST_F(CAAInitializeTestFixture, input_data_are_specified)
    { 
        this->InitializeCAA();
        EXPECT_EQ(catch_at_age_model->populations[0]->id_g, id_g);
        EXPECT_EQ(catch_at_age_model->populations[0]->nyears, nyears);
        EXPECT_EQ(catch_at_age_model->populations[0]->nseasons, nseasons);
        EXPECT_EQ(catch_at_age_model->populations[0]->nages, nages);
        EXPECT_EQ(catch_at_age_model->populations[0]->fleets.size(), nfleets);
        EXPECT_EQ(catch_at_age_model->populations[0]->nfleets, nfleets);
    }

    TEST_F(CAAInitializeTestFixture, Initialize_works)
    {
        this->InitializeCAA();
        auto& dq = catch_at_age_model->GetPopulationDerivedQuantities(0);
        EXPECT_EQ(dq["mortality_F"].size(), nyears * nages);
        EXPECT_EQ(dq["mortality_Z"].size(), nyears * nages);
        EXPECT_EQ(dq["proportion_mature_at_age"].size(), (nyears+1) * nages);
       // EXPECT_EQ(dq["weight_at_age"].size(), nages);
        EXPECT_EQ(dq["unfished_numbers_at_age"].size(), (nyears + 1) * nages);
        EXPECT_EQ(dq["numbers_at_age"].size(), (nyears + 1) * nages);
        EXPECT_EQ(dq["total_landings_weight"].size(), nyears);
        EXPECT_EQ(dq["total_landings_numbers"].size(), nyears);
        EXPECT_EQ(dq["biomass"].size(), (nyears + 1));
        EXPECT_EQ(dq["unfished_biomass"].size(), (nyears + 1));
        EXPECT_EQ(dq["unfished_spawning_biomass"].size(), (nyears + 1));
        EXPECT_EQ(dq["spawning_biomass"].size(), nyears + 1);
        EXPECT_EQ(catch_at_age_model->populations[0]->proportion_female.size(), nages);
        EXPECT_EQ(catch_at_age_model->populations[0]->M.size(), nyears * nages);
        EXPECT_EQ(dq["expected_recruitment"].size(), nyears + 1);
        EXPECT_EQ(dq["sum_selectivity"].size(), nyears * nages);
    }

    TEST_F(CAAPrepareTestFixture, Prepare_works)
    {
        this->InitializeCAA();
        catch_at_age_model->Prepare();
        auto &dq = catch_at_age_model->GetPopulationDerivedQuantities(1);

        // vector size type = nyears and vector value = 0
        EXPECT_EQ(dq["total_landings_weight"],
            fims::Vector<double>(nyears, 0)
        );
        EXPECT_EQ(dq["total_landings_numbers"],
            fims::Vector<double>(nyears, 0)
        );
        EXPECT_EQ(dq["mortality_F"],
            fims::Vector<double>(nyears * nages, 0)
        );
        EXPECT_EQ(dq["mortality_Z"],
            fims::Vector<double>(nyears * nages, 0)
        );
        EXPECT_EQ(dq["numbers_at_age"],
            fims::Vector<double>((nyears + 1) * nages, 0)
        );
        EXPECT_EQ(dq["unfished_numbers_at_age"],
            fims::Vector<double>((nyears + 1) * nages, 0)
        );
        EXPECT_EQ(dq["biomass"],
            fims::Vector<double>(nyears + 1, 0) 
        );
        EXPECT_EQ(dq["spawning_biomass"],
            fims::Vector<double>(nyears + 1, 0) 
        );
        EXPECT_EQ(dq["unfished_biomass"],
            fims::Vector<double>(nyears + 1, 0) 
        );
        EXPECT_EQ(dq["unfished_spawning_biomass"],
            fims::Vector<double>(nyears + 1, 0) 
        );
        EXPECT_EQ(dq["proportion_mature_at_age"],
            fims::Vector<double>((nyears + 1) * nages, 0) 
        );
        EXPECT_EQ(dq["expected_recruitment"],
            fims::Vector<double>(nyears + 1, 0) 
        );
        EXPECT_EQ(dq["sum_selectivity"],
            fims::Vector<double>(nyears * nages, 0) 
        );
        
      

        // Test population.M
        fims::Vector<double> M(nyears * nages, 0);
        for (int i = 0; i < nyears * nages; i++)
        {
            M[i] = fims_math::exp(catch_at_age_model->populations[0]->log_M[i]);
            EXPECT_EQ(catch_at_age_model->populations[0]->M[i], M[i]);
        }
        EXPECT_EQ(catch_at_age_model->populations[0]->M.size(), nyears * nages);

        // Test population.proportion_female
        fims::Vector<double> p_female(nages, 0.5);
        for(int i = 0; i < nages; i++)
        {
            EXPECT_EQ(catch_at_age_model->populations[0]->proportion_female[i], p_female[i]);
        }
        
    }
} // namespace

