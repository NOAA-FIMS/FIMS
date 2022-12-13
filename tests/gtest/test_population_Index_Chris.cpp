#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{

    TEST_F(PopulationPrepareTestFixture, CalculateIndex_works)
    {
        
        std::vector<double> expected_index(nyears * nfleets, 0);
        // set up an arbitrary year/age combo to test
        int year = 4;
        int age = 6;
        int index_ya = year * population.nages + age;
        int index_ya2 = (year - 1) * population.nages + age - 1;
       
        // calculate index numbers at age in population module
        population.CalculateMortality(index_ya, year, age);
        population.CalculateNumbersAA(index_ya, index_ya2, age);
        population.CalculateIndex(index_ya, year, age);

        for (int fleet_ = 0; fleet_ < population.nfleets; fleet_++)
        {
            int index_yaf = year * population.nages * population.nfleets +
                            age * population.nfleets + fleet_;
            int index_yf = year * population.nfleets + fleet_;
            
            expected_index[index_yf] += population.number_at_age[index_yaf]*
                                        population.fleets[fleet_]->q[index_yf]*
                                        population.fleets[fleet_]->selectivity->evaluate(age)*
                                        population.weight_at_age[age];
        }
        EXPECT_EQ(population.expected_index[index_yf], expected_index[index_yf]);
        EXPECT_GT(population.expected_index[index_yf], 0);
        EXPECT_GT(expected_index[index_yf], 0);
        
    }
}