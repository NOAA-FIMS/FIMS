#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{

    TEST_F(CAAPrepareTestFixture, CalculateMortality_works)
    {
        this->InitializeCAA();
        catch_at_age_model->Prepare();
        size_t pop_id = population->GetId();
        auto& dq = catch_at_age_model->GetPopulationDerivedQuantities(pop_id);

        for (int year = 0; year < population->nyears; year++)
        {
            for (int age = 0; age < population->nages; age++)
            {
                int i_age_year = year * population->nages + age;
                // Call FIMS CalculateMortality() function to compare FIMS mortality values with "true" values later
                catch_at_age_model->CalculateMortality(population, i_age_year, year, age);

                
            }
        }

        std::vector<double> mortality_F(population->nages * population->nyears, 0.0); // n_ages should be defined appropriately
        std::vector<double> mortality_Z(population->nages * population->nyears, 0.0);
        for (int year = 0; year < population->nyears; year++)
        {
            for (int age = 0; age < population->nages; age++)
            {
                int i_age_year = year * population->nages + age;

                for (int fleet_index = 0; fleet_index < population->nfleets; fleet_index++)
                {
                    // Known values were used to generate "true" value and test CalculateMortality()
                    mortality_F[i_age_year] += population->fleets[fleet_index]->Fmort[year] *
                                             population->fleets[fleet_index]->selectivity->evaluate(population->ages[age]);
                
                }
                EXPECT_LT(abs(dq["mortality_F"][i_age_year] - mortality_F[i_age_year]), 1e-7);
                
               
                mortality_Z[i_age_year] = fims_math::exp(population->log_M[i_age_year]) +
                                        dq["mortality_F"][i_age_year];
                EXPECT_LT(abs(dq["mortality_Z"][i_age_year] - mortality_Z[i_age_year]), 1e-7);
            }
        }
    }

    TEST_F(CAAEvaluateTestFixture, CalculateInitialNumbersAA_works)
    {

        size_t pop_id = population->GetId();
        auto& dq = catch_at_age_model->GetPopulationDerivedQuantities(pop_id);

        std::vector<double> numbers_at_age(nyears * nages, 0);

        for (int year = 0; year < population->nyears; year++)
        {
            for (int age = 0; age < population->nages; age++)
            {
                int i_age_year = year * population->nages + age;

                catch_at_age_model->CalculateInitialNumbersAA(population, i_age_year, age);

                numbers_at_age[i_age_year] = fims_math::exp(population->log_init_naa[age]);
                EXPECT_EQ(dq["numbers_at_age"][i_age_year], numbers_at_age[i_age_year]);
            }
        }
    }

    TEST_F(CAAEvaluateTestFixture, CalculateUnfishedNumbersAAandUnfishedSpawningBiomass_works)
    {
        std::vector<double> test_unfished_numbers_at_age((nyears + 1) * nages, 0);
        std::vector<double> test_unfished_spawning_biomass(nyears+1, 0);
        
        size_t pop_id = population->GetId();
        auto& dq = catch_at_age_model->GetPopulationDerivedQuantities(pop_id);

        for (int year = 0; year < (population->nyears + 1); year++)
        {
            for (int age = 0; age < population->nages; age++)
            {
                int i_age_year = year * population->nages + age;

                if (age == 0)
                {            
                    dq["unfished_numbers_at_age"][i_age_year] = fims_math::exp(population->recruitment->log_rzero[0]);
                    test_unfished_numbers_at_age[i_age_year] = fims_math::exp(population->recruitment->log_rzero[0]);
                }

                if (year == 0 && age > 0){
                    
                    // values from FIMS
                    catch_at_age_model->CalculateUnfishedNumbersAA(population, i_age_year, age-1, age);
                    // true values from test
                    test_unfished_numbers_at_age[i_age_year] = 
                        test_unfished_numbers_at_age[i_age_year-1] * 
                        fims_math::exp(-fims_math::exp(population->log_M[i_age_year-1]));

                }

                if (year>0 && age > 0)
                {
                    int i_agem1_yearm1 = (year - 1) * population->nages + (age - 1);
                    EXPECT_GT(population->M[i_agem1_yearm1], 0.0);
                    // values from FIMS
                    catch_at_age_model->CalculateUnfishedNumbersAA(population,i_age_year, i_agem1_yearm1, age);
                    // true values from test
                    // unfished_numbers_at_age[i_age_year] = unfished_numbers_at_age[i_age_year-1] * fims_math::exp(-fims_math::exp(population.log_M[i_age_year-1]));
                    test_unfished_numbers_at_age[i_age_year] = 
                        test_unfished_numbers_at_age[i_agem1_yearm1] * 
                        fims_math::exp(-fims_math::exp(population->log_M[i_agem1_yearm1]));
                }

                if(age==(population->nages-1)){
                    int i_agem1_yearm1 = 0;
                    if(year == 0){
                        i_agem1_yearm1 = (age - 1);
                    } else{
                        i_agem1_yearm1 = (year - 1) * population->nages + (age - 1);
                    }
                        test_unfished_numbers_at_age[i_age_year] = 
                        test_unfished_numbers_at_age[i_age_year] +
                        test_unfished_numbers_at_age[i_agem1_yearm1 + 1] *
                        fims_math::exp(-fims_math::exp(population->log_M[i_agem1_yearm1 + 1]));

                }

                catch_at_age_model->CalculateMaturityAA(population,i_age_year, age);
                catch_at_age_model->CalculateUnfishedSpawningBiomass(population,i_age_year, year, age);
                
                test_unfished_spawning_biomass[year] += dq["proportion_mature_at_age"][i_age_year] *
                                                        population->proportion_female[age] *
                                                        test_unfished_numbers_at_age[i_age_year] *
                                                        population->growth->evaluate(population->ages[age]);


                EXPECT_EQ(dq["unfished_numbers_at_age"][i_age_year], test_unfished_numbers_at_age[i_age_year]);
                EXPECT_GT(dq["unfished_numbers_at_age"][i_age_year], 0.0);

            }

            EXPECT_NEAR(dq["unfished_spawning_biomass"][year], test_unfished_spawning_biomass[year], 1e-7);
            EXPECT_GT(dq["unfished_spawning_biomass"][year], 0.0);
        }
    }
}