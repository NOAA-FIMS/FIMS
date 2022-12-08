#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{
    TEST_F(PopulationPrepareTestFixture, CalculateNumbersAA_works)
    {

        int year = 4;
        int age = 6;
        int index_ya = year * population.nages + age;
        int index_ya2 = (year - 1) * population.nages + age - 1;

        population.CalculateMortality(index_ya, year, age);
        population.CalculateNumbersAA(index_ya, index_ya2, age);

        std::vector<double> mortality_F(nyears * nages, 0);
        std::vector<double> test_naa((nyears + 1) * nages, 0);

        for (int i = 0; i < (nyears + 1) * nages; i++)
        {
            test_naa[i] = population.numbers_at_age[i];
        }
        
        test_naa[index_ya] = test_naa[index_ya2] * exp(-population.mortality_Z[index_ya2]);
        
        EXPECT_EQ(population.numbers_at_age[index_ya], test_naa[index_ya]);     
        EXPECT_GT(population.numbers_at_age[index_ya], 0);                           
    }

    TEST_F(PopulationPrepareTestFixture, CalculateNumbersAA_PlusGroup_works)
    {

        int year = 4;
        int age = population.nages - 1;
        int index_ya = year * population.nages + age;
        int index_ya2 = (year - 1) * population.nages + age - 1;

        population.CalculateMortality(index_ya, year, age);
        population.CalculateNumbersAA(index_ya, index_ya2, age);

        std::vector<double> mortality_F(nyears * nages, 0);
        std::vector<double> test_naa((nyears + 1) * nages, 0);

        for (int i = 0; i < (nyears + 1) * nages; i++)
        {
          test_naa[i] = population.numbers_at_age[i];
        }
        
        test_naa[index_ya] = test_naa[index_ya2] * exp(-population.mortality_Z[index_ya2]);

        // plus group calculation
        test_naa[index_ya] = 
          test_naa[index_ya] + 
          test_naa[index_ya2 + 1] *
          exp(-population.mortality_Z[index_ya2 + 1]);

        EXPECT_EQ(population.numbers_at_age[index_ya], test_naa[index_ya]);     
        EXPECT_GT(population.numbers_at_age[index_ya], 0);                           
    }
}