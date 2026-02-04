#ifndef TEST_SURPLUS_PRODUCTION_TEST_FIXTURE_HPP
#define TEST_SURPLUS_PRODUCTION_TEST_FIXTURE_HPP
#include <random>

#include "../../inst/include/models/functors/surplus_production.hpp"
#include "population_dynamics/population/population.hpp"


namespace {

// Use test fixture to reuse the same configuration of objects for
// several different tests. To use a test fixture, derive a class
// from testing::Test.
class SPInitializeTestFixture : public testing::Test {
     // Make members protected and they can be accessed from
  // sub-classes.
 protected:
  // Use SetUp function to prepare the objects for each test.
  // Use override in C++11 to make sure SetUp (e.g., not Setup with
  // a lowercase u) is spelled
  // correctly.
  void SetUp() override {
    population = std::make_shared<fims_popdy::Population<double>>();
    surplus_production_model = std::make_shared<fims_popdy::SurplusProduction<double>>();
    population->id_g = id_g;
    population->nyears = nyears;
    population->nfleets = nfleets;

    for (int i = 0; i < nfleets; i++) {
      auto fleet = std::make_shared<fims_popdy::Fleet<double>>();
      fleet->nyears = nyears;
      fleet->log_q.resize(1);
      population->fleets.push_back(fleet);
      surplus_production_model->fleets[fleet->GetId()] =
          fleet;  // Add to SurplusProduction model's fleets map
    }
    surplus_production_model->populations.push_back(population);
  }

  // Virtual void TearDown() will be called after each test is
  // run. It needs to be defined if there is clearup work to
  // do. Otherwise, it does not need to be provided.
  virtual void TearDown() {}
  std::shared_ptr<fims_popdy::Population<double>> population;
  std::shared_ptr<fims_popdy::SurplusProduction<double>> surplus_production_model;

  // Use default values from the JABBA example in R testthat example
  // https://github.com/NOAA-FIMS/FIMS/data-raw/data_sp.R
  int id_g = 0;
  int nyears = 70;
  int nfleets = 2;
};


class SPPrepareTestFixture : public testing::Test {
 protected:
  std::shared_ptr<fims_popdy::Population<double>> population;
  std::shared_ptr<fims_popdy::SurplusProduction<double>> surplus_production_model;
  void SetUp() override {
    population = std::make_shared<fims_popdy::Population<double>>();
    population->id_g = id_g;
    population->nyears = nyears;
    population->nfleets = nfleets;

    // C++ code to set up true values for log_q:
    int seed = 1234;
    std::default_random_engine generator(seed);

    // Declare SurplusProduction model
    surplus_production_model = std::make_shared<fims_popdy::SurplusProduction<double>>();

    // log_q
    double log_q_min = fims_math::log(0.1);
    double log_q_max = fims_math::log(1);
    std::uniform_real_distribution<double> log_q_distribution(log_q_min,
                                                              log_q_max);

    // Make a shared pointer to fleet because
    // fleet object needs a shared pointer in fleet.hpp
    // and population object needs a shared pointer in population.hpp
    // (std::vector<std::shared_ptr<fims::Fleet<double> > > fleets;)

    for (int i = 0; i < nfleets; i++) {
      auto fleet = std::make_shared<fims_popdy::Fleet<double>>();

      fleet->nyears = nyears;
      fleet->log_q.resize(1);
      fleet->log_q[0] = log_q_distribution(generator);

      population->fleets.push_back(fleet);
      surplus_production_model->fleets[fleet->GetId()] =
          fleet;  // Add to SurplusProduction model's fleets map
    }

    surplus_production_model->populations.push_back(population);
    // Initialize derived quantities
    surplus_production_model->Initialize();

  }

  virtual void TearDown() {}

