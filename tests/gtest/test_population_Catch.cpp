#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{

    TEST_F(PopulationPrepareTestFixture, CalculateCatch_works)
    {
        
        std::vector<double> expected_catch(nyears * nfleets, 0);
        // set up an arbitrary year/age combo to test
        int year = 4;
        int age = 6;
        int index_ya = year * population.nages + age;
        int index_ya2 = (year - 1) * population.nages + age - 1;

        // calculate catch numbers at age in population module
        // Ian: not sure which of these are needed
        population.CalculateMortality(index_ya, year, age);
        population.CalculateNumbersAA(index_ya, index_ya2, age);

        population.CalculateCatchNumbersAA(index_ya, year, age);
        population.CalculateCatchWeightAA(year, age);
        population.CalculateCatch(year, age);

        for (int fleet_ = 0; fleet_ < population.nfleets; fleet_++)
        {
            int index_yaf = year * population.nages * population.nfleets +
                            age * population.nfleets + fleet_;
            int index_yf = year * population.nfleets + fleet_;
            
            expected_catch[index_yf] += population.catch_weight_at_age[index_yaf];
            
            EXPECT_EQ(population.expected_catch[index_yf], expected_catch[index_yf]);
            EXPECT_GT(population.expected_catch[index_yf], 0);
            EXPECT_GT(expected_catch[index_yf], 0);
        }
    }
}