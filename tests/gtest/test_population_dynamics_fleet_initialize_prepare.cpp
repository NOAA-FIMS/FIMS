#include "gtest/gtest.h"
#include "population_dynamics/fleet/fleet.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"
#include <random>

namespace
{

    TEST_F(CAAInitializeTestFixture, FleetInitializeWorks)
    {

        this->InitializeCAA();
        for (auto fit = catch_at_age_model->fleets.begin(); 
            fit != catch_at_age_model->fleets.end(); ++fit) {
            auto &fleet = (*fit).second;
            auto& dq = catch_at_age_model->GetFleetDerivedQuantities(fleet->GetId());
            EXPECT_EQ(dq["landings_numbers_at_age"].size(), nyears * nages);
            EXPECT_EQ(dq["landings_weight_at_age"].size(), nyears * nages);
            EXPECT_EQ(dq["landings_numbers_at_length"].size(), nyears * nlengths);
            EXPECT_EQ(dq["landings_weight"].size(), nyears);
            EXPECT_EQ(dq["landings_numbers"].size(), nyears);
            EXPECT_EQ(dq["landings_expected"].size(), nyears);
            EXPECT_EQ(dq["log_landings_expected"].size(), nyears);
            EXPECT_EQ(dq["agecomp_proportion"].size(), nyears * nages);
            EXPECT_EQ(dq["lengthcomp_proportion"].size(), nyears * nlengths);
            EXPECT_EQ(dq["index_numbers_at_age"].size(), nyears * nages);
            EXPECT_EQ(dq["index_numbers_at_length"].size(), nyears * nlengths);
            EXPECT_EQ(dq["index_weight"].size(), nyears);
            EXPECT_EQ(dq["index_numbers"].size(), nyears);
            EXPECT_EQ(dq["index_expected"].size(), nyears);
            EXPECT_EQ(dq["log_index_expected"].size(), nyears);
            EXPECT_EQ(dq["catch_index"].size(), nyears);
            EXPECT_EQ(dq["expected_catch"].size(), nyears);
            EXPECT_EQ(dq["agecomp_expected"].size(), nyears * nages);
            EXPECT_EQ(dq["lengthcomp_expected"].size(), nyears * nlengths);
            // EXPECT_EQ(dq["age_to_length_conversion"].size(), nages * nlengths);
            EXPECT_EQ(fleet->Fmort.size(), nyears);
            EXPECT_EQ(fleet->q.size(), 1);
        }
    }

    TEST_F(CAAPrepareTestFixture, FleetPrepareWorks)
    {
        this->InitializeCAA();
        catch_at_age_model->Prepare();

        for (auto fit = catch_at_age_model->fleets.begin(); 
            fit != catch_at_age_model->fleets.end(); ++fit) {
            auto &fleet = (*fit).second;
            auto& dq = catch_at_age_model->GetFleetDerivedQuantities(fleet->GetId());

            EXPECT_EQ(dq["landings_numbers_at_age"], 
                fims::Vector(nyears * nages, 0.0));
            EXPECT_EQ(dq["landings_weight_at_age"], 
                fims::Vector(nyears * nages, 0.0));
            EXPECT_EQ(dq["landings_numbers_at_length"], 
                fims::Vector(nyears * nlengths, 0.0));
            EXPECT_EQ(dq["landings_weight"], 
                fims::Vector(nyears, 0.0));
            EXPECT_EQ(dq["landings_numbers"], 
                fims::Vector(nyears, 0.0));
            EXPECT_EQ(dq["landings_expected"], 
                fims::Vector(nyears, 0.0));
            EXPECT_EQ(dq["log_landings_expected"], 
                fims::Vector(nyears, 0.0));
            EXPECT_EQ(dq["agecomp_proportion"], 
                fims::Vector(nyears * nages, 0.0));
            EXPECT_EQ(dq["lengthcomp_proportion"], 
                fims::Vector(nyears * nlengths, 0.0));
            EXPECT_EQ(dq["index_numbers_at_age"], 
                fims::Vector(nyears * nages, 0.0));
            EXPECT_EQ(dq["index_numbers_at_length"], 
                fims::Vector(nyears * nlengths, 0.0));
            EXPECT_EQ(dq["index_weight"], 
                fims::Vector(nyears, 0.0));
            EXPECT_EQ(dq["index_numbers"], 
                fims::Vector(nyears, 0.0));
            EXPECT_EQ(dq["index_expected"], 
                fims::Vector(nyears, 0.0));
            EXPECT_EQ(dq["log_index_expected"], 
                fims::Vector(nyears, 0.0));
            EXPECT_EQ(dq["catch_index"], 
                fims::Vector(nyears, 0.0));
            EXPECT_EQ(dq["expected_catch"], 
                fims::Vector(nyears, 0.0));
            EXPECT_EQ(dq["agecomp_expected"], 
                fims::Vector(nyears * nages, 0.0));
            EXPECT_EQ(dq["lengthcomp_expected"], 
                fims::Vector(nyears * nlengths, 0.0));

            fims::Vector<double> Fmort(nyears, 0);
            for(size_t y = 0; y < nyears; y++){
                Fmort[y] = fims_math::exp(fleet->log_Fmort[y]);
                EXPECT_EQ(fleet->Fmort[y], Fmort[y]);
            }
            EXPECT_EQ(fleet->Fmort.size(), fleet->nyears);
            double q = fims_math::exp(fleet->log_q[0]);
            EXPECT_EQ(fleet->q[0], q);
            EXPECT_EQ(fleet->q.size(), 1);

        }
    }
} // namespace
