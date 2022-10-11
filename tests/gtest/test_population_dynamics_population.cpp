#include "gtest/gtest.h"
#include "population_dynamics/population/population.hpp"
#include <random>

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
        int nfleets = 2;
    };

    TEST_F(PopulationTestData, input_data_are_specified)
    {
        EXPECT_EQ(population.id_g, id_g);
        EXPECT_EQ(population.nyears, nyears);
        EXPECT_EQ(population.nseasons, nseasons);
        EXPECT_EQ(population.nages, nages);
        EXPECT_EQ(population.nfleets, nfleets);
    }

    TEST_F(PopulationTestData, Initialize_works)
    {

        // Initialize is not called in evaluate()
        population.Initialize(nyears, nseasons, nages);

        // test failed: population.nfleets equals to 0 not nfleets
        // change nfleets = fleets.size(); to fleets.resize(nfleets);?
        EXPECT_EQ(population.nfleets, nfleets);
        EXPECT_EQ(population.ages.size(), nages);
        // What is catch_at_age? Is it used anywhere?
        // It is not catch_numbers_at_age or catch_weight_at_age
        EXPECT_EQ(population.catch_at_age.size(), nages);
        // test failed: population.nfleets equals to 0 not nfleets
        EXPECT_EQ(
            population.catch_numbers_at_age.size(),
            (nyears + 1) * nages * nfleets);
        EXPECT_EQ(population.mortality_F.size(), nyears * nages);
        EXPECT_EQ(population.mortality_Z.size(), nages);
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
        // Use (nyears+1)*nages or nyears*n
        EXPECT_EQ(population.unfished_numbers_at_age.size(), nages);
    }

    TEST_F(PopulationTestData, Prepare_works)
    {
        population.Initialize(nyears, nseasons, nages);
        
        // R code to set up true values for log_naa and naa:
        // set.seed(1234)
        // log_naa <- round(rnorm(12, 10, 3), digits = 4)
        // naa <- round(exp(log_naa), digit = 4)
        // paste(log_naa, collapse = ", ")
        // paste(naa, collapse = ", ")
        std::vector<double> log_naa = 
            {6.3788, 10.8323, 13.2533, 2.9629, 11.2874, 11.5182, 8.2758, 8.3601, 8.3066, 7.3299, 8.5684, 7.0048};
        std::vector<double> naa = 
            {589.2202, 50630.022, 569947.7677, 19.354, 79809.6737, 100528.847, 3927.6635, 4273.1221, 4050.5178, 1525.2292, 5262.7027, 1101.9097};
        population.log_naa = log_naa;

        // C++ code to set up true values for log_M:
        int seed = 1234;
        std::default_random_engine generator (seed);
        double M_min = 0.1;
        double M_max = 0.3;
        std::uniform_real_distribution<double> distribution(M_min, M_max);
        std::vector<double> M(nyears*nages, 0);
        for (int i = 0; i < nyears*nages; i++) {
            population.log_M[i] = fims::log(distribution(generator));
            M[i] = fims::exp(population.log_M[i]);
        }
        
        population.Prepare();

        // size of unfished_spawning_biomsss need to be 1 or nyears+1?
        EXPECT_EQ(
            population.unfished_spawning_biomass,
            std::vector<double>(1, 0) // vector size type = 1 and vector value = 0
        );

        // SEGFAULT when running the transformation Section
        // Need to call population.Initialize before calling population.Prepare() 
        // to initialize population.naa
        // Need to add population.log_naa before calling population.Prepare()
        for (int i = 0; i < naa.size(); i++)
        {
            EXPECT_EQ(population.log_naa[i], log_naa[i]);
            EXPECT_NEAR(population.naa[i], naa[i], 0.0001);
        }

        for (int i = 0; i < population.M.size(); i++)
        {
            EXPECT_NEAR(population.M[i], M[i], 0.01);
        }

    }
    
    TEST_F(PopulationTestData, CalculateMortality_works){
        population.Initialize(nyears, nseasons, nages);

        // C++ code to set up true values for Fmort:
        int seed = 1234;
        std::default_random_engine generator (seed);
        double distribution_min = 0.1;
        double distribution_max = 1.5;
        std::uniform_real_distribution<double> distribution(distribution_min, distribution_max);
        std::vector<double> Fmort(nfleets*nyears, 0);
        for (int i = 0; i < nfleets*nyears; i++) {
            Fmort[i] = distribution(generator);
        }

        std::vector<double> Mortality_F(nyears*nages, 0);
        
        // Not sure about the output of this -> fleets[fleet_] 
        
        population.Prepare();
        int index_ya = 2;
        int year = 3;
        int age = 4;

        fims::LogisticSelectivity<double> fishery_selectivity;
        fishery_selectivity.median = 20.5;
        fishery_selectivity.slope = 0.2;
        fishery_selectivity.evaluate(age);

        population.CalculateMortality(index_ya, year, age);

    }
} // namespace