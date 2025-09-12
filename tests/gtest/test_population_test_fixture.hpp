#ifndef TEST_POPULATION_TEST_FIXTURE_HPP
#define TEST_POPULATION_TEST_FIXTURE_HPP
#include <random>

#include "../../inst/include/models/functors/catch_at_age.hpp"
#include "population_dynamics/population/population.hpp"

namespace
{

  // Use test fixture to reuse the same configuration of objects for
  // several different tests. To use a test fixture, derive a class
  // from testing::Test.
  class CAAInitializeTestFixture : public testing::Test
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
      population = std::make_shared<fims_popdy::Population<double>>();
      catch_at_age_model = std::make_shared<fims_popdy::CatchAtAge<double>>();
      population->id_g = id_g;
      population->nyears = nyears;
      population->nseasons = nseasons;
      population->nages = nages;
      population->nfleets = nfleets;

      for (int i = 0; i < nfleets; i++)
      {
        auto fleet = std::make_shared<fims_popdy::Fleet<double>>();
        fleet->nyears = nyears;
        fleet->nages = nages;
        fleet->nlengths = nlengths;
        fleet->log_q.resize(1);
        population->fleets.push_back(fleet);
        catch_at_age_model->fleets[fleet->GetId()] =
            fleet; // Add to CatchAtAge model's fleets map
      }
      catch_at_age_model->populations.push_back(population);
    }

    /**
     * @brief Initialize CatchAtAge model derived quantities and population/fleet
     */
    void InitializeCAA()
    {
      typedef fims_popdy::CatchAtAge<double>::fleet_iterator fleet_iterator;

      //       // The following are initialized in the rcpp interface: ages, log_init_naa,
      //       //   numbers_at_age, log_Fmort, log_q
      for (size_t p = 0; p < this->catch_at_age_model->populations.size(); p++)
      {
        std::map<std::string, fims::Vector<double>> &derived_quantities =
            this->catch_at_age_model->GetPopulationDerivedQuantities(this->catch_at_age_model->populations[p]->GetId());

     
        derived_quantities["total_landings_weight"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears);

        derived_quantities["total_landings_numbers"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears);

        derived_quantities["mortality_F"] = fims::Vector<double>(
            this->catch_at_age_model->populations[p]->nyears * this->catch_at_age_model->populations[p]->nages);

        derived_quantities["mortality_Z"] = fims::Vector<double>(
            this->catch_at_age_model->populations[p]->nyears * this->catch_at_age_model->populations[p]->nages);

        // TODO: numbers_at_age are resized in rcpp_population, should this be
        // removed?
        derived_quantities["numbers_at_age"] = fims::Vector<double>(
            (this->catch_at_age_model->populations[p]->nyears + 1) * this->catch_at_age_model->populations[p]->nages);

        derived_quantities["unfished_numbers_at_age"] = fims::Vector<double>(
            (this->catch_at_age_model->populations[p]->nyears + 1) * this->catch_at_age_model->populations[p]->nages);

        derived_quantities["biomass"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears + 1);

        derived_quantities["spawning_biomass"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears + 1);

        derived_quantities["unfished_biomass"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears + 1);

        derived_quantities["unfished_spawning_biomass"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears + 1);

        derived_quantities["proportion_mature_at_age"] = fims::Vector<double>(
            (this->catch_at_age_model->populations[p]->nyears + 1) * this->catch_at_age_model->populations[p]->nages);

        derived_quantities["expected_recruitment"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears + 1);

        derived_quantities["sum_selectivity"] = fims::Vector<double>(
            this->catch_at_age_model->populations[p]->nyears * this->catch_at_age_model->populations[p]->nages);

        this->catch_at_age_model->populations[p]->proportion_female.resize(
            this->catch_at_age_model->populations[p]->nages);

        this->catch_at_age_model->populations[p]->M.resize(this->catch_at_age_model->populations[p]->nyears *
                                                           this->catch_at_age_model->populations[p]->nages);
      }

      for (fleet_iterator fit = this->catch_at_age_model->fleets.begin(); fit != this->catch_at_age_model->fleets.end();
           ++fit)
      {
        std::shared_ptr<fims_popdy::Fleet<double>> &fleet = (*fit).second;
        std::map<std::string, fims::Vector<double>> &derived_quantities =
            this->catch_at_age_model->GetFleetDerivedQuantities(fleet->GetId());

        // initialize derive quantities
        // landings
        derived_quantities["landings_numbers_at_age"] =
            fims::Vector<double>(fleet->nyears * fleet->nages);

        derived_quantities["landings_weight_at_age"] =
            fims::Vector<double>(fleet->nyears * fleet->nages);

        derived_quantities["landings_numbers_at_length"] =
            fims::Vector<double>(fleet->nyears * fleet->nlengths);

        derived_quantities["landings_weight"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["landings_numbers"] =
            fims::Vector<double>(fleet->nyears);

        derived_quantities["landings_expected"] =
            fims::Vector<double>(fleet->nyears);

        derived_quantities["log_landings_expected"] =
            fims::Vector<double>(fleet->nyears);

        derived_quantities["agecomp_proportion"] =
            fims::Vector<double>(fleet->nyears * fleet->nages);

        derived_quantities["lengthcomp_proportion"] =
            fims::Vector<double>(fleet->nyears * fleet->nlengths);

        // index
        derived_quantities["index_numbers_at_age"] =
            fims::Vector<double>(fleet->nyears * fleet->nages);

        derived_quantities["index_weight_at_age"] =
            fims::Vector<double>(fleet->nyears * fleet->nages);

        derived_quantities["index_numbers_at_length"] =
            fims::Vector<double>(fleet->nyears * fleet->nlengths);

        derived_quantities["index_weight"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["index_numbers"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["index_expected"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["log_index_expected"] =
            fims::Vector<double>(fleet->nyears);

        //
        derived_quantities["catch_index"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["expected_catch"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["expected_index"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["agecomp_expected"] =
            fims::Vector<double>(fleet->nyears * fleet->nages);

        derived_quantities["lengthcomp_expected"] =
            fims::Vector<double>(fleet->nyears * fleet->nlengths);

        if (fleet->nlengths > 0)
        {
          derived_quantities["age_to_length_conversion"] =
              fims::Vector<double>(fleet->nages * fleet->nlengths);
        }

        if (fleet->log_q.size() == 0)
        {
          fleet->log_q.resize(1);
          fleet->log_q[0] = static_cast<double>(0.0);
        }
        fleet->q.resize(fleet->log_q.size());
        fleet->Fmort.resize(fleet->nyears);
      }
    }

    void PrepareCAA()
    {
      catch_at_age_model->Prepare();
    }

    // Virtual void TearDown() will be called after each test is
    // run. It needs to be defined if there is clearup work to
    // do. Otherwise, it does not need to be provided.
    virtual void TearDown() {}
    std::shared_ptr<fims_popdy::Population<double>> population;
    std::shared_ptr<fims_popdy::CatchAtAge<double>> catch_at_age_model;

    // Use default values from the Li et al., 2021
    // https://github.com/NOAA-FIMS/Age_Structured_Stock_Assessment_Model_Comparison/blob/main/R/save_initial_input.R
    int id_g = 0;
    int nyears = 30;
    int nseasons = 1;
    int nages = 12;
    int nfleets = 2;
    int nlengths = 23;
  };

  class CAAEvaluateTestFixture : public testing::Test
  {
  protected:
    // Declare population here as a member, and initialize it in SetUp
    std::shared_ptr<fims_popdy::Population<double>> population;
    std::shared_ptr<fims_popdy::CatchAtAge<double>>
        catch_at_age_model; // New member for the model

    void SetUp() override
    {
      // C++ code to set up true values for log_naa, log_M,
      // log_Fmort, and log_q:
      int seed = 1234;
      std::default_random_engine generator(seed);

      // Initialize the population directly
      population = std::make_shared<fims_popdy::Population<double>>();
      population->id_g = id_g;
      population->nyears = nyears;
      population->nseasons = nseasons;
      population->nages = nages;
      population->nfleets = nfleets;

      // Initialize CatchAtAge model
      catch_at_age_model = std::make_shared<fims_popdy::CatchAtAge<double>>();

      // Setup fleet parameters needed for catch_at_age model->Prepare()
      // log_Fmort
      double log_Fmort_min = fims_math::log(0.1);
      double log_Fmort_max = fims_math::log(2.3);
      std::uniform_real_distribution<double> log_Fmort_distribution(
          log_Fmort_min, log_Fmort_max);

      // log_q
      double log_q_min = fims_math::log(0.1);
      double log_q_max = fims_math::log(1);
      std::uniform_real_distribution<double> log_q_distribution(log_q_min,
                                                                log_q_max);

      // Initialize fleet parameters needed for catch_at_age model->Prepare()
      for (int i = 0; i < nfleets; i++)
      {
        auto fleet = std::make_shared<fims_popdy::Fleet<double>>();
        fleet->nyears = nyears;
        fleet->nages = nages;
        fleet->nlengths = nlengths;
        fleet->log_q.resize(1);
        fleet->log_q.get_force_scalar(i) = log_q_distribution(generator);
        fleet->log_Fmort.resize(nyears);
        for (int year = 0; year < nyears; year++)
        {
          fleet->log_Fmort[year] = log_Fmort_distribution(generator);
        }
        auto selectivity =
            std::make_shared<fims_popdy::LogisticSelectivity<double>>();
        selectivity->inflection_point.resize(1);
        selectivity->inflection_point[0] = 7;
        selectivity->slope.resize(1);
        selectivity->slope[0] = 0.5;
        fleet->selectivity = selectivity;

        // Push fleet to population and catch_at_age_model
        population->fleets.push_back(fleet);
        catch_at_age_model->fleets[fleet->GetId()] =
            fleet; // Add to CatchAtAge model's fleets map
      }

      // Push population to catch_at_age_model
      catch_at_age_model->populations.push_back(population);
      // Initialize derived quantities
      this->InitializeCAA();

      // Setup population parameters needed for catch_at_age model->Prepare()
      catch_at_age_model->populations[0]->ages.resize(nages);
      catch_at_age_model->populations[0]->log_init_naa.resize(nages);
      catch_at_age_model->populations[0]->log_M.resize(nyears * nages);
      for (int i = 0; i < nages; i++)
      {
        catch_at_age_model->populations[0]->ages[i] = i + 1;
      }
      // weight_at_age
      double weight_at_age_min = 0.5;
      double weight_at_age_max = 12.0;
      std::shared_ptr<fims_popdy::EWAAgrowth<double>> growth =
          std::make_shared<fims_popdy::EWAAgrowth<double>>();
      std::uniform_real_distribution<double> weight_at_age_distribution(
          weight_at_age_min, weight_at_age_max);
      for (int i = 0; i < nages; i++)
      {
        growth->ewaa[static_cast<double>(population->ages[i])] =
            weight_at_age_distribution(generator);
      }

      catch_at_age_model->populations[0]->growth = growth;
      // log_M
      double log_M_min = fims_math::log(0.1);
      double log_M_max = fims_math::log(0.3);
      std::uniform_real_distribution<double> log_M_distribution(log_M_min,
                                                                log_M_max);
      for (int i = 0; i < nyears * nages; i++)
      {
        catch_at_age_model->populations[0]->log_M[i] =
            log_M_distribution(generator);
      }

      // Set initialized values for derived quantities
      catch_at_age_model->Prepare();

      // log_naa
      double log_init_naa_min = 10.0;
      double log_init_naa_max = 12.0;
      std::uniform_real_distribution<double> log_naa_distribution(
          log_init_naa_min, log_init_naa_max);
      for (int i = 0; i < nages; i++)
      {
        catch_at_age_model->populations[0]->log_init_naa[i] =
            log_naa_distribution(generator);
      }

      // prop_female
      double prop_female_min = 0.1;
      double prop_female_max = 0.9;
      std::uniform_real_distribution<double> prop_female_distribution(
          prop_female_min, prop_female_max);
      for (int i = 0; i < nages; i++)
      {
        catch_at_age_model->populations[0]->proportion_female[i] =
            prop_female_distribution(generator);
      }

      // numbers_at_age
      double numbers_at_age_min = fims_math::exp(10.0);
      double numbers_at_age_max = fims_math::exp(12.0);
      std::uniform_real_distribution<double> numbers_at_age_distribution(
          numbers_at_age_min, numbers_at_age_max);
      std::map<std::string, fims::Vector<double>> &pop_dq =
          catch_at_age_model->GetPopulationDerivedQuantities(0);
      for (int i = 0; i < (nyears + 1) * nages; i++)
      {
        pop_dq["numbers_at_age"][i] = numbers_at_age_distribution(generator);
      }

      auto maturity = std::make_shared<fims_popdy::LogisticMaturity<double>>();
      maturity->inflection_point.resize(1);
      maturity->inflection_point[0] = 6;
      maturity->slope.resize(1);
      maturity->slope[0] = 0.15;
      catch_at_age_model->populations[0]->maturity = maturity;

      auto recruitment = std::make_shared<fims_popdy::SRBevertonHolt<double>>();
      auto log_devs = std::make_shared<fims_popdy::LogDevs<double>>();
      recruitment->process = log_devs;
      recruitment->process->recruitment = recruitment;
      recruitment->logit_steep.resize(1);
      recruitment->log_rzero.resize(1);
      recruitment->logit_steep[0] = fims_math::logit(0.2, 1.0, 0.75);
      recruitment->log_rzero[0] = fims_math::log(1000000.0);
      /*the log_recruit_dev vector does not include a value for year == 0
      and is of length nyears - 1 where the first position of the vector
      corresponds to the second year of the time series.*/
      recruitment->log_recruit_devs.resize(nyears - 1);
      for (int i = 0; i < recruitment->log_recruit_devs.size(); i++)
      {
        recruitment->log_recruit_devs[i] = 0.0;
      }
      recruitment->log_expected_recruitment.resize(nyears + 1);
      for (int i = 0; i < recruitment->log_expected_recruitment.size(); i++)
      {
        recruitment->log_expected_recruitment[i] = 0.0;
      }
      catch_at_age_model->populations[0]->recruitment = recruitment;

      int year = 4;
      int age = 6;
      int i_age_year = year * population->nages + age;
      int i_agem1_yearm1 = (year - 1) * population->nages + age - 1;

      catch_at_age_model->CalculateMortality(population, i_age_year, year, age);
      catch_at_age_model->CalculateNumbersAA(population, i_age_year,
                                             i_agem1_yearm1, age);
    }

    /**
     * @brief Initialize CatchAtAge model derived quantities and population/fleet
     */
    void InitializeCAA()
    {
      typedef fims_popdy::CatchAtAge<double>::fleet_iterator fleet_iterator;

      //       // The following are initialized in the rcpp interface: ages, log_init_naa,
      //       //   numbers_at_age, log_Fmort, log_q
      for (size_t p = 0; p < this->catch_at_age_model->populations.size(); p++)
      {
        std::map<std::string, fims::Vector<double>> &derived_quantities =
            this->catch_at_age_model->GetPopulationDerivedQuantities(this->catch_at_age_model->populations[p]->GetId());

        derived_quantities["total_landings_weight"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears);

        derived_quantities["total_landings_numbers"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears);

        derived_quantities["mortality_F"] = fims::Vector<double>(
            this->catch_at_age_model->populations[p]->nyears * this->catch_at_age_model->populations[p]->nages);

        derived_quantities["mortality_Z"] = fims::Vector<double>(
            this->catch_at_age_model->populations[p]->nyears * this->catch_at_age_model->populations[p]->nages);

        // TODO: numbers_at_age are resized in rcpp_population, should this be
        // removed?
        derived_quantities["numbers_at_age"] = fims::Vector<double>(
            (this->catch_at_age_model->populations[p]->nyears + 1) * this->catch_at_age_model->populations[p]->nages);

        derived_quantities["unfished_numbers_at_age"] = fims::Vector<double>(
            (this->catch_at_age_model->populations[p]->nyears + 1) * this->catch_at_age_model->populations[p]->nages);

        derived_quantities["biomass"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears + 1);

        derived_quantities["spawning_biomass"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears + 1);

        derived_quantities["unfished_biomass"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears + 1);

        derived_quantities["unfished_spawning_biomass"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears + 1);

        derived_quantities["proportion_mature_at_age"] = fims::Vector<double>(
            (this->catch_at_age_model->populations[p]->nyears + 1) * this->catch_at_age_model->populations[p]->nages);

        derived_quantities["expected_recruitment"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears + 1);

        derived_quantities["sum_selectivity"] = fims::Vector<double>(
            this->catch_at_age_model->populations[p]->nyears * this->catch_at_age_model->populations[p]->nages);

        this->catch_at_age_model->populations[p]->proportion_female.resize(
            this->catch_at_age_model->populations[p]->nages);

        this->catch_at_age_model->populations[p]->M.resize(this->catch_at_age_model->populations[p]->nyears *
                                                           this->catch_at_age_model->populations[p]->nages);
      }

      for (fleet_iterator fit = this->catch_at_age_model->fleets.begin(); fit != this->catch_at_age_model->fleets.end();
           ++fit)
      {
        std::shared_ptr<fims_popdy::Fleet<double>> &fleet = (*fit).second;
        std::map<std::string, fims::Vector<double>> &derived_quantities =
            this->catch_at_age_model->GetFleetDerivedQuantities(fleet->GetId());

        // initialize derive quantities
        // landings
        derived_quantities["landings_numbers_at_age"] =
            fims::Vector<double>(fleet->nyears * fleet->nages);

        derived_quantities["landings_weight_at_age"] =
            fims::Vector<double>(fleet->nyears * fleet->nages);

        derived_quantities["landings_numbers_at_length"] =
            fims::Vector<double>(fleet->nyears * fleet->nlengths);

        derived_quantities["landings_weight"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["landings_numbers"] =
            fims::Vector<double>(fleet->nyears);

        derived_quantities["landings_expected"] =
            fims::Vector<double>(fleet->nyears);

        derived_quantities["log_landings_expected"] =
            fims::Vector<double>(fleet->nyears);

        derived_quantities["agecomp_proportion"] =
            fims::Vector<double>(fleet->nyears * fleet->nages);

        derived_quantities["lengthcomp_proportion"] =
            fims::Vector<double>(fleet->nyears * fleet->nlengths);

        // index
        derived_quantities["index_numbers_at_age"] =
            fims::Vector<double>(fleet->nyears * fleet->nages);

        derived_quantities["index_weight_at_age"] =
            fims::Vector<double>(fleet->nyears * fleet->nages);

        derived_quantities["index_numbers_at_length"] =
            fims::Vector<double>(fleet->nyears * fleet->nlengths);

        derived_quantities["index_weight"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["index_numbers"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["index_expected"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["log_index_expected"] =
            fims::Vector<double>(fleet->nyears);

        //
        derived_quantities["catch_index"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["expected_catch"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["expected_index"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["agecomp_expected"] =
            fims::Vector<double>(fleet->nyears * fleet->nages);

        derived_quantities["lengthcomp_expected"] =
            fims::Vector<double>(fleet->nyears * fleet->nlengths);

        if (fleet->nlengths > 0)
        {
          derived_quantities["age_to_length_conversion"] =
              fims::Vector<double>(fleet->nages * fleet->nlengths);
        }

        if (fleet->log_q.size() == 0)
        {
          fleet->log_q.resize(1);
          fleet->log_q[0] = static_cast<double>(0.0);
        }
        fleet->q.resize(fleet->log_q.size());
        fleet->Fmort.resize(fleet->nyears);
      }
    }

    void PrepareCAA()
    {
      catch_at_age_model->Prepare();
    }

    virtual void TearDown() {}

    int id_g = 0;
    int nyears = 30;
    int nseasons = 1;
    int nages = 12;
    int nfleets = 2;
    int nlengths = 23;

    int year = 4;
    int age = 6;
    int i_age_year = year * nages + age;
    int i_agem1_yearm1 = (year - 1) * nages + age - 1;
  };

  class CAAPrepareTestFixture : public testing::Test
  {
  protected:
    std::shared_ptr<fims_popdy::Population<double>> population;
    std::shared_ptr<fims_popdy::CatchAtAge<double>> catch_at_age_model;
    void SetUp() override
    {
      population = std::make_shared<fims_popdy::Population<double>>();
      population->id_g = id_g;
      population->nyears = nyears;
      population->nseasons = nseasons;
      population->nages = nages;
      population->nfleets = nfleets;

      // C++ code to set up true values for log_Fmort, and log_q:
      int seed = 1234;
      std::default_random_engine generator(seed);

      // Declare CatchAtAge model
      catch_at_age_model = std::make_shared<fims_popdy::CatchAtAge<double>>();

      // log_Fmort
      double log_Fmort_min = fims_math::log(0.1);
      double log_Fmort_max = fims_math::log(2.3);
      std::uniform_real_distribution<double> log_Fmort_distribution(
          log_Fmort_min, log_Fmort_max);

      // age_to_length_conversiondouble log_Fmort_min = fims_math::log(0.1);
      double age_2_length_conversion_min = 0.0;
      double age_2_length_conversion_max = 1.0;
      std::uniform_real_distribution<double> alc_distribution(
          age_2_length_conversion_min, age_2_length_conversion_max);

      // log_q
      double log_q_min = fims_math::log(0.1);
      double log_q_max = fims_math::log(1);
      std::uniform_real_distribution<double> log_q_distribution(log_q_min,
                                                                log_q_max);

      // Make a shared pointer to selectivity and fleet because
      // fleet object needs a shared pointer in fleet.hpp
      // (std::shared_ptr<fims::SelectivityBase<double> > selectivity;)
      // and population object needs a shared pointer in population.hpp
      // (std::vector<std::shared_ptr<fims::Fleet<double> > > fleets;)

      for (int i = 0; i < nfleets; i++)
      {
        auto fleet = std::make_shared<fims_popdy::Fleet<double>>();

        fleet->nlengths = nlengths;
        fleet->nages = nages;
        fleet->nyears = nyears;
        fleet->log_q.resize(1);
        fleet->log_q[0] = log_q_distribution(generator);
        fleet->log_Fmort.resize(nyears);
        for (int year = 0; year < nyears; year++)
        {
          fleet->log_Fmort[year] = log_Fmort_distribution(generator);
        }
        fleet->age_to_length_conversion.resize(nages * nlengths);
        for (int j = 0; j < nages * nlengths; j++)
        {
          fleet->age_to_length_conversion[j] = alc_distribution(generator);
        }
        auto selectivity =
            std::make_shared<fims_popdy::LogisticSelectivity<double>>();
        selectivity->inflection_point.resize(1);
        selectivity->inflection_point[0] = 7;
        selectivity->slope.resize(1);
        selectivity->slope[0] = 0.5;
        fleet->selectivity = selectivity;

        population->fleets.push_back(fleet);
        catch_at_age_model->fleets[fleet->GetId()] =
            fleet; // Add to CatchAtAge model's fleets map
      }

      population->ages.resize(nages);
      for (int i = 0; i < nages; i++)
      {
        population->ages[i] = i + 1;
      }

      catch_at_age_model->populations.push_back(population);
      // Initialize derived quantities
      this->InitializeCAA();

      // log_M
      double log_M_min = fims_math::log(0.1);
      double log_M_max = fims_math::log(0.3);
      std::uniform_real_distribution<double> log_M_distribution(log_M_min,
                                                                log_M_max);
      catch_at_age_model->populations[0]->log_M.resize(nyears * nages);
      for (int i = 0; i < nyears * nages; i++)
      {
        catch_at_age_model->populations[0]->log_M[i] =
            log_M_distribution(generator);
      }

      // weight_at_age
      double weight_at_age_min = 0.5;
      double weight_at_age_max = 12.0;

      std::shared_ptr<fims_popdy::EWAAgrowth<double>> growth =
          std::make_shared<fims_popdy::EWAAgrowth<double>>();
      std::uniform_real_distribution<double> weight_at_age_distribution(
          weight_at_age_min, weight_at_age_max);
      for (int i = 0; i < nages; i++)
      {
        growth->ewaa[static_cast<double>(
            catch_at_age_model->populations[0]->ages[i])] =
            weight_at_age_distribution(generator);
      }

      catch_at_age_model->populations[0]->growth = growth;
    }

    /**
     * @brief Initialize CatchAtAge model derived quantities and population/fleet
     */
    void InitializeCAA()
    {
      typedef fims_popdy::CatchAtAge<double>::fleet_iterator fleet_iterator;

      //       // The following are initialized in the rcpp interface: ages, log_init_naa,
      //       //   numbers_at_age, log_Fmort, log_q
      for (size_t p = 0; p < this->catch_at_age_model->populations.size(); p++)
      {
        std::map<std::string, fims::Vector<double>> &derived_quantities =
            this->catch_at_age_model->GetPopulationDerivedQuantities(this->catch_at_age_model->populations[p]->GetId());

     
        derived_quantities["total_landings_weight"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears);

        derived_quantities["total_landings_numbers"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears);

        derived_quantities["mortality_F"] = fims::Vector<double>(
            this->catch_at_age_model->populations[p]->nyears * this->catch_at_age_model->populations[p]->nages);

        derived_quantities["mortality_Z"] = fims::Vector<double>(
            this->catch_at_age_model->populations[p]->nyears * this->catch_at_age_model->populations[p]->nages);

        // TODO: numbers_at_age are resized in rcpp_population, should this be
        // removed?
        derived_quantities["numbers_at_age"] = fims::Vector<double>(
            (this->catch_at_age_model->populations[p]->nyears + 1) * this->catch_at_age_model->populations[p]->nages);

        derived_quantities["unfished_numbers_at_age"] = fims::Vector<double>(
            (this->catch_at_age_model->populations[p]->nyears + 1) * this->catch_at_age_model->populations[p]->nages);

        derived_quantities["biomass"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears + 1);

        derived_quantities["spawning_biomass"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears + 1);

        derived_quantities["unfished_biomass"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears + 1);

        derived_quantities["unfished_spawning_biomass"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears + 1);

        derived_quantities["proportion_mature_at_age"] = fims::Vector<double>(
            (this->catch_at_age_model->populations[p]->nyears + 1) * this->catch_at_age_model->populations[p]->nages);

        derived_quantities["expected_recruitment"] =
            fims::Vector<double>(this->catch_at_age_model->populations[p]->nyears + 1);

        derived_quantities["sum_selectivity"] = fims::Vector<double>(
            this->catch_at_age_model->populations[p]->nyears * this->catch_at_age_model->populations[p]->nages);

        this->catch_at_age_model->populations[p]->proportion_female.resize(
            this->catch_at_age_model->populations[p]->nages);

        this->catch_at_age_model->populations[p]->M.resize(this->catch_at_age_model->populations[p]->nyears *
                                                           this->catch_at_age_model->populations[p]->nages);
      }

      for (fleet_iterator fit = this->catch_at_age_model->fleets.begin(); fit != this->catch_at_age_model->fleets.end();
           ++fit)
      {
        std::shared_ptr<fims_popdy::Fleet<double>> &fleet = (*fit).second;
        std::map<std::string, fims::Vector<double>> &derived_quantities =
            this->catch_at_age_model->GetFleetDerivedQuantities(fleet->GetId());

        // initialize derive quantities
        // landings
        derived_quantities["landings_numbers_at_age"] =
            fims::Vector<double>(fleet->nyears * fleet->nages);

        derived_quantities["landings_weight_at_age"] =
            fims::Vector<double>(fleet->nyears * fleet->nages);

        derived_quantities["landings_numbers_at_length"] =
            fims::Vector<double>(fleet->nyears * fleet->nlengths);

        derived_quantities["landings_weight"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["landings_numbers"] =
            fims::Vector<double>(fleet->nyears);

        derived_quantities["landings_expected"] =
            fims::Vector<double>(fleet->nyears);

        derived_quantities["log_landings_expected"] =
            fims::Vector<double>(fleet->nyears);

        derived_quantities["agecomp_proportion"] =
            fims::Vector<double>(fleet->nyears * fleet->nages);

        derived_quantities["lengthcomp_proportion"] =
            fims::Vector<double>(fleet->nyears * fleet->nlengths);

        // index
        derived_quantities["index_numbers_at_age"] =
            fims::Vector<double>(fleet->nyears * fleet->nages);

        derived_quantities["index_weight_at_age"] =
            fims::Vector<double>(fleet->nyears * fleet->nages);

        derived_quantities["index_numbers_at_length"] =
            fims::Vector<double>(fleet->nyears * fleet->nlengths);

        derived_quantities["index_weight"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["index_numbers"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["index_expected"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["log_index_expected"] =
            fims::Vector<double>(fleet->nyears);

        //
        derived_quantities["catch_index"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["expected_catch"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["expected_index"] = fims::Vector<double>(fleet->nyears);

        derived_quantities["agecomp_expected"] =
            fims::Vector<double>(fleet->nyears * fleet->nages);

        derived_quantities["lengthcomp_expected"] =
            fims::Vector<double>(fleet->nyears * fleet->nlengths);

        if (fleet->nlengths > 0)
        {
          derived_quantities["age_to_length_conversion"] =
              fims::Vector<double>(fleet->nages * fleet->nlengths);
        }

        if (fleet->log_q.size() == 0)
        {
          fleet->log_q.resize(1);
          fleet->log_q[0] = static_cast<double>(0.0);
        }
        fleet->q.resize(fleet->log_q.size());
        fleet->Fmort.resize(fleet->nyears);
      }
    }

    void PrepareCAA()
    {
      catch_at_age_model->Prepare();
    }

    virtual void TearDown() {}

    fims_popdy::Population<double> pop;
    int id_g = 0;
    int nyears = 30;
    int nseasons = 1;
    int nages = 12;
    int nfleets = 2;
    int nlengths = 23;
  };
} // namespace

#endif