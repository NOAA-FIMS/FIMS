#include "gtest/gtest.h"
#include "../../tests/gtest/test_caa_test_fixture.hpp"

namespace
{
    TEST_F(CAAEvaluateTestFixture, CalculateMaturityAA_works)
    {        
        double inflection_point = 6;
        double slope = 0.15;
        std::vector<double> expect_maturity(n_years * n_ages, 0);

        for (size_t year = 0; year < n_years; year++) {
           for (size_t age = 0; age < n_ages; age++){
               int i_age_year = year * population->n_ages + age;
               catch_at_age_model->CalculateMaturityAA(population, i_age_year, age);
               expect_maturity[i_age_year] = 1.0/(1.0+exp(-(population->ages[age]-inflection_point)*slope));
           }
        }
        size_t pop_id = population->GetId();
        auto& dq = catch_at_age_model->GetPopulationDerivedQuantities(pop_id);
        EXPECT_NEAR(dq["proportion_mature_at_age"][10], expect_maturity[10], 0.0001);
   }
}