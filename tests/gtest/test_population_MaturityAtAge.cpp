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
               int index_ya = year * population.nages + age;
               population.CalculateMaturityAA(index_ya, age);
               expect_maturity[index_ya] = 1.0/(1.0+exp(-(age-median)*slope));
           }
        }

        EXPECT_NEAR(population.proportion_mature_at_age[10], expect_maturity[10], 0.0001);
   }
}