#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{

    TEST_F(PopulationPrepareTestFixture, CalculateMortality_works)
    {
        // Heap corruption (Exit code 0xc0000374) occurs when using
        // year <= population.nyears. Details from the failed test:
        // Failure Expected equality of these values:
        // population.mortality_Z[index_ya]
        //  Which is: -5.20867e-98
        // mortality_Z[index_ya]
        //  Which is: 1

        // for (size_t year = 0; year <= population.nyears; year++)
        for (size_t year = 0; year < population.nyears; year++)
        {
            for (size_t age = 0; age < population.nages; age++)
            {
                size_t index_ya = year * population.nages + age;
                population.CalculateMortality(index_ya, year, age);

                std::vector<double> mortality_F(nyears * nages, 0);

                for (size_t fleet_index = 0; fleet_index < population.nfleets; fleet_index++)
                {
                    size_t index_yf = year * population.nfleets + fleet_index;
                    mortality_F[index_ya] += population.Fmort[index_yf] *
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
}