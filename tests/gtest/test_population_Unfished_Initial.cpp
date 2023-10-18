#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{

    TEST_F(PopulationPrepareTestFixture, CalculateMortality_works)
    {
        for (int year = 0; year < population.nyears; year++)
        {
            for (int age = 0; age < population.nages; age++)
            {
                int i_age_year = year * population.nages + age;
                // Call FIMS CalculateMortality() function to compare FIMS mortality values with "true" values later
                population.CalculateMortality(i_age_year, year, age);

                std::vector<double> mortality_F(nyears * nages, 0);
                for (int fleet_index = 0; fleet_index < population.nfleets; fleet_index++)
                { 
                    if(!population.fleets[fleet_index]->is_survey){
                    // Known values were used to generate "true" value and test CalculateMortality()
                    size_t index_yf = year * population.nfleets + fleet_index;
                    mortality_F[i_age_year] += population.fleets[fleet_index]->Fmort[year] *
                                             population.fleets[fleet_index]->selectivity->evaluate(population.ages[age]);
                
                    EXPECT_EQ(population.mortality_F[i_age_year], mortality_F[i_age_year]);
                

                std::vector<double> mortality_Z(nyears * nages, 0);
                mortality_Z[i_age_year] = fims::exp(population.log_M[i_age_year]) +
                                        mortality_F[i_age_year];
                EXPECT_EQ(population.mortality_Z[i_age_year], mortality_Z[i_age_year]);
                    }
            }
        }
    }
    }

    TEST_F(PopulationPrepareTestFixture, CalculateInitialNumbersAA_works)
    {

        std::vector<double> numbers_at_age(nyears * nages, 0);

        for (int year = 0; year < population.nyears; year++)
        {
            for (int age = 0; age < population.nages; age++)
            {
                int i_age_year = year * population.nages + age;

                population.CalculateInitialNumbersAA(i_age_year, age);

                numbers_at_age[i_age_year] = fims::exp(population.log_init_naa[age]);
                EXPECT_EQ(population.numbers_at_age[i_age_year], numbers_at_age[i_age_year]);
            }
        }
    }

    TEST_F(PopulationPrepareTestFixture, CalculateUnfishedNumbersAAandUnfishedSpawningBiomass_works)
    {
        std::vector<double> test_unfished_numbers_at_age((nyears + 1) * nages, 0);
        std::vector<double> test_unfished_spawning_biomass(nyears+1, 0);

        for (int year = 0; year < (population.nyears + 1); year++)
        {
            for (int age = 0; age < population.nages; age++)
            {
                int i_age_year = year * population.nages + age;
 
                if (age == 0)
                {            
                    population.unfished_numbers_at_age[i_age_year] = fims::exp(population.recruitment->log_rzero);
                    test_unfished_numbers_at_age[i_age_year] = fims::exp(population.recruitment->log_rzero);
                }

                if (year == 0 && age > 0){
                    
                    // values from FIMS
                    // Bai: change CalculateUnfishedNumbersAA(i_age_year, a); in 
                    // population.hpp to CalculateUnfishedNumbersAA(i_age_year, i_age_year-1);
                    // or to CalculateUnfishedNumbersAA(i_age_year, a-1);?
                    // population.CalculateUnfishedNumbersAA(i_age_year, age);
                    // population.CalculateUnfishedNumbersAA(i_age_year, i_age_year-1);
                    population.CalculateUnfishedNumbersAA(i_age_year, age-1, age);
                    // true values from test
                    test_unfished_numbers_at_age[i_age_year] = 
                        test_unfished_numbers_at_age[i_age_year-1] * 
                        fims::exp(-fims::exp(population.log_M[i_age_year-1]));

                }

                if (year>0 && age > 0)
                {
                    int i_agem1_yearm1 = (year - 1) * population.nages + (age - 1);
                    EXPECT_GT(population.M[i_agem1_yearm1], 0.0);
                    // values from FIMS
                    // Bai: change CalculateUnfishedNumbersAA(i_age_year, i_agem1_yearm1); in 
                    // population.hpp to CalculateUnfishedNumbersAA(i_age_year, i_age_year-1); ?
                    // Test fails if log_M from test fixture is not constant over years and ages.
                    population.CalculateUnfishedNumbersAA(i_age_year, i_agem1_yearm1, age);
                    // true values from test
                    // unfished_numbers_at_age[i_age_year] = unfished_numbers_at_age[i_age_year-1] * fims::exp(-fims::exp(population.log_M[i_age_year-1]));
                    test_unfished_numbers_at_age[i_age_year] = 
                        test_unfished_numbers_at_age[i_agem1_yearm1] * 
                        fims::exp(-fims::exp(population.log_M[i_agem1_yearm1]));
                }

                if(age==(population.nages-1)){
                    int i_agem1_yearm1 = 0;
                    if(year == 0){
                        i_agem1_yearm1 = (age - 1);
                    } else{
                        i_agem1_yearm1 = (year - 1) * population.nages + (age - 1);
                    }
                        test_unfished_numbers_at_age[i_age_year] = 
                        test_unfished_numbers_at_age[i_age_year] +
                        test_unfished_numbers_at_age[i_agem1_yearm1 + 1] *
                        fims::exp(-fims::exp(population.log_M[i_agem1_yearm1 + 1]));

                }

                population.CalculateMaturityAA(i_age_year, age);
                population.CalculateUnfishedSpawningBiomass(i_age_year, year, age);
                
                test_unfished_spawning_biomass[year] += population.proportion_mature_at_age[i_age_year] *
                                                        population.proportion_female *
                                                        test_unfished_numbers_at_age[i_age_year] *
                                                        population.growth->evaluate(population.ages[age]);


                EXPECT_EQ(population.unfished_numbers_at_age[i_age_year], test_unfished_numbers_at_age[i_age_year]);
                EXPECT_GT(population.unfished_numbers_at_age[i_age_year], 0.0);

            }
            
            EXPECT_EQ(population.unfished_spawning_biomass[year], test_unfished_spawning_biomass[year]);
            EXPECT_GT(population.unfished_spawning_biomass[year], 0.0);
        }
    }
}