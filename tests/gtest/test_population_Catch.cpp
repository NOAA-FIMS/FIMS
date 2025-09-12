#include "gtest/gtest.h"
#include "../../tests/gtest/test_caa_test_fixture.hpp"

namespace
{

    TEST_F(CAAEvaluateTestFixture, CalculateLandings_works)
    {

        std::vector<double> landings_expected(nyears * nfleets, 0);
        // calculate landings numbers at age in population module
         catch_at_age_model->CalculateLandingsNumbersAA(population, i_age_year, year, age);

        catch_at_age_model->CalculateLandingsWeightAA(population, year, age);
        catch_at_age_model->CalculateLandings(population, year, age);

        for (int fleet_ = 0; fleet_ < population->nfleets; fleet_++)
        {
            int index_yf = year * population->nfleets + fleet_;
            uint32_t fleet_id = population->fleets[fleet_]->GetId();
            auto& dq_fleet = catch_at_age_model->fleet_derived_quantities[fleet_id];

            landings_expected[index_yf] += dq_fleet["landings_weight_at_age"][i_age_year];

            EXPECT_EQ(landings_expected[index_yf], dq_fleet["landings_weight"][year]);
        }
    }
}