  fims_popdy::Population<double> pop;
  int id_g = 0;
  int nyears = 70;
  int nfleets = 2;
};

class SPEvaluateTestFixture : public testing::Test {
 protected:
  // Declare population here as a member, and initialize it in SetUp
  std::shared_ptr<fims_popdy::Population<double>> population;
  std::shared_ptr<fims_popdy::SurplusProduction<double>>
      surplus_production_model;  // New member for the model

  void SetUp() override {
    // C++ code to set up true values for log_naa, log_M,
    // log_Fmort, and log_q:
    int seed = 1234;
    std::default_random_engine generator(seed);

    // Initialize the population directly
    population = std::make_shared<fims_popdy::Population<double>>();
    population->id_g = id_g;
    population->nyears = nyears;
    population->nfleets = nfleets;

    // Initialize SurplusProduction model
    surplus_production_model = std::make_shared<fims_popdy::SurplusProduction<double>>();

    // Setup fleet parameters needed for surplus_production model->Prepare()
    // log_q
    double log_q_min = fims_math::log(0.1);
    double log_q_max = fims_math::log(1);
    std::uniform_real_distribution<double> log_q_distribution(log_q_min,
                                                              log_q_max);
    double catch_min = 9.0;
    double catch_max = 30.0;
    std::uniform_real_distribution<double> catch_distribution(catch_min,
                                                              catch_max);

    // Initialize fleet parameters needed for surplus_production model->Prepare()
    for (int i = 0; i < nfleets; i++) {
      auto fleet = std::make_shared<fims_popdy::Fleet<double>>();
      auto catch_data = std::make_shared<fims_data_object::DataObject<double>>(nyears);
      for(int year=0; year<nyears; year++){
          catch_data->at(year) = catch_distribution(generator);
      }
      fleet->observed_landings_data = catch_data;
      fleet->fleet_observed_landings_data_id_m = catch_data->id_g;
      fleet->nyears = nyears;
      fleet->log_q.resize(1);
      fleet->log_q.get_force_scalar(i) = log_q_distribution(generator);

      // Push fleet to population and surplus_production_model
      population->fleets.push_back(fleet);
      surplus_production_model->fleets[fleet->GetId()] =
          fleet;  // Add to SurplusProduction model's fleets map
    }

    // Push population to surplus_production_model
    surplus_production_model->populations.push_back(population);
    // Initialize derived quantities
    surplus_production_model->Initialize();

    // Set initialized values for derived quantities
    surplus_production_model->Prepare();

    // init_depletion
    double depletion_min = 0.01;
    double depletion_max = 0.99;
    std::uniform_real_distribution<double> depletion_distribution(
        depletion_min, depletion_max);
    double init_depletion = depletion_distribution(generator);

    //depletion
    auto depletion = std::make_shared<fims_popdy::PellaTomlinsonDepletion<double>>();
    depletion->log_r.resize(1);
    depletion->log_K.resize(1);
    depletion->log_m.resize(1);
    depletion->log_r[0] = fims_math::log(0.2);
    depletion->log_K[0] = fims_math::log(645.0);
    depletion->log_m[0] = fims_math::log(2.0);
    depletion->log_depletion.resize(nyears-1);
    for(int i=0; i<depletion->log_depletion.size(); i++){
      double input_depletion = depletion_distribution(generator);
      //log transformation
      depletion->log_depletion[i] = log(input_depletion);
    }
    //initialize depletion for first year at 0.5
    depletion->depletion.resize(nyears);
    depletion->log_expected_depletion.resize(nyears);
    depletion->log_init_depletion.resize(1);
    depletion->log_init_depletion[0] =
          log(init_depletion); //log transformation
    surplus_production_model->populations[0]->depletion = depletion;

    int year = 4;
  }

  virtual void TearDown() {}

  int id_g = 0;
  int nyears = 70;
  int nseasons = 1;
  int nfleets = 2;

  int year = 4;
};


}  // namespace
#endif // TEST_SURPLUS_PRODUCTION_TEST_FIXTURE_HPP