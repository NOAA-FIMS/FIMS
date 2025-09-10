#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{
    TEST_F(CAAEvaluateTestFixture, CalculateNumbersAA_works)
    {

        std::vector<double> mortality_F(nyears * nages, 0);
        std::vector<double> test_naa((nyears + 1) * nages, 0);

        size_t pop_id = population->GetId();
        auto& dq = catch_at_age_model->GetPopulationDerivedQuantities(pop_id);

        for (int i = 0; i < (nyears + 1) * nages; i++)
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
        int age = population->nages - 1;
        int i_age_year = year * population->nages + age;
        int i_agem1_yearm1 = (year - 1) * population->nages + age - 1;

        size_t pop_id = population->GetId();
        auto& dq = catch_at_age_model->GetPopulationDerivedQuantities(pop_id);

        catch_at_age_model->CalculateMortality(population, i_age_year, year, age);
        catch_at_age_model->CalculateNumbersAA(population, i_age_year, i_agem1_yearm1, age);

        std::vector<double> mortality_F(nyears * nages, 0);
        std::vector<double> test_naa((nyears + 1) * nages, 0);

        for (int i = 0; i < (nyears + 1) * nages; i++)
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