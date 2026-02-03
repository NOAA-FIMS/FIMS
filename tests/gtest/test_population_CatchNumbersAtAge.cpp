#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{
    TEST_F(CAAEvaluateTestFixture, CalculateLandingsNumbersAA_CalculateLandingsWeightAA_works)
    {
        
        size_t pop_id = population->GetId();
        auto& dq_pop = catch_at_age_model->GetPopulationDerivedQuantities(pop_id);

        // calculate landings numbers at age in population module
        catch_at_age_model->CalculateLandingsNumbersAA(population, i_age_year, year, age);
        catch_at_age_model->CalculateLandingsWeightAA(population, year, age);

        std::vector<double> mortality_F(n_years * n_ages, 0);
        // dimension of test_landings_naa matches population module, not
        // fleet module
        std::vector<double> test_landings_naa(n_years * n_ages * n_fleets, 0);
        std::vector<double> test_landings_waa(n_years * n_ages * n_fleets, 0);
        std::vector<double> test_naa((n_years + 1) * n_ages, 0);
        
        double landings_temp;

        for (int i = 0; i < (n_years + 1) * n_ages; i++)
        {
          test_naa[i] = dq_pop["numbers_at_age"][i];
       }
        test_naa[i_age_year] = test_naa[i_agem1_yearm1] * exp(-dq_pop["mortality_Z"][i_agem1_yearm1]);

        // loop over fleets to get landings numbers at age for each fleet
        for (size_t fleet_index = 0; fleet_index < population->n_fleets; fleet_index++)
        {
            // indices for use in landings equation copied from
            //   \inst\include\population_dynamics\population\population.hpp
            int i_age_yearf = year * population->n_ages * population->n_fleets +
              age * population->n_fleets + fleet_index;

            uint32_t fleet_id = population->fleets[fleet_index]->GetId();
            auto& dq_fleet = catch_at_age_model->GetFleetDerivedQuantities(fleet_id);

            // Baranov Landings Equation adapted from 
            // \inst\include\population_dynamics\population\population.hpp
           landings_temp =
              (population->fleets[fleet_index]->Fmort[year] *
              population->fleets[fleet_index]->selectivity->evaluate(population->ages[age])) /
              dq_pop["mortality_Z"][i_age_year] *
              test_naa[i_age_year] *
              (1 - exp(-(dq_pop["mortality_Z"][i_age_year])));
            test_landings_naa[i_age_yearf] += landings_temp;
            test_landings_waa[i_age_yearf] += landings_temp * population->growth->evaluate(year, population->ages[age]);

            // test value
          EXPECT_EQ(dq_fleet["landings_numbers_at_age"][i_age_year], test_landings_naa[i_age_yearf]);
          EXPECT_EQ(dq_fleet["landings_weight_at_age"][i_age_year], test_landings_waa[i_age_yearf]);

        }
    }
}