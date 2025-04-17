#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{

    TEST_F(PopulationEvaluateTestFixture, CalculateLandings_works)
    {

        std::vector<double> landings_expected(nyears * nfleets, 0);
        // calculate landings numbers at age in population module
         population.CalculateLandingsNumbersAA(i_age_year, year, age);
        
        population.CalculateLandingsWeightAA(year, age);
        population.CalculateLandings(year, age);

        for (int fleet_ = 0; fleet_ < population.nfleets; fleet_++)
        {
            int index_yf = year * population.nfleets + fleet_;
            
            landings_expected[index_yf] += population.fleets[fleet_]->landings_weight_at_age[i_age_year];
            
            EXPECT_EQ(landings_expected[index_yf], population.fleets[fleet_]->landings_weight[year]);
        }
    }
}