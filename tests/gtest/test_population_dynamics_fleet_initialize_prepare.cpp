#include "gtest/gtest.h"
#include "population_dynamics/fleet/fleet.hpp"
#include <random>

namespace
{

    TEST(fleet_tests, Fleet_Initialize_works)
    {
        fims_popdy::Fleet<double> fleet;
        int nyears = 30;
        int nages = 12;
        fleet.Initialize(nyears, nages);
        fleet.Prepare();
        
      
        EXPECT_EQ(fleet.log_Fmort.size(), nyears);
        EXPECT_EQ(fleet.Fmort.size(), nyears);
        EXPECT_EQ(fleet.catch_weight_at_age.size(), nyears*nages);
        EXPECT_EQ(fleet.catch_index.size(), nyears);
    }

    TEST(fleet_tests, Fleet_Prepare_works)
    {
        fims_popdy::Fleet<double> fleet;
        int nyears = 30;
        int nages = 12;
        fleet.Initialize(nyears, nages);

        int seed = 1234;
        std::default_random_engine generator(seed);

          // log_Fmort
        double log_Fmort_min = fims_math::log(0.1);
        double log_Fmort_max = fims_math::log(2.3);
        std::uniform_real_distribution<double> log_Fmort_distribution(log_Fmort_min, log_Fmort_max);

        double log_q_min = fims_math::log(0.1);
        double log_q_max = fims_math::log(1);
        std::uniform_real_distribution<double> log_q_distribution(log_q_min, log_q_max);
        fleet.log_q = log_q_distribution(generator);
        for(int i = 0; i < nyears; i++)
        {
            fleet.log_Fmort[i] = log_Fmort_distribution(generator);
        }
        fleet.Prepare();

        // Test fleet.Fmort and fleet.q
        std::vector<double> Fmort(nyears, 0);
        double q = fims_math::exp(fleet.log_q);
        EXPECT_EQ(fleet.q, q);
        for (int i = 0; i < nyears; i++)
        {
            Fmort[i] = fims_math::exp(fleet.log_Fmort[i]);
            EXPECT_EQ(fleet.Fmort[i], Fmort[i]);

        }
        EXPECT_EQ(fleet.Fmort.size(), nyears);
    }
} // namespace
