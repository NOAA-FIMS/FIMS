#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{
    TEST_F(PopulationPrepareTestFixture, CalculateNumbersAA_works)
    {

        // Change Fmort[index_yf] to this->Fmort[index_yf] in population.hpp?

        int year = 4;
        int age = 6;
        int index_ya = year * population.nages + age;
        int index_ya2 = (year - 1) * population.nages + age - 1;

        population.CalculateMortality(index_ya, year, age);
        population.CalculateNumbersAA(index_ya, index_ya2);

        std::vector<double> mortality_F(nyears * nages, 0);
        std::vector<double> test_naa(nyears * nages, 0);
        
        for (size_t fleet_index = 0; fleet_index < population.nfleets; fleet_index++)
        {
            int index_yf = year * population.nfleets + fleet_index;
            mortality_F[index_ya] += population.Fmort[index_yf] *
                                     population.fleets[fleet_index]->selectivity->evaluate(age);
        }
        EXPECT_EQ(population.mortality_F[index_ya], mortality_F[index_ya]);

        std::vector<double> mortality_Z(nyears * nages, 0);
        mortality_Z[index_ya] = fims::exp(population.log_M[index_ya]) +
                                mortality_F[index_ya];

        //should this be <=? Are we calculating NYY in terminal year + 1?
        test_naa[index_ya] = test_naa[index_ya2]*exp(-population.mortality_Z[index_ya2]);

        EXPECT_EQ(population.numbers_at_age[index_ya], test_naa[index_ya]);                                
    }
}