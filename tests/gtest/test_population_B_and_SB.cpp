#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"
#include "../../inst/include/models/functors/catch_at_age.hpp"             
#include "../../tests/gtest/test_population_test_fixture.hpp" 

namespace
{
    TEST_F(CAAEvaluateTestFixture, CalculateB_and_SB_works)
    {
        uint32_t pop_id = population->GetId();
        catch_at_age_model->CalculateMaturityAA(population, i_age_year, age);
        catch_at_age_model->CalculateSpawningBiomass(population, i_age_year, year, age);
        catch_at_age_model->CalculateBiomass(population, i_age_year, year, age);

        std::vector<double> test_SB(n_years + 1, 0);
        std::vector<double> test_B(n_years + 1, 0);

        auto& dq = catch_at_age_model->GetPopulationDerivedQuantities(pop_id);
        test_SB[year] += dq["numbers_at_age"][i_age_year] *
            catch_at_age_model->populations[0]->proportion_female[age] *
            dq["proportion_mature_at_age"][i_age_year] *
            catch_at_age_model->populations[0]->growth->evaluate(year, population->ages[age]);
        test_B[year] += dq["numbers_at_age"][i_age_year] *
                         catch_at_age_model->populations[0]->growth->evaluate(year, population->ages[age]);

        EXPECT_EQ(dq["spawning_biomass"][year], test_SB[year]);
        EXPECT_GT(dq["spawning_biomass"][year], 0);

        EXPECT_EQ(dq["biomass"][year], test_B[year]);
        EXPECT_GT(dq["biomass"][year], 0);
    }

    TEST_F(CAAEvaluateTestFixture, CalculateSpawningBiomass_ExtraYear_works)
    {
        uint32_t pop_id = population->GetId();
        int year = population->n_years;
        int age = 6;
        int i_age_year = year * population->n_ages + age;
        int i_agem1_yearm1 = (year - 1) * population->n_ages + age - 1;

        catch_at_age_model->CalculateMortality(population, i_agem1_yearm1, year-1, age-1);
        catch_at_age_model->CalculateMaturityAA(population, i_age_year, age);
        catch_at_age_model->CalculateNumbersAA(population, i_age_year, i_agem1_yearm1, age);
        catch_at_age_model->CalculateSpawningBiomass(population, i_age_year, year, age);

        std::vector<double> test_SSB(n_years + 1, 0);

        auto& dq = catch_at_age_model->GetPopulationDerivedQuantities(pop_id);
        test_SSB[n_years] += dq["numbers_at_age"][i_age_year] *
            catch_at_age_model->populations[0]->proportion_female[age] *
            dq["proportion_mature_at_age"][i_age_year] *
            catch_at_age_model->populations[0]->growth->evaluate(year, population->ages[age]);

        EXPECT_EQ(dq["spawning_biomass"][year], test_SSB[year]);
        EXPECT_GT(dq["spawning_biomass"][year], 0);
    }
}