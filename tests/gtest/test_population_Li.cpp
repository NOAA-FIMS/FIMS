#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{
    TEST_F(PopulationPrepareTestFixture, CalculateMortality_works)
    {

        // Change Fmort[index_yf] to this->Fmort[index_yf] in population.hpp?
        
        std::vector<double> mortality_F(nyears * nages, 0);
        std::vector<double> mortality_Z(nyears * nages, 0);

        for(size_t year = 0; year < nyears; year++){
            for(size_t age = 0; age < nages; age++){        
                int index_ya = year * population.nages + age;

                population.CalculateMortality(index_ya, year, age);

        
        for (int fleet_index = 0; fleet_index < population.nfleets; fleet_index++)
        {
            int index_yf = year * population.nfleets + fleet_index;
            mortality_F[index_ya] += population.Fmort[index_yf] *
                                     population.fleets[fleet_index]->selectivity->evaluate(age);
        }
        EXPECT_EQ(population.mortality_F[index_ya], mortality_F[index_ya]);
            
        mortality_Z[index_ya] = fims::exp(population.log_M[index_ya]) +
                                mortality_F[index_ya];
        EXPECT_EQ(population.mortality_Z[index_ya], mortality_Z[index_ya]);
    }
        }
    }
}