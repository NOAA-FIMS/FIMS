#include <random>
#include "population_dynamics/population/population.hpp"



namespace
{

    // Use test fixture to reuse the same configuration of objects for
    // several different tests. To use a test fixture, derive a class
    // from testing::Test.
    class PopulationInitializeTestFixture : public testing::Test
    {

        // Make members protected and they can be accessed from
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
            for (int i = 0; i < nfleets; i++)
            {
                auto fleet = std::make_shared<fims::Fleet<double>>();
                population.fleets.push_back(fleet);
            }
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

    class PopulationPrepareTestFixture : public testing::Test
    {
    protected:
        void SetUp() override
        {
            population.id_g = id_g;
            population.nyears = nyears;
            population.nseasons = nseasons;
            population.nages = nages;
            population.nfleets = nfleets;

            population.Initialize(nyears, nseasons, nages);

            // C++ code to set up true values for log_naa, log_M,
            // log_Fmort, and log_q:
            int seed = 1234;
            std::default_random_engine generator(seed);

             // log_Fmort
            double log_Fmort_min = fims::log(0.1);
            double log_Fmort_max = fims::log(2.3);
            std::uniform_real_distribution<double> log_Fmort_distribution(log_Fmort_min, log_Fmort_max);

            // log_q
            double log_q_min = fims::log(0.1);
            double log_q_max = fims::log(1);
            std::uniform_real_distribution<double> log_q_distribution(log_q_min, log_q_max);
            // Does Fmort need to be in side of the year loop like log_q?
            for (int i = 0; i < nfleets; i++)
            {
                auto fleet = std::make_shared<fims::Fleet<double>>();
                fleet->Initialize(nyears, nages);
                for(int year = 0; year < nyears; year++)
                {
                    fleet->log_Fmort[year] = log_Fmort_distribution(generator);
                    fleet->log_q[year] = log_q_distribution(generator);
                }
                fleet->Prepare();
                population.fleets.push_back(fleet);
            }

            // log_naa
            double log_naa_min = 10.0;
            double log_naa_max = 12.0;
            std::uniform_real_distribution<double> log_naa_distribution(log_naa_min, log_naa_max);
            for (int i = 0; i < nages; i++)
            {
                population.log_naa[i] = log_naa_distribution(generator);
            }

            // log_M
            double log_M_min = fims::log(0.1);
            double log_M_max = fims::log(0.3);
            std::uniform_real_distribution<double> log_M_distribution(log_M_min, log_M_max);
            for (int i = 0; i < nyears * nages; i++)
            {
                population.log_M[i] = log_M_distribution(generator);
            }      

            // numbers_at_age
            double numbers_at_age_min = fims::exp(10.0);
            double numbers_at_age_max = fims::exp(12.0);
            std::uniform_real_distribution<double> numbers_at_age_distribution(numbers_at_age_min, numbers_at_age_max);
            for (int i = 0; i < (nyears + 1) * nages; i++)
            {
                population.numbers_at_age[i] = numbers_at_age_distribution(generator);
            }

            // weight_at_age
            double weight_at_age_min = 0.5;
            double weight_at_age_max = 12.0;
            std::uniform_real_distribution<double> weight_at_age_distribution(weight_at_age_min, weight_at_age_max);
            for (int i = 0; i < nages; i++)
            {
                population.weight_at_age[i] = weight_at_age_distribution(generator);
            }


            population.Prepare();

            // Make a shared pointer to selectivity and fleet because
            // fleet object needs a shared pointer in fleet.hpp
            // (std::shared_ptr<fims::SelectivityBase<Type> > selectivity;)
            // and population object needs a shared pointer in population.hpp
            // (std::vector<std::shared_ptr<fims::Fleet<Type> > > fleets;)
            for (int i = 0; i < population.nfleets; i++)
            {
                auto selectivity = std::make_shared<fims::LogisticSelectivity<double>>();
                selectivity->median = 7;
                selectivity->slope = 0.5;

                auto fleet = std::make_shared<fims::Fleet<double>>();
                fleet->Initialize(nyears, nages);
                fleet->selectivity = selectivity;
                population.fleets[i] = fleet;
            }

            auto maturity = std::make_shared<fims::LogisticMaturity<double>>();
            maturity->median = 6;
            maturity->slope = 0.15;
            population.maturity = maturity;
            
            auto recruitment = std::make_shared<fims::SRBevertonHolt<double>>();
            recruitment->steep = 0.75;
            recruitment->rzero = 1000000.0;
            population.recruitment = recruitment;
            

        }

        virtual void TearDown()
        {
        }

        fims::Population<double> population;
        int id_g = 0;
        int nyears = 30;
        int nseasons = 1;
        int nages = 12;
        int nfleets = 2;
    };
}