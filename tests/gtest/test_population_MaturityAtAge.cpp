#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{
    TEST_F(PopulationPrepareTestFixture, CalculateMaturityAA_works)
    {        
        double median = 6;
        double slope = 0.15;
        std::vector<double> expect_maturity(nyears * nages, 0);

        for (size_t year = 0; year < nyears; year++) {
           for (size_t age = 0; age < nages; age++){
               int i_age_year = year * population.nages + age;
               population.CalculateMaturityAA(i_age_year, age);
               expect_maturity[i_age_year] = 1.0/(1.0+exp(-(population.ages[age]-median)*slope));
           }
        }

        EXPECT_NEAR(population.proportion_mature_at_age[10], expect_maturity[10], 0.0001);
   }
}