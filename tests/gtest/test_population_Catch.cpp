#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{

    TEST_F(PopulationPrepareTestFixture, CalculateCatch_works)
    {

        std::vector<double> expected_catch(nyears * nfleets, 0);
        // calculate catch numbers at age in population module
         population.CalculateCatchNumbersAA(i_age_year, year, age);
        
        population.CalculateCatchWeightAA(year, age);
        population.CalculateCatch(year, age);

        for (int fleet_ = 0; fleet_ < population.nfleets; fleet_++)
        {
            if(!population.fleets[fleet_]->is_survey){
            int index_yf = year * population.nfleets + fleet_;
            
            expected_catch[index_yf] += population.fleets[fleet_]->catch_weight_at_age[i_age_year];
            
            EXPECT_EQ(population.expected_catch[index_yf], expected_catch[index_yf]);
            EXPECT_GT(population.expected_catch[index_yf], 0);
            EXPECT_GT(expected_catch[index_yf], 0);
            EXPECT_EQ(expected_catch[index_yf], population.fleets[fleet_]->expected_catch[year]);
            }
        }
    }
}