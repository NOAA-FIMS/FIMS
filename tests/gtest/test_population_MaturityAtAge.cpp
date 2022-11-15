#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{
    TEST_F(PopulationPrepareTestFixture, CalculateMaturityAA_works)
    {
        for (size_t year = 0; year < nyears; year++) {
           for (size_t age = 0; age < nages; age++){
               int index_ya = year * population.nages + age;
               population.CalculateMaturityAA(index_ya, age);
           }
        }

        fims::LogisticMaturity<double> maturity;
        maturity.median = 6;
        maturity.slope = 0.15;
        double age_test = 10;
        // 1.0/(1.0+exp(-(10-6)*0.15)) = 0.0.6456563

        double expect_maturity = 0.6456563;


    
        EXPECT_NEAR(population.proportion_mature_at_age[age_test], expect_maturity, 0.0001);
   }
}