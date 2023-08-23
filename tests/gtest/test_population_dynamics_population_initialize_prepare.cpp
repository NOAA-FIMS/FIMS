#include "gtest/gtest.h"
#include "../../tests/gtest/test_population_test_fixture.hpp"

namespace
{

    TEST_F(PopulationInitializeTestFixture, input_data_are_specified)
    {
        EXPECT_EQ(population.id_g, id_g);
        EXPECT_EQ(population.nyears, nyears);
        EXPECT_EQ(population.nseasons, nseasons);
        EXPECT_EQ(population.nages, nages);
        EXPECT_EQ(population.fleets.size(), nfleets);
    }

    TEST_F(PopulationInitializeTestFixture, Initialize_works)
    {

        population.Initialize(nyears, nseasons, nages);

        EXPECT_EQ(population.nfleets, nfleets);
        EXPECT_EQ(population.ages.size(), nages);
        EXPECT_EQ(population.mortality_F.size(), nyears * nages);
        EXPECT_EQ(population.mortality_Z.size(), nyears * nages);
        EXPECT_EQ(population.proportion_mature_at_age.size(), (nyears+1) * nages);
        EXPECT_EQ(population.weight_at_age.size(), nages);
        // What is unfished number at age? A vector of values before
        // model start year or a vector of values for each model year?
        // Is unfished biomass_at_age needed?
        EXPECT_EQ(population.unfished_numbers_at_age.size(), (nyears + 1) * nages);
        EXPECT_EQ(population.numbers_at_age.size(), (nyears + 1) * nages);
        EXPECT_EQ(population.expected_catch.size(), nyears * nfleets);
        EXPECT_EQ(population.biomass.size(), (nyears + 1));
        // What is unfished spawning biomass? A single value before
        // model start year or a vector of values for each year?
        EXPECT_EQ(population.unfished_spawning_biomass.size(), (nyears + 1));
        EXPECT_EQ(population.spawning_biomass.size(), nyears + 1);
        EXPECT_EQ(population.log_init_naa.size(), nages);
        EXPECT_EQ(population.log_M.size(), nyears * nages);
        EXPECT_EQ(population.M.size(), nyears * nages);
    }

    TEST_F(PopulationPrepareTestFixture, Prepare_works)
    {

        // size of unfished_spawning_biomsss need to be 1 or nyears+1?
        EXPECT_EQ(
            population.unfished_spawning_biomass,
            std::vector<double>(nyears + 1, 0) // vector size type = 1 and vector value = 0
        );

        for (int i = 0; i < population.spawning_biomass.size(); i++)
        {
            EXPECT_EQ(
                population.spawning_biomass,
                std::vector<double>(nyears + 1, 0) // vector size type = 1 and vector value = 0)
            );
        };

        for (int i = 0; i < population.mortality_F.size(); i++)
        {
            EXPECT_EQ(
                population.mortality_F,
                std::vector<double>(nyears * nages, 0) // vector size type = 1 and vector value = 0)
            );
        };

        for (int i = 0; i < population.expected_catch.size(); i++)
        {
            EXPECT_EQ(
                population.expected_catch,
                std::vector<double>(nyears * nfleets, 0)
            );
        };

        // Do we need to use std::fill() for catchability q and log_q?

        // Test population.naa
        std::vector<double> naa(nages, 0);
        for (int i = 0; i < nages; i++)
        {
            naa[i] = fims_math::exp(population.log_init_naa[i]);
        }

        // Test population.M
        std::vector<double> M(nyears * nages, 0);
        for (int i = 0; i < nyears * nages; i++)
        {
            M[i] = fims_math::exp(population.log_M[i]);
            EXPECT_EQ(population.M[i], M[i]);
        }
        EXPECT_EQ(population.M.size(), nyears * nages);

        // Test population.fleet->Fmort 
        // fmort and logfmort are vectors of length year
        std::vector<double> Fmort(nfleets * nyears, 0);
        for(size_t i = 0; i < nfleets; i++){
            for(size_t y = 0; y < nyears; y++){
                size_t index_yf = y * population.nfleets + i;
                Fmort[index_yf] = fims_math::exp(population.fleets[i]->log_Fmort[y]);
                EXPECT_EQ(population.fleets[i]->Fmort[y], Fmort[index_yf]);
            }
            EXPECT_EQ(population.fleets[i]->Fmort.size(), nyears);
        }
        
    }
} // namespace

