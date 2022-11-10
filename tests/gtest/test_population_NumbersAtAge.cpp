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
        std::vector<double> mortality_Z(nyears * nages, 0);

        //need to calculate values for all ages in year 0
        size_t y = 0;
        for(size_t a = 0; a < nages; a++){
            int index_ya1 = y*population.nages + a;
            population.CalculateMortality(index_ya1, y, a);
            test_numbers_at_age[index_ya1] = population.numbers_at_age[index_ya1];
            for(size_t f = 0; f < population.nfleets; f++){
                size_t index_yf1 = y*population.nfleets + f;
                mortality_F[index_ya1] += population.Fmort[index_yf1] * population.fleets[f]->selectivity->evaluate(a);
            }
        }

        //need to calculate values for all ages in year 0
        size_t a = 0;
        for(size_t y = 1; y < nyears; y++){
            int index_ya1 = y*population.nages + a;
            population.CalculateMortality(index_ya1, y, a);
            test_numbers_at_age[index_ya1] = population.numbers_at_age[index_ya1];
            for(size_t f = 0; f < population.nfleets; f++){
                size_t index_yf1 = y*population.nfleets + f;
                mortality_F[index_ya1] += population.Fmort[index_yf1] * population.fleets[f]->selectivity->evaluate(a);
            }
        }
        
        for(size_t year = 1; year < nyears; year++){ 
           for(size_t age = 1; age < nages; age++){
                int index_ya = year * population.nages + age;
                int index_ya2 = (year-1) * population.nages + (age - 1);

                // mortality from the previous year/age combination
                population.CalculateMortality(index_ya2, (year - 1), (age - 1));

                // this needs to be after we calculate mortality
                population.CalculateMortality(index_ya2, year-1, age-1);
                
                //this needs to be after we calculate mortality                        
                population.CalculateNumbersAA(index_ya, index_ya2);

                // loop over fleets to calculate mortality
                for (size_t fleet_index = 0; fleet_index < population.nfleets; fleet_index++)
                {
                    int index_yf = (year - 1) * population.nfleets + fleet_index;
                    mortality_F[index_ya2] += population.Fmort[index_yf] *
                                              population.fleets[fleet_index]->selectivity->evaluate(age - 1);
                }
                EXPECT_EQ(population.mortality_F[index_ya2], mortality_F[index_ya2]);

                std::vector<double> mortality_Z(nyears * nages, 0.0);
                mortality_Z[index_ya2] = fims::exp(population.log_M[index_ya2]) +
                                         mortality_F[index_ya2];

                test_numbers_at_age[index_ya] = population.numbers_at_age[index_ya2] * fims::exp(-population.mortality_Z[index_ya2]);

                EXPECT_EQ(population.numbers_at_age[index_ya], test_numbers_at_age[index_ya]);
            }
        }
    }

    TEST_F(PopulationPrepareTestFixture, CalculateNumbersAA_forloop_works)
    {

        for (size_t year = 0; year < nyears; year++)
        {
            for (size_t age = 0; age < nages; age++)
            {

                int index_ya = year * population.nages + age;
                // Calculate mortality starts from year = 0 and age = 0
                population.CalculateMortality(index_ya, year, age);

                // loop over fleets to calculate mortality and set up
                // true values for the test
                std::vector<double> mortality_F(nyears * nages, 0.0);

                for (size_t fleet_index = 0; fleet_index < population.nfleets; fleet_index++)
                {
                    size_t index_yf = year * population.nfleets + fleet_index;
                    mortality_F[index_ya] += population.Fmort[index_yf] *
                                             population.fleets[fleet_index]->selectivity->evaluate(age);
                }

                std::vector<double> mortality_Z(nyears * nages, 0.0);
                mortality_Z[index_ya] = fims::exp(population.log_M[index_ya]) +
                                        mortality_F[index_ya];
                EXPECT_EQ(population.mortality_Z[index_ya], mortality_Z[index_ya]);

                std::vector<double> test_numbers_at_age((nyears + 1) * nages, 0.0);
                if (year == 0)
                {
                    // Calculate initial numbers at age when year = 0
                    population.CalculateInitialNumbersAA(index_ya, age);
                    // Set up true values for the test
                    test_numbers_at_age[index_ya] = population.naa[age];
                    EXPECT_EQ(fims::exp(population.log_naa[age]), test_numbers_at_age[index_ya]);
                }

                if (year > 0 && age == 0)
                {
                    population.CalculateRecruitment(index_ya, year);
                    // The test will get complicated if we calculate
                    // test_numbers_at_age by calling recruitment module
                }

                if (year > 0 && age > 0)
                {
                    int index_ya2 = (year - 1) * population.nages + age - 1;
                    // this needs to be after we calculate mortality
                    population.CalculateNumbersAA(index_ya, index_ya2);
                    test_numbers_at_age[index_ya] = test_numbers_at_age[index_ya2] * fims::exp(-mortality_Z[index_ya2]);
                }
                    // loop over fleets to calculate mortality
                    for (size_t fleet_index = 0; fleet_index < population.nfleets; fleet_index++)
                    {
                        int index_yf = year * population.nfleets + fleet_index;
                        mortality_F[index_ya2] += population.Fmort[index_yf] *
                                                population.fleets[fleet_index]->selectivity->evaluate(age - 1);
                    }
                    //
                    EXPECT_EQ(population.mortality_F[index_ya2], mortality_F[index_ya2]);
                
                    
                    mortality_Z[index_ya2] = fims::exp(population.log_M[index_ya2]) +
                                            mortality_F[index_ya2];

                
                test_numbers_at_age[index_ya2] = population.numbers_at_age[index_ya2]*fims::exp(-population.mortality_Z[index_ya2]);

                EXPECT_EQ(population.numbers_at_age[index_ya], test_numbers_at_age[index_ya]);
            }
        }
    }

}