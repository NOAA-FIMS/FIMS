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
            EXPECT_EQ(dq["landings_numbers_at_age"].size(), n_years * n_ages);
            EXPECT_EQ(dq["landings_weight_at_age"].size(), n_years * n_ages);
            EXPECT_EQ(dq["landings_numbers_at_length"].size(), n_years * n_lengths);
            EXPECT_EQ(dq["landings_weight"].size(), n_years);
            EXPECT_EQ(dq["landings_numbers"].size(), n_years);
            EXPECT_EQ(dq["landings_expected"].size(), n_years);
            EXPECT_EQ(dq["log_landings_expected"].size(), n_years);
            EXPECT_EQ(dq["agecomp_proportion"].size(), n_years * n_ages);
            EXPECT_EQ(dq["lengthcomp_proportion"].size(), n_years * n_lengths);
            EXPECT_EQ(dq["index_numbers_at_age"].size(), n_years * n_ages);
            EXPECT_EQ(dq["index_numbers_at_length"].size(), n_years * n_lengths);
            EXPECT_EQ(dq["index_weight"].size(), n_years);
            EXPECT_EQ(dq["index_numbers"].size(), n_years);
            EXPECT_EQ(dq["index_expected"].size(), n_years);
            EXPECT_EQ(dq["log_index_expected"].size(), n_years);
            EXPECT_EQ(dq["catch_index"].size(), n_years);
            EXPECT_EQ(dq["expected_catch"].size(), n_years);
            EXPECT_EQ(dq["agecomp_expected"].size(), n_years * n_ages);
            EXPECT_EQ(dq["lengthcomp_expected"].size(), n_years * n_lengths);
            EXPECT_EQ(fleet->Fmort.size(), n_years);
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
                fims::Vector(n_years * n_ages, 0.0));
            EXPECT_EQ(dq["landings_weight_at_age"], 
                fims::Vector(n_years * n_ages, 0.0));
            EXPECT_EQ(dq["landings_numbers_at_length"], 
                fims::Vector(n_years * n_lengths, 0.0));
            EXPECT_EQ(dq["landings_weight"], 
                fims::Vector(n_years, 0.0));
            EXPECT_EQ(dq["landings_numbers"], 
                fims::Vector(n_years, 0.0));
            EXPECT_EQ(dq["landings_expected"], 
                fims::Vector(n_years, 0.0));
            EXPECT_EQ(dq["log_landings_expected"], 
                fims::Vector(n_years, 0.0));
            EXPECT_EQ(dq["agecomp_proportion"], 
                fims::Vector(n_years * n_ages, 0.0));
            EXPECT_EQ(dq["lengthcomp_proportion"], 
                fims::Vector(n_years * n_lengths, 0.0));
            EXPECT_EQ(dq["index_numbers_at_age"], 
                fims::Vector(n_years * n_ages, 0.0));
            EXPECT_EQ(dq["index_numbers_at_length"], 
                fims::Vector(n_years * n_lengths, 0.0));
            EXPECT_EQ(dq["index_weight"], 
                fims::Vector(n_years, 0.0));
            EXPECT_EQ(dq["index_numbers"], 
                fims::Vector(n_years, 0.0));
            EXPECT_EQ(dq["index_expected"], 
                fims::Vector(n_years, 0.0));
            EXPECT_EQ(dq["log_index_expected"], 
                fims::Vector(n_years, 0.0));
            EXPECT_EQ(dq["catch_index"], 
                fims::Vector(n_years, 0.0));
            EXPECT_EQ(dq["expected_catch"], 
                fims::Vector(n_years, 0.0));
            EXPECT_EQ(dq["agecomp_expected"], 
                fims::Vector(n_years * n_ages, 0.0));
            EXPECT_EQ(dq["lengthcomp_expected"], 
                fims::Vector(n_years * n_lengths, 0.0));


        }
    }
} // namespace
