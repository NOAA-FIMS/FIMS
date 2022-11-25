#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{

    TEST_F(PopulationPrepareTestFixture, CalculateCatch_works)
    {

        std::vector<double> expected_catch(nyears * nfleets, 0);

        for(int year = 0; year < population.nyears; year++)
        {
            for (int age = 0; age < population.nages; age++)
            {
                int index_ya = year * population.nages + age;
                int index_ya2 = (year - 1) * population.nages + age - 1;

                population.CalculateMortality(index_ya, year, age);
                population.CalculateInitialNumbersAA(index_ya, age);
                population.CalculateNumbersAA(index_ya, index_ya2);
                population.CalculateCatchNumbersAA(index_ya, year, age);
                population.CalculateCatch(year, age);
                
                for(int fleet_ = 0; fleet_ < population.nfleets; fleet_++)
                {
                    int index_yaf = year * population.nages * population.nfleets +
                        age * population.nfleets + fleet_;
                    int index_yf = year * population.nfleets + fleet_; 

                    expected_catch[index_yf] += population.catch_weight_at_age[index_yaf];

                }
            }
        }
        for(int year = 0; year < population.nyears; year++)
        {
            for(int fleet_ = 0; fleet_ < population.nfleets; fleet_++)
                {
                    int index_yf = year * population.nfleets + fleet_; 
                    EXPECT_EQ(population.expected_catch[index_yf], expected_catch[index_yf]);
                }
        }

    }
}