#include "gtest/gtest.h"
#include "../../tests/gtest/test_surplus_production_test_fixture.hpp"

namespace
{
     TEST_F(SPInitializeTestFixture, input_data_are_specified)
    { 
        surplus_production_model->Initialize();
        EXPECT_EQ(surplus_production_model->populations[0]->id_g, id_g);
        EXPECT_EQ(surplus_production_model->populations[0]->n_years, nyears);
        EXPECT_EQ(surplus_production_model->populations[0]->fleets.size(), nfleets);
        EXPECT_EQ(surplus_production_model->populations[0]->n_fleets, nfleets);
    }

     TEST_F(SPInitializeTestFixture, Population_Initialize_Works)
    {
        //TODO: derived quantities are set up in interface so the dq tests below  
        // are just testing the test fixture. Ideally, we should set up an
        // R test to ensure all the derived quantities in the interface 
        //  are correctly initialized and then we can remove the dq tests here.
        surplus_production_model->Initialize();
        auto& dq = surplus_production_model->GetPopulationDerivedQuantities(population->GetId());
        EXPECT_EQ(dq["biomass"].size(), nyears+1);
        EXPECT_EQ(dq["observed_catch"].size(), nyears);
        EXPECT_EQ(dq["harvest_rate"].size(), nyears);
        EXPECT_EQ(dq["fmsy"].size(), 1);
        EXPECT_EQ(dq["bmsy"].size(), 1);
        EXPECT_EQ(dq["msy"].size(), 1);
    }

    TEST_F(SPPrepareTestFixture, Population_Prepare_Works)
    {
        surplus_production_model->Prepare();  
        auto &dq = surplus_production_model->GetPopulationDerivedQuantities(population->GetId());
        
        // vector size type = nyears and vector value = 0
        EXPECT_EQ(dq["biomass"],
            fims::Vector<double>(nyears+1, 0)
        );
        EXPECT_EQ(dq["observed_catch"],
            fims::Vector<double>(nyears, 0)
        );
        EXPECT_EQ(dq["harvest_rate"],
            fims::Vector<double>(nyears, 0)
        );
        EXPECT_EQ(dq["fmsy"],
            fims::Vector<double>(1, 0)
        );
        EXPECT_EQ(dq["bmsy"],
            fims::Vector<double>(1, 0)
        );
        EXPECT_EQ(dq["msy"],
            fims::Vector<double>(1, 0)
        );
    }
} //namespace