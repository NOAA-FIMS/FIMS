#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"

namespace
{

    // Use test fixture to reuse the same configuration of objects for
    // several different tests. To use a test fixture, derive a class
    // from testing::Test.
    class PopulationTestData : public testing::Test
    {

        // Make members protected otherwise they can be accessed from
        // sub-classes.
    protected:
        // Use SetUp function to prepare the objects for each test.
        // Use override in C++11 to make sure SetUp (e.g., not Setup with
        // a lowercase u) is spelled
        // correctly.
        void SetUp() override
        {
            population.id_g = id_g;
            population.nyears = nyears;
            population.nseasons = nseasons;
            population.nages = nages;
            population.nfleets = nfleets;
        }

        // Virtual void TearDown() will be called after each test is
        // run. It needs to be defined if there is clearup work to
        // do. Otherwise, it does not need to be provided.
        virtual void TearDown()
        {
        }

        fims::Population<double> population;
        // Use default values from the Li et al., 2021
        // https://github.com/Bai-Li-NOAA/Age_Structured_Stock_Assessment_Model_Comparison/blob/master/R/save_initial_input.R
        int id_g = 0;
        int nyears = 30;
        int nseasons = 1;
        int nages = 12;
        int nfleets = 1;
    };

    TEST_F(PopulationTestData, input_data_are_specified)
    {
        EXPECT_EQ(population.id_g, id_g);
        EXPECT_EQ(population.nyears, nyears);
        EXPECT_EQ(population.nseasons, nseasons);
        EXPECT_EQ(population.nages, nages);
        EXPECT_EQ(population.nfleets, nfleets);
    }

    TEST_F(PopulationTestData, initialize_function_works)
    {

        population.Initialize(nyears, nseasons, nages);

        // test failed: population.nfleets equals to 0 not nfleets
        // change nfleets = fleets.size(); to fleets.resize(nfleets);?
        EXPECT_EQ(population.nfleets, nfleets);
        EXPECT_NE(population.nfleets, nfleets + 1);

        EXPECT_EQ(population.ages.size(), nages);
        EXPECT_NE(population.ages.size(), nages + 1);

        // What is catch_at_age? Is it used anywhere?
        // It is not catch_numbers_at_age or catch_weight_at_age
        EXPECT_EQ(population.catch_at_age.size(), nages);
        EXPECT_NE(population.catch_at_age.size(), nages + 1);

        // test failed: population.nfleets equals to 0 not nfleets
        EXPECT_EQ(
            population.catch_numbers_at_age.size(),
            (nyears + 1) * nages * nfleets);
        EXPECT_NE(
            population.catch_numbers_at_age.size(),
            (nyears + 1) * nages * nfleets + 1);

        EXPECT_EQ(population.mortality_F.size(), nyears * nages);
        EXPECT_NE(population.mortality_F.size(), nyears * nages + 1);
    }

} // namespace