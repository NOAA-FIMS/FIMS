#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{
    TEST_F(PopulationEvaluateTestFixture, CalculateNumbersAA_works)
    {

        std::vector<double> mortality_F(nyears * nages, 0);
        std::vector<double> test_naa((nyears + 1) * nages, 0);

        for (int i = 0; i < (nyears + 1) * nages; i++)
        {
            test_naa[i] = population.numbers_at_age[i];
        }
        
        test_naa[i_age_year] = test_naa[i_agem1_yearm1] * exp(-population.mortality_Z[i_agem1_yearm1]);
        
        EXPECT_EQ(population.numbers_at_age[i_age_year], test_naa[i_age_year]);     
        EXPECT_GT(population.numbers_at_age[i_age_year], 0);                           
    }

    TEST_F(PopulationEvaluateTestFixture, CalculateNumbersAA_PlusGroup_works)
    {

        int year = 4;
        int age = population.nages - 1;
        int i_age_year = year * population.nages + age;
        int i_agem1_yearm1 = (year - 1) * population.nages + age - 1;

        population.CalculateMortality(i_age_year, year, age);
        population.CalculateNumbersAA(i_age_year, i_agem1_yearm1, age);

        std::vector<double> mortality_F(nyears * nages, 0);
        std::vector<double> test_naa((nyears + 1) * nages, 0);

        for (int i = 0; i < (nyears + 1) * nages; i++)
        {
          test_naa[i] = population.numbers_at_age[i];
        }
        
        test_naa[i_age_year] = test_naa[i_agem1_yearm1] * exp(-population.mortality_Z[i_agem1_yearm1]);

        // plus group calculation
        test_naa[i_age_year] = 
          test_naa[i_age_year] + 
          test_naa[i_agem1_yearm1 + 1] *
          exp(-population.mortality_Z[i_agem1_yearm1 + 1]);

        EXPECT_EQ(population.numbers_at_age[i_age_year], test_naa[i_age_year]);     
        EXPECT_GT(population.numbers_at_age[i_age_year], 0);                           
    }
}