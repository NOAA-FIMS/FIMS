#include "gtest/gtest.h"
#include "../../tests/gtest/test_surplus_production_test_fixture.hpp"

namespace
{

    TEST_F(SPEvaluateTestFixture, CalculateLandings_works)
    {
        std::vector<double> observed_catch(nyears, 0);
        // calculate landings in in suplus production module
        surplus_production_model->CalculateCatch(population, year);
        auto& dq_pop = surplus_production_model->population_derived_quantities[population->GetId()];

        for (int fleet_ = 0; fleet_ < population->nfleets; fleet_++) {
            observed_catch[year] += population->fleets[fleet_]->observed_landings_data->at(year);
        }

        EXPECT_EQ( observed_catch[year], dq_pop["observed_catch"][year]);
        

    }

}