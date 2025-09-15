#include "gtest/gtest.h"
#include "../../tests/gtest/test_surplus_production_test_fixture.hpp"

namespace
{
     TEST_F(SPInitializeTestFixture, input_data_are_specified)
    { 
        surplus_production_model->Initialize();
        EXPECT_EQ(surplus_production_model->populations[0]->id_g, id_g);
        EXPECT_EQ(surplus_production_model->populations[0]->nyears, nyears);
        EXPECT_EQ(surplus_production_model->populations[0]->fleets.size(), nfleets);
        EXPECT_EQ(surplus_production_model->populations[0]->nfleets, nfleets);
    }

     TEST_F(SPInitializeTestFixture, Initialize_works)
    {
        surplus_production_model->Initialize();
        auto& dq = surplus_production_model->population_derived_quantities[0];
        EXPECT_EQ(dq["biomass"].size(), nyears+1);
        EXPECT_EQ(dq["observed_catch"].size(), nyears);
        EXPECT_EQ(dq["harvest_rate"].size(), nyears);
        EXPECT_EQ(dq["fmsy"].size(), 1);
        EXPECT_EQ(dq["bmsy"].size(), 1);
        EXPECT_EQ(dq["msy"].size(), 1);
    }

    TEST_F(SPPrepareTestFixture, Prepare_works)
    {
        surplus_production_model->Prepare();  
        auto &dq = surplus_production_model->population_derived_quantities[1];
        
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