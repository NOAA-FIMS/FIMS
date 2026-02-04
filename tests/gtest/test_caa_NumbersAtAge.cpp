#include "gtest/gtest.h"
#include "../../tests/gtest/test_caa_test_fixture.hpp"

namespace
{
    TEST_F(CAAEvaluateTestFixture, CalculateNumbersAA_works)
    {

        std::vector<double> mortality_F(n_years * n_ages, 0);
        std::vector<double> test_naa((n_years + 1) * n_ages, 0);

        size_t pop_id = population->GetId();
        auto& dq = catch_at_age_model->GetPopulationDerivedQuantities(pop_id);

        for (int i = 0; i < (n_years + 1) * n_ages; i++)
        {
            test_naa[i] = dq["numbers_at_age"][i];
        }

        test_naa[i_age_year] = test_naa[i_agem1_yearm1] * exp(-dq["mortality_Z"][i_agem1_yearm1]);

        EXPECT_EQ(dq["numbers_at_age"][i_age_year], test_naa[i_age_year]);
        EXPECT_GT(dq["numbers_at_age"][i_age_year], 0);
    }

    TEST_F(CAAEvaluateTestFixture, CalculateNumbersAA_PlusGroup_works)
    {

        int year = 4;
        int age = population->n_ages - 1;
        int i_age_year = year * population->n_ages + age;
        int i_agem1_yearm1 = (year - 1) * population->n_ages + age - 1;

        size_t pop_id = population->GetId();
        auto& dq = catch_at_age_model->GetPopulationDerivedQuantities(pop_id);

        catch_at_age_model->CalculateMortality(population, i_age_year, year, age);
        catch_at_age_model->CalculateNumbersAA(population, i_age_year, i_agem1_yearm1, age);

        std::vector<double> mortality_F(n_years * n_ages, 0);
        std::vector<double> test_naa((n_years + 1) * n_ages, 0);

        for (int i = 0; i < (n_years + 1) * n_ages; i++)
        {
          test_naa[i] = dq["numbers_at_age"][i];
        }

        test_naa[i_age_year] = test_naa[i_agem1_yearm1] * exp(-dq["mortality_Z"][i_agem1_yearm1]);

        // plus group calculation
        test_naa[i_age_year] = 
          test_naa[i_age_year] + 
          test_naa[i_agem1_yearm1 + 1] *
          exp(-dq["mortality_Z"][i_agem1_yearm1 + 1]);

        EXPECT_EQ(dq["numbers_at_age"][i_age_year], test_naa[i_age_year]);     
        EXPECT_GT(dq["numbers_at_age"][i_age_year], 0);                           
    }
}