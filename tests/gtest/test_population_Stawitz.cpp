#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{
    TEST_F(PopulationPrepareTestFixture, CalculateNumbersAA_works)
    {

        // Change Fmort[index_yf] to this->Fmort[index_yf] in population.hpp?

        std::vector<double> mortality_F(nyears * nages, 0);
        std::vector<double> test_numbers_at_age(nyears * nages, 0);
        
        for(size_t year = 1; year <= nyears; year++){ 
            for(size_t age = 1; age < nages; age++){
                int index_ya = year * population.nages + age;
                int index_ya2 = (year - 1) * population.nages + age - 1;

                // mortality from the previous year/age combination
                population.CalculateMortality(index_ya2, (year-1), (age-1));
                
                //this needs to be after we calculate mortality                        
                population.CalculateNumbersAA(index_ya, index_ya2);

                    // loop over fleets to calculate mortality
                    for (size_t fleet_index = 0; fleet_index < population.nfleets; fleet_index++)
                    {
                        int index_yf = (year-1) * population.nfleets + fleet_index;
                        mortality_F[index_ya2] += population.Fmort[index_yf] *
                                                population.fleets[fleet_index]->selectivity->evaluate(age-1);
                    }
                    EXPECT_EQ(population.mortality_F[index_ya2], mortality_F[index_ya2]);
                
                    std::vector<double> mortality_Z(nyears * nages, 0);
                    mortality_Z[index_ya2] = fims::exp(population.log_M[index_ya2]) +
                                            mortality_F[index_ya2];
                
                test_numbers_at_age[index_ya] = population.numbers_at_age[index_ya2]*fims::exp(-population.mortality_Z[index_ya2]);

                EXPECT_EQ(population.numbers_at_age[index_ya], test_numbers_at_age[index_ya]);  
            }
        }                              
    }
}