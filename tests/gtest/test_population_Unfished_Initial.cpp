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
                int index_ya = year * population.nages + age;
                population.CalculateMortality(index_ya, year, age);

                std::vector<double> mortality_F(nyears * nages, 0);

                for (int fleet_index = 0; fleet_index < population.nfleets; fleet_index++)
                {
                    size_t index_yf = year * population.nfleets + fleet_index;
                    mortality_F[index_ya] += population.fleets[fleet_index]->Fmort[index_yf] *
                                             population.fleets[fleet_index]->selectivity->evaluate(age);
                }
                EXPECT_EQ(population.mortality_F[index_ya], mortality_F[index_ya]);

                std::vector<double> mortality_Z(nyears * nages, 0);
                mortality_Z[index_ya] = fims::exp(population.log_M[index_ya]) +
                                        mortality_F[index_ya];
                EXPECT_EQ(population.mortality_Z[index_ya], mortality_Z[index_ya]);
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
                int index_ya = year * population.nages + age;

                population.CalculateInitialNumbersAA(index_ya, age);

                numbers_at_age[index_ya] = fims::exp(population.log_naa[age]);
                EXPECT_EQ(population.numbers_at_age[index_ya], numbers_at_age[index_ya]);
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
                int index_ya = year * population.nages + age;
 
                if (age == 0)
                {            
                    population.unfished_numbers_at_age[index_ya] = population.recruitment->rzero;
                    test_unfished_numbers_at_age[index_ya] = population.recruitment->rzero;
                }

                if (year == 0 && age > 0){
                    
                    // values from FIMS
                    // Bai: change CalculateUnfishedNumbersAA(index_ya, a); in 
                    // population.hpp to CalculateUnfishedNumbersAA(index_ya, index_ya-1);
                    // or to CalculateUnfishedNumbersAA(index_ya, a-1);?
                    // population.CalculateUnfishedNumbersAA(index_ya, age);
                    // population.CalculateUnfishedNumbersAA(index_ya, index_ya-1);
                    population.CalculateUnfishedNumbersAA(index_ya, age-1, age);
                    // true values from test
                    test_unfished_numbers_at_age[index_ya] = 
                        test_unfished_numbers_at_age[index_ya-1] * 
                        fims::exp(-fims::exp(population.log_M[index_ya-1]));

                }

                if (year>0 && age > 0)
                {
                    int index_ya2 = (year - 1) * population.nages + (age - 1);
                    EXPECT_GT(population.M[index_ya2], 0.0);
                    // values from FIMS
                    // Bai: change CalculateUnfishedNumbersAA(index_ya, index_ya2); in 
                    // population.hpp to CalculateUnfishedNumbersAA(index_ya, index_ya-1); ?
                    // Test fails if log_M from test fixture is not constant over years and ages.
                    population.CalculateUnfishedNumbersAA(index_ya, index_ya2, age);
                    // true values from test
                    // unfished_numbers_at_age[index_ya] = unfished_numbers_at_age[index_ya-1] * fims::exp(-fims::exp(population.log_M[index_ya-1]));
                    test_unfished_numbers_at_age[index_ya] = 
                        test_unfished_numbers_at_age[index_ya2] * 
                        fims::exp(-fims::exp(population.log_M[index_ya2]));
                }

                if(age==(population.nages-1)){
                    int index_ya2 = 0;
                    if(year == 0){
                        index_ya2 = (age - 1);
                    } else{
                        index_ya2 = (year - 1) * population.nages + (age - 1);
                    }
                        test_unfished_numbers_at_age[index_ya] = 
                        test_unfished_numbers_at_age[index_ya] +
                        test_unfished_numbers_at_age[index_ya2 + 1] *
                        fims::exp(-fims::exp(population.log_M[index_ya2 + 1]));

                }

                population.CalculateMaturityAA(index_ya, age);
                population.CalculateUnfishedSpawningBiomass(index_ya, year, age);
                
                test_unfished_spawning_biomass[year] += population.proportion_mature_at_age[index_ya] *
                                                        population.proportion_female *
                                                        test_unfished_numbers_at_age[index_ya] *
                                                        population.growth->evaluate(population.ages[age]);


                EXPECT_EQ(population.unfished_numbers_at_age[index_ya], test_unfished_numbers_at_age[index_ya]);
                EXPECT_GT(population.unfished_numbers_at_age[index_ya], 0.0);

            }
            
            EXPECT_EQ(population.unfished_spawning_biomass[year], test_unfished_spawning_biomass[year]);
            EXPECT_GT(population.unfished_spawning_biomass[year], 0.0);
        }
    }
}