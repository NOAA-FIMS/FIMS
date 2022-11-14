#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{
    TEST_F(PopulationPrepareTestFixture, CalculateCatchNumbersAA_works)
    {
        // set up an arbitrary year/age combo to test
        int year = 4;
        int age = 6;
        int index_ya = year * population.nages + age;
        int index_ya2 = (year - 1) * population.nages + age - 1;

        // calculate catch numbers at age in population module
        // Ian: not sure which of these are needed
        population.CalculateMortality(index_ya, year, age);
        population.CalculateNumbersAA(index_ya, index_ya2);
        population.CalculateCatchNumbersAA(index_ya, index_ya2);

        std::vector<double> mortality_F(nyears * nages, 0);
        std::vector<double> test_catch_naa(nyears * nages, 0);
        
        // loop over fleets to get catch numbers at age for each fleet
        for (size_t fleet_index = 0; fleet_index < population.nfleets; fleet_index++)
        {
            // indices for use in catch equation copied from
            //   \inst\include\population_dynamics\population\population.hpp
            int index_yaf = year * population.nages * population.nfleets + 
              age * population.nfleets + fleet_index;
            int index_yf = year * population.nfleets + fleet_index;
            // Baranov Catch Equation adapted from 
            //   \inst\include\population_dynamics\population\population.hpp
            test_catch_naa[index_ya] += 
              (population.Fmort[index_yf] *
              // Ian: not sure about getting selectivity in the same way as population.hpp
              fleets[fleet_index]->selectivity->evaluate(age)) / 
              population.mortality_Z[index_ya] *
              population.numbers_at_age[index_ya] *
              (1 - exp(-(population.mortality_Z[index_ya])));
        }
        // test value
        EXPECT_EQ(population.catch_numbers_at_age[index_ya], test_catch_naa[index_ya]);
    }
}