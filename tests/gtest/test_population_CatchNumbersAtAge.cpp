#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{
    TEST_F(PopulationPrepareTestFixture, CalculateCatchNumbersAA_CalculateCatchWeightAA_works)
    {
        // set up an arbitrary year/age combo to test
        int year = 4;
        int age = 6;
        int index_ya = year * population.nages + age;
        int index_ya2 = (year - 1) * population.nages + age - 1;

        std::ofstream out("CatchNumbers.txt");
        // calculate catch numbers at age in population module
        // Ian: not sure which of these are needed
        population.CalculateMortality(index_ya, year, age);
        population.CalculateNumbersAA(index_ya, index_ya2, age);

        population.CalculateCatchNumbersAA(index_ya, year, age);
        
        population.CalculateCatchWeightAA(year, age);

        std::vector<double> mortality_F(nyears * nages, 0);
        // dimension of test_catch_naa matches population module, not
        // fleet module
        std::vector<double> test_catch_naa(nyears * nages * nfleets, 0);
        std::vector<double> test_catch_waa(nyears * nages * nfleets, 0);
        std::vector<double> test_naa((nyears + 1) * nages, 0);
        
        double catch_temp;

        for (int i = 0; i < (nyears + 1) * nages; i++)
        {
          test_naa[i] = population.numbers_at_age[i];
          out << population.numbers_at_age[i] << std::endl;
        }
        test_naa[index_ya] = test_naa[index_ya2] * exp(-population.mortality_Z[index_ya2]);

        // loop over fleets to get catch numbers at age for each fleet
        for (size_t fleet_index = 0; fleet_index < population.nfleets; fleet_index++)
        {
            // indices for use in catch equation copied from
            //   \inst\include\population_dynamics\population\population.hpp
            int index_yaf = year * population.nages * population.nfleets + 
              age * population.nfleets + fleet_index;

            // Baranov Catch Equation adapted from 
            // \inst\include\population_dynamics\population\population.hpp
            out << "F:" << population.fleets[fleet_index]->Fmort[year]  << std::endl;
            catch_temp =
              (population.fleets[fleet_index]->Fmort[year] *
              population.fleets[fleet_index]->selectivity->evaluate(population.ages[age])) / 
              population.mortality_Z[index_ya] *
              test_naa[index_ya] *
              (1 - exp(-(population.mortality_Z[index_ya])));
            test_catch_naa[index_yaf] += catch_temp;
            test_catch_waa[index_yaf] += catch_temp * population.growth->evaluate(population.ages[age]);

            // test value
          EXPECT_EQ(population.fleets[fleet_index]->catch_numbers_at_age[index_ya], test_catch_naa[index_yaf]);
          EXPECT_EQ(population.fleets[fleet_index]->catch_weight_at_age[index_ya], test_catch_waa[index_yaf]);
          EXPECT_GT(population.fleets[fleet_index]->catch_numbers_at_age[index_ya], 0);
          EXPECT_GT(population.fleets[fleet_index]->catch_weight_at_age[index_ya], 0);
        }
        
        
    }
}