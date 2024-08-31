#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{
    TEST_F(PopulationEvaluateTestFixture, CalculateB_and_SB_works)
    {
        population.CalculateMaturityAA(i_age_year, age);
        population.CalculateSpawningBiomass(i_age_year, year, age);
        population.CalculateBiomass(i_age_year, year, age);

        std::vector<double> test_SB(nyears + 1, 0);
        std::vector<double> test_B(nyears + 1, 0);

        test_SB[year] += population.numbers_at_age[i_age_year] * 0.5 * population.proportion_mature_at_age[i_age_year] *
                         population.growth->evaluate(population.ages[age]);
        test_B[year] += population.numbers_at_age[i_age_year] *
                         population.growth->evaluate(population.ages[age]);

        EXPECT_EQ(population.spawning_biomass[year], test_SB[year]);
        EXPECT_GT(population.spawning_biomass[year], 0);

        EXPECT_EQ(population.biomass[year], test_B[year]);
        EXPECT_GT(population.biomass[year], 0);
    }

    TEST_F(PopulationEvaluateTestFixture, CalculateSpawningBiomass_ExtraYear_works)
    {

        int year = population.nyears;
        int age = 6;
        int i_age_year = year * population.nages + age;
        int i_agem1_yearm1 = (year - 1) * population.nages + age - 1;

        population.CalculateMortality(i_agem1_yearm1, year-1, age-1);
        population.CalculateMaturityAA(i_age_year, age);
        population.CalculateNumbersAA(i_age_year, i_agem1_yearm1, age);
        population.CalculateSpawningBiomass(i_age_year, year, age);

        std::vector<double> test_SSB(nyears + 1, 0);

        test_SSB[nyears] += population.numbers_at_age[i_age_year] * 0.5 * 
            population.proportion_mature_at_age[i_age_year] * 
            population.growth->evaluate(population.ages[age]);

        EXPECT_EQ(population.spawning_biomass[year], test_SSB[year]);
        EXPECT_GT(population.spawning_biomass[year], 0);
    }
}