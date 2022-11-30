#include "gtest/gtest.h"
#include "population_dynamics/fleet/fleet.hpp"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{

    TEST_F(PopulationInitializeTestFixture, input_data_are_specified)
    {
        EXPECT_EQ(population.id_g, id_g);
        EXPECT_EQ(population.nyears, nyears);
        EXPECT_EQ(population.nseasons, nseasons);
        EXPECT_EQ(population.nages, nages);
        EXPECT_EQ(population.nfleets, nfleets);
    }

    TEST_F(PopulationInitializeTestFixture, Fleet_Initialize_works)
    {
        
        fims::Fleet<double> fleet;
        fleet.Initialize(nyears, nages);

        EXPECT_EQ(fleet.log_Fmort.size(), nyears);
        EXPECT_EQ(fleet.log_q.size(), nyears);
        EXPECT_EQ(fleet.Fmort.size(), nyears);
        EXPECT_EQ(fleet.q.size(), nyears);
    }

    TEST_F(PopulationPrepareTestFixture, Fleet_Prepare_works)
    {
        fims::Fleet<double> fleet;
        fleet.Initialize(nyears, nages);
        fleet.Prepare();

        // Test fleet.Fmort
        std::vector<double> Fmort(nyears, 0);
        for (int i = 0; i < nyears; i++)
        {
            Fmort[i] = fims::exp(fleet.log_Fmort[i]);
            EXPECT_EQ(fleet.Fmort, Fmort[i]);
        }
        EXPECT_EQ(fleet.Fmort.size(), nyears);
    }
} // namespace