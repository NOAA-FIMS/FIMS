#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
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

    TEST_F(PopulationInitializeTestFixture, Initialize_works)
    {

        population.Initialize(nyears, nseasons, nages);

        // test failed: population.nfleets equals to 0 not nfleets
        // change nfleets = fleets.size(); to fleets.resize(nfleets);?
        EXPECT_EQ(population.nfleets, nfleets);
        EXPECT_EQ(population.ages.size(), nages);
        // What is catch_at_age? Is it used anywhere?
        // It is not catch_numbers_at_age or catch_weight_at_age
        EXPECT_EQ(population.catch_at_age.size(), nages);
        // test failed: population.nfleets equals to 0 not nfleets
        // Use (nyears + 1) * nages * nfleets or nyears * nages * nfleets?
        // Is catch numbers-at-age in year nyears+1 known?
        EXPECT_EQ(
            population.catch_numbers_at_age.size(),
            (nyears + 1) * nages * nfleets);
        // Resize mortality_M in the population.hpp?
        EXPECT_EQ(population.mortality_F.size(), nyears * nages);
        // Use nages or nyears * nages for mortality_Z?
        // CalculateMortality() uses nyears * nages for mortality_Z
        EXPECT_EQ(population.mortality_Z.size(), nyears * nages);
        // Will input values be nyears + 1 or nyears?
        EXPECT_EQ(population.proportion_mature_at_age.size(), (nyears + 1) * nages);
        // Error: 'struct fims::Population<double>' has no member named 'initial_numbers'
        // EXPECT_EQ(population.initial_numbers.size(), nages);
        EXPECT_EQ(population.weight_at_age.size(), nages);
        // Use nyears*nages*nfleets or (nyears + 1) * nages * nfleets?
        // The size of catch_numbers_at_age is (nyears + 1) * nages * nfleets)
        EXPECT_EQ(
            population.catch_weight_at_age.size(),
            (nyears + 1) * nages * nfleets);
        // What is unfished number at age? A vector of values before
        // model start year or a vector of values for each model year?
        // Is unfished biomass_at_age needed?
        EXPECT_EQ(population.unfished_numbers_at_age.size(), nages);
        EXPECT_EQ(population.numbers_at_age.size(), (nyears + 1) * nages);
        // Does nfleets include both fishery and survey fleets?
        // How to distinguish between catch and survey indices?
        // Can expected_catch and expected_index be combined into one
        // vector like log_q?
        EXPECT_EQ(population.expected_catch.size(), nyears * nfleets);
        EXPECT_EQ(population.expected_index.size(), nyears * nfleets);
        EXPECT_EQ(population.biomass.size(), (nyears + 1));
        // What is unfished spawning biomass? A single value before
        // model start year or a vector of values for each year?
        EXPECT_EQ(population.unfished_spawning_biomass.size(), 1);
        EXPECT_EQ(population.spawning_biomass.size(), nyears + 1);
        EXPECT_EQ(population.log_naa.size(), nages);
        EXPECT_EQ(population.log_Fmort.size(), nfleets * nyears);
        EXPECT_EQ(population.log_M.size(), nyears * nages);
        // Is the dimention of log_q nfleets or nfleets * nyears?
        // Prepare() uses nfleets * nyears?
        EXPECT_EQ(population.log_q.size(), nfleets * nyears);
        EXPECT_EQ(population.naa.size(), nages);
        EXPECT_EQ(population.Fmort.size(), nfleets * nyears);

        //////////////////////////////////////////////////////////////////
        // Task: write a test for M.resize(nyears*nages); and q.resize(nfleets);
        //////////////////////////////////////////////////////////////////
    }

    TEST_F(PopulationPrepareTestFixture, Prepare_works)
    {

        // size of unfished_spawning_biomsss need to be 1 or nyears+1?
        EXPECT_EQ(
            population.unfished_spawning_biomass,
            std::vector<double>(1, 0) // vector size type = 1 and vector value = 0
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

        ///////////////////////////////////////////////////////////////////////////////
        // Task: Write a test for std::fill(expected_catch.begin(), expected_catch.end(), 0);
        ///////////////////////////////////////////////////////////////////////////////
        
        // Test population.naa
        std::vector<double> naa(nages, 0);
        for (int i = 0; i < nages; i++)
        {
            naa[i] = fims::exp(population.log_naa[i]);
            EXPECT_EQ(population.naa[i], naa[i]);
        }
        EXPECT_EQ(population.naa.size(), nages);

        // Test population.M
        std::vector<double> M(nyears * nages, 0);
        for (int i = 0; i < nyears * nages; i++)
        {
            M[i] = fims::exp(population.log_M[i]);
            EXPECT_EQ(population.M[i], M[i]);
        }
        EXPECT_EQ(population.M.size(), nyears * nages);

        // Test population.Fmort
        std::vector<double> Fmort(nfleets * nyears, 0);
        for (int i = 0; i < nfleets * nyears; i++)
        {
            Fmort[i] = fims::exp(population.log_Fmort[i]);
            EXPECT_EQ(population.Fmort[i], Fmort[i]);
        }
        EXPECT_EQ(population.Fmort.size(), nyears * nfleets);
    }
} // namespace