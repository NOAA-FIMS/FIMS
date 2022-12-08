#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{
    TEST_F(PopulationPrepareTestFixture, CalculateSpawningBiomass_works)
    {

        int year = 4;
        int age = 6;
        int index_ya = year * population.nages + age;
        int index_ya2 = (year - 1) * population.nages + age - 1;

        population.CalculateMortality(index_ya, year, age);
        population.CalculateNumbersAA(index_ya, index_ya2);
        population.CalculateMaturityAA(index_ya, age);
        population.CalculateSpawningBiomass(index_ya, year, age);

        // double test_SSB = 0;
        std::vector<double> test_SSB(nyears + 1, 0);

        // test_SSB = population.numbers_at_age[index_ya]
        // * population.proportion_female * population.proportion_mature_at_age[age] *
        // population.weight_at_age[age];

        test_SSB[year] += population.numbers_at_age[index_ya] * 0.5 * population.proportion_mature_at_age[index_ya] *
                         population.weight_at_age[age];

        // EXPECT_EQ(population.spawning_biomass[year], test_SSB);
        EXPECT_EQ(population.spawning_biomass[year], test_SSB[year]);
        EXPECT_GT(population.spawning_biomass[year], 0);
    }

    TEST_F(PopulationPrepareTestFixture, CalculateSpawningBiomass_ExtraYear_works)
    {

        int year = population.nyears;
        int age = 6;
        int index_ya = year * population.nages + age;
        int index_ya2 = (year - 1) * population.nages + age - 1;

        population.CalculateMortality(index_ya2, year-1, age-1);
        population.CalculateMaturityAA(index_ya, age);
        population.CalculateNumbersAA(index_ya, index_ya2);
        population.CalculateSpawningBiomass(index_ya, year, age);

        std::vector<double> test_SSB(nyears + 1, 0);

        test_SSB[nyears] += population.numbers_at_age[index_ya] * 0.5 * 
            population.proportion_mature_at_age[index_ya] * 
            population.weight_at_age[age];

        EXPECT_EQ(population.spawning_biomass[year], test_SSB[year]);
        EXPECT_GT(population.spawning_biomass[year], 0);
    }
}