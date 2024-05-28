#include "gtest/gtest.h"
#include "population_dynamics/fleet/fleet.hpp"
#include <random>

namespace
{

    TEST(FleetTests, FleetInitializeWorks)
    {
        fims_popdy::Fleet<double> fleet;
        int nyears = 30;
        int nages = 12;  
        fleet.expected_catch.resize(nyears);
        fleet.expected_index.resize(nyears);  
        fleet.catch_numbers_at_age.resize(nyears * nages);
        fleet.Initialize(nyears, nages);
        fleet.Prepare();
        
      
        EXPECT_EQ(fleet.log_Fmort.size(), nyears);
        EXPECT_EQ(fleet.Fmort.size(), nyears);
        EXPECT_EQ(fleet.catch_weight_at_age.size(), nyears*nages);
        EXPECT_EQ(fleet.catch_index.size(), nyears);
    }

    TEST(FleetTests, FleetPrepareWorks)
    {
        fims_popdy::Fleet<double> fleet;
        int nyears = 30;
        int nages = 12;
        fleet.expected_catch.resize(nyears);
        fleet.expected_index.resize(nyears);  
        fleet.catch_numbers_at_age.resize(nyears * nages);
         fleet.log_q.resize(1);//needs to be initialized here, size used by q in Initialize
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
       
        fleet.log_q[0] = log_q_distribution(generator);
        for(int i = 0; i < nyears; i++)
        {
            fleet.log_Fmort[i] = log_Fmort_distribution(generator);
        }
        
        fleet.Prepare();

        // Test fleet.Fmort and fleet.q
        std::vector<double> Fmort(nyears, 0);
        double q = fims_math::exp(fleet.log_q[0]);
        EXPECT_EQ(fleet.q[0], q);
        for (int i = 0; i < nyears; i++)
        {
            Fmort[i] = fims_math::exp(fleet.log_Fmort[i]);
            EXPECT_EQ(fleet.Fmort[i], Fmort[i]);

        }
        EXPECT_EQ(fleet.Fmort.size(), nyears);
    }
} // namespace
