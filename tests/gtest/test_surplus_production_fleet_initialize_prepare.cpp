#include "gtest/gtest.h"
#include "../../tests/gtest/test_surplus_production_test_fixture.hpp"
#include <random>

namespace
{
   TEST_F(SPInitializeTestFixture, Fleet_Initialize_Works)
    {
        surplus_production_model->Initialize();
        for (auto fit = surplus_production_model->fleets.begin(); 
            fit != surplus_production_model->fleets.end(); ++fit) {
            auto &fleet = (*fit).second;
            auto& dq = surplus_production_model->GetFleetDerivedQuantities(fleet->GetId());
            //TODO: derived quantities are set up in interface so the dq tests below  
            // are just testing the test fixture. Ideally, we should set up an
            // R test to ensure all the derived quantities in the interface 
            //  are correctly initialized and then we can remove the dq tests here.
            EXPECT_EQ(dq["index_expected"].size(), nyears);
            EXPECT_EQ(dq["log_index_expected"].size(), nyears);
            EXPECT_EQ(dq["log_index_depletionK_ratio"].size(), nyears);
            EXPECT_EQ(dq["mean_log_q"].size(), 1);
            EXPECT_EQ(fleet->q.size(), 1);
            }
    }

      TEST_F(SPPrepareTestFixture, Fleet_Prepare_Works)
    {
        
        surplus_production_model->Prepare();
        for (auto fit = surplus_production_model->fleets.begin(); 
            fit != surplus_production_model->fleets.end(); ++fit) {
            auto &fleet = (*fit).second;
            auto& dq = surplus_production_model->GetFleetDerivedQuantities(fleet->GetId());

            EXPECT_EQ(dq["index_expected"], 
                fims::Vector(nyears, 0.0));
            EXPECT_EQ(dq["log_index_expected"], 
                fims::Vector(nyears, 0.0));
            EXPECT_EQ(dq["log_index_depletionK_ratio"], 
                fims::Vector(nyears, 0.0));
            EXPECT_EQ(dq["mean_log_q"], 
                fims::Vector(1, 0.0));
            double q = fims_math::exp(fleet->log_q[0]);
            EXPECT_EQ(fleet->q[0], q);
            EXPECT_EQ(fleet->q.size(), 1);
        }
    }

} //namespace