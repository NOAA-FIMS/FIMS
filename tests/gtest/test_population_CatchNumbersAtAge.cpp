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
        int i_age_year = year * population.nages + age;
        int i_agem1_yearm1 = (year - 1) * population.nages + age - 1;

        // calculate catch numbers at age in population module
        // Ian: not sure which of these are needed
        population.CalculateMortality(i_age_year, year, age);
        population.CalculateNumbersAA(i_age_year, i_agem1_yearm1, age);

        population.CalculateCatchNumbersAA(i_age_year, year, age);

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
       }
        test_naa[i_age_year] = test_naa[i_agem1_yearm1] * exp(-population.mortality_Z[i_agem1_yearm1]);

        // loop over fleets to get catch numbers at age for each fleet
        for (size_t fleet_index = 0; fleet_index < population.nfleets; fleet_index++)
        {
          
            if(!population.fleets[fleet_index]->is_survey){
            // indices for use in catch equation copied from
            //   \inst\include\population_dynamics\population\population.hpp
            int i_age_yearf = year * population.nages * population.nfleets + 
              age * population.nfleets + fleet_index;

            // Baranov Catch Equation adapted from 
            // \inst\include\population_dynamics\population\population.hpp
           catch_temp =
              (population.fleets[fleet_index]->Fmort[year] *
              population.fleets[fleet_index]->selectivity->evaluate(population.ages[age])) / 
              population.mortality_Z[i_age_year] *
              test_naa[i_age_year] *
              (1 - exp(-(population.mortality_Z[i_age_year])));
            test_catch_naa[i_age_yearf] += catch_temp;
            test_catch_waa[i_age_yearf] += catch_temp * population.growth->evaluate(population.ages[age]);

            // test value
          EXPECT_EQ(population.fleets[fleet_index]->catch_numbers_at_age[i_age_year], test_catch_naa[i_age_yearf]);
          EXPECT_EQ(population.fleets[fleet_index]->catch_weight_at_age[i_age_year], test_catch_waa[i_age_yearf]);
          EXPECT_GT(population.fleets[fleet_index]->catch_numbers_at_age[i_age_year], 0);
          EXPECT_GT(population.fleets[fleet_index]->catch_weight_at_age[i_age_year], 0);
            }
        }
        
        
    }
}