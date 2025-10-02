#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{
    TEST_F(CAAEvaluateTestFixture, CalculateMaturityAA_works)
    {        
        double inflection_point = 6;
        double slope = 0.15;
        std::vector<double> expect_maturity(nyears * nages, 0);

        for (size_t year = 0; year < nyears; year++) {
           for (size_t age = 0; age < nages; age++){
               int i_age_year = year * population->nages + age;
               catch_at_age_model->CalculateMaturityAA(population, i_age_year, age);
               expect_maturity[i_age_year] = 1.0/(1.0+exp(-(population->ages[age]-inflection_point)*slope));
           }
        }
        size_t pop_id = population->GetId();
        auto& dq = catch_at_age_model->GetPopulationDerivedQuantities(pop_id);
        EXPECT_NEAR(dq["proportion_mature_at_age"][10], expect_maturity[10], 0.0001);
   }
}