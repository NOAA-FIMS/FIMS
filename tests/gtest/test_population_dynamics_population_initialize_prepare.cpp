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

        population.numbers_at_age.resize((nyears + 1) * nages);
        population.Initialize(nyears, nseasons, nages);

        EXPECT_EQ(population.nfleets, nfleets);
        EXPECT_EQ(population.ages.size(), nages);
        EXPECT_EQ(population.mortality_F.size(), nyears * nages);
        EXPECT_EQ(population.mortality_Z.size(), nyears * nages);
        EXPECT_EQ(population.proportion_mature_at_age.size(), (nyears+1) * nages);
        EXPECT_EQ(population.weight_at_age.size(), nages);
        EXPECT_EQ(population.unfished_numbers_at_age.size(), (nyears + 1) * nages);
        EXPECT_EQ(population.numbers_at_age.size(), (nyears + 1) * nages);
        EXPECT_EQ(population.expected_catch.size(), nyears * nfleets);
        EXPECT_EQ(population.biomass.size(), (nyears + 1));
        EXPECT_EQ(population.unfished_spawning_biomass.size(), (nyears + 1));
        EXPECT_EQ(population.spawning_biomass.size(), nyears + 1);
        EXPECT_EQ(population.log_init_naa.size(), nages);
        EXPECT_EQ(population.proportion_female.size(), nages);
        EXPECT_EQ(population.log_M.size(), nyears * nages);
        EXPECT_EQ(population.M.size(), nyears * nages);
    }

    TEST_F(PopulationPrepareTestFixture, Prepare_works)
    {
        
        EXPECT_EQ(
            population.unfished_spawning_biomass,
            fims::Vector<double>(nyears + 1, 0) // vector size type = 1 and vector value = 0
        );

        for (int i = 0; i < population.spawning_biomass.size(); i++)
        {
            EXPECT_EQ(
                population.spawning_biomass,
                fims::Vector<double>(nyears + 1, 0) // vector size type = 1 and vector value = 0)
            );
        };

        for (int i = 0; i < population.mortality_F.size(); i++)
        {
            EXPECT_EQ(
                population.mortality_F,
                fims::Vector<double>(nyears * nages, 0) // vector size type = 1 and vector value = 0)
            );
        };

        for (int i = 0; i < population.expected_catch.size(); i++)
        {
            EXPECT_EQ(
                population.expected_catch,
                fims::Vector<double>(nyears * nfleets, 0)
            );
        };

        // Test population.naa
        fims::Vector<double> naa(nages, 0);
        for (int i = 0; i < nages; i++)
        {
            naa[i] = fims_math::exp(population.log_init_naa[i]);
        }

        // Test population.M
        fims::Vector<double> M(nyears * nages, 0);
        for (int i = 0; i < nyears * nages; i++)
        {
            M[i] = fims_math::exp(population.log_M[i]);
            EXPECT_EQ(population.M[i], M[i]);
        }
        EXPECT_EQ(population.M.size(), nyears * nages);

        // Test population.proportion_female
        fims::Vector<double> p_female(nages, 0.5);
        for(int i = 0; i < nages; i++)
        {
            EXPECT_EQ(population.proportion_female[i], p_female[i]);
        }

        // Test population.fleet->Fmort 
        // fmort and logfmort are vectors of length year
        fims::Vector<double> Fmort(nfleets * nyears, 0);
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

