#include "gtest/gtest.h"
#include "../../tests/gtest/test_surplus_production_test_fixture.hpp"
#include <random>

namespace
{
   TEST_F(SPInitializeTestFixture, FleetInitializeWorks)
    {
        surplus_production_model->Initialize();
        for (auto fit = surplus_production_model->fleets.begin(); 
            fit != surplus_production_model->fleets.end(); ++fit) {
            auto &fleet = (*fit).second;
            auto& dq = surplus_production_model->fleet_derived_quantities[fleet->GetId()];
            EXPECT_EQ(dq["index_expected"].size(), nyears);
            EXPECT_EQ(dq["log_index_expected"].size(), nyears);
            EXPECT_EQ(fleet->q.size(), 1);
            }
    }

      TEST_F(SPPrepareTestFixture, FleetPrepareWorks)
    {
        
        surplus_production_model->Prepare();
        for (auto fit = surplus_production_model->fleets.begin(); 
            fit != surplus_production_model->fleets.end(); ++fit) {
            auto &fleet = (*fit).second;
            auto& dq = surplus_production_model->fleet_derived_quantities[fleet->GetId()];

            EXPECT_EQ(dq["index_expected"], 
                fims::Vector(nyears, 0.0));
            EXPECT_EQ(dq["log_index_expected"], 
                fims::Vector(nyears, 0.0));
             double q = fims_math::exp(fleet->log_q[0]);
            EXPECT_EQ(fleet->q[0], q);
            EXPECT_EQ(fleet->q.size(), 1);
        }
    }

} //namespace