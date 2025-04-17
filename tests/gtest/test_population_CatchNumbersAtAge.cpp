#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{
    TEST_F(PopulationEvaluateTestFixture, CalculateLandingsNumbersAA_CalculateLandingsWeightAA_works)
    {
        // calculate landings numbers at age in population module
        population.CalculateLandingsNumbersAA(i_age_year, year, age);

        population.CalculateLandingsWeightAA(year, age);

        std::vector<double> mortality_F(nyears * nages, 0);
        // dimension of test_landings_naa matches population module, not
        // fleet module
        std::vector<double> test_landings_naa(nyears * nages * nfleets, 0);
        std::vector<double> test_landings_waa(nyears * nages * nfleets, 0);
        std::vector<double> test_naa((nyears + 1) * nages, 0);
        
        double landings_temp;

        for (int i = 0; i < (nyears + 1) * nages; i++)
        {
          test_naa[i] = population.numbers_at_age[i];
       }
        test_naa[i_age_year] = test_naa[i_agem1_yearm1] * exp(-population.mortality_Z[i_agem1_yearm1]);

        // loop over fleets to get landings numbers at age for each fleet
        for (size_t fleet_index = 0; fleet_index < population.nfleets; fleet_index++)
        {
            // indices for use in landings equation copied from
            //   \inst\include\population_dynamics\population\population.hpp
            int i_age_yearf = year * population.nages * population.nfleets + 
              age * population.nfleets + fleet_index;

            // Baranov Landings Equation adapted from 
            // \inst\include\population_dynamics\population\population.hpp
           landings_temp =
              (population.fleets[fleet_index]->Fmort[year] *
              population.fleets[fleet_index]->selectivity->evaluate(population.ages[age])) / 
              population.mortality_Z[i_age_year] *
              test_naa[i_age_year] *
              (1 - exp(-(population.mortality_Z[i_age_year])));
            test_landings_naa[i_age_yearf] += landings_temp;
            test_landings_waa[i_age_yearf] += landings_temp * population.growth->evaluate(population.ages[age]);

            // test value
          EXPECT_EQ(population.fleets[fleet_index]->landings_numbers_at_age[i_age_year], test_landings_naa[i_age_yearf]);
          EXPECT_EQ(population.fleets[fleet_index]->landings_weight_at_age[i_age_year], test_landings_waa[i_age_yearf]);
            
        }
    }
}