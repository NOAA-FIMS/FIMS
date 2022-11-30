#include "gtest/gtest.h"
#include "population_dynamics/fleet/fleet.hpp"
#include <random>

namespace
{

    TEST(fleet_tests, Fleet_Initialize_works)
    {
        fims::Fleet<double> fleet;
        int nyears = 30;
        int nages = 12;
        fleet.Initialize(nyears, nages);
        fleet.Prepare();
        
      
        EXPECT_EQ(fleet.log_Fmort.size(), nyears);
        EXPECT_EQ(fleet.log_q.size(), nyears);
        EXPECT_EQ(fleet.Fmort.size(), nyears);
        EXPECT_EQ(fleet.q.size(), nyears);
    }

    TEST(fleet_tests, Fleet_Prepare_works)
    {
        fims::Fleet<double> fleet;
        int nyears = 30;
        int nages = 12;
        fleet.Initialize(nyears, nages);

        int seed = 1234;
        std::default_random_engine generator(seed);

          // log_Fmort
        double log_Fmort_min = fims::log(0.1);
        double log_Fmort_max = fims::log(2.3);
        std::uniform_real_distribution<double> log_Fmort_distribution(log_Fmort_min, log_Fmort_max);
        for(int i = 0; i < nyears; i++)
        {
            fleet.log_Fmort[i] = log_Fmort_distribution(generator);
        }
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