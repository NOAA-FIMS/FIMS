#include <random>

#include "population_dynamics/population/population.hpp"




namespace {

// Use test fixture to reuse the same configuration of objects for
// several different tests. To use a test fixture, derive a class
// from testing::Test.
class PopulationInitializeTestFixture : public testing::Test {
  // Make members protected and they can be accessed from
  // sub-classes.
 protected:
  // Use SetUp function to prepare the objects for each test.
  // Use override in C++11 to make sure SetUp (e.g., not Setup with
  // a lowercase u) is spelled
  // correctly.
  void SetUp() override {
    population.id_g = id_g;
    population.nyears = nyears;
    population.nseasons = nseasons;
    population.nages = nages;
    for (int i = 0; i < nfleets; i++) {
      auto fleet = std::make_shared<fims_popdy::Fleet<double>>();
      fleet->log_q.resize(1);
      population.fleets.push_back(fleet);
    }
  }

  // Virtual void TearDown() will be called after each test is
  // run. It needs to be defined if there is clearup work to
  // do. Otherwise, it does not need to be provided.
  virtual void TearDown() {}

  fims_popdy::Population<double> population;

  // Use default values from the Li et al., 2021
  // https://github.com/Bai-Li-NOAA/Age_Structured_Stock_Assessment_Model_Comparison/blob/master/R/save_initial_input.R
  int id_g = 0;
  int nyears = 30;
  int nseasons = 1;
  int nages = 12;
  int nfleets = 2;
};

class PopulationEvaluateTestFixture : public testing::Test {
 protected:
  void SetUp() override {
    population.id_g = id_g;
    population.nyears = nyears;
    population.nseasons = nseasons;
    population.nages = nages;
    population.nfleets = nfleets;

    // C++ code to set up true values for log_naa, log_M,
    // log_Fmort, and log_q:
    int seed = 1234;
    std::default_random_engine generator(seed);

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

    // Make a shared pointer to selectivity and fleet because
    // fleet object needs a shared pointer in fleet.hpp
    // (std::shared_ptr<fims_popdy::SelectivityBase<Type> > selectivity;)
    // and population object needs a shared pointer in population.hpp
    // (std::vector<std::shared_ptr<fims_popdy::Fleet<Type> > > fleets;)

    // Does Fmort need to be in side of the year loop like log_q?
    for (int i = 0; i < nfleets; i++) {
      auto fleet = std::make_shared<fims_popdy::Fleet<double>>();
      auto selectivity =
          std::make_shared<fims_popdy::LogisticSelectivity<double>>();
      selectivity->inflection_point.resize(1);
      selectivity->inflection_point[0] = 7;
      selectivity->slope.resize(1);
      selectivity->slope[0] = 0.5;

      fleet->expected_catch.resize(nyears);
      fleet->expected_index.resize(nyears);  
      fleet->catch_numbers_at_age.resize(nyears * nages);
      fleet->log_q.resize(1);
      fleet->Initialize(nyears, nages);
      fleet->selectivity = selectivity;
      fleet->log_q[0] = log_q_distribution(generator);
      for (int year = 0; year < nyears; year++) {
        fleet->log_Fmort[year] = log_Fmort_distribution(generator);
      }
      if (i == 0) {
        fleet->is_survey = true;
      }
      fleet->Prepare();
      population.fleets.push_back(fleet);
    }
    population.numbers_at_age.resize((nyears + 1) * nages);
    try {
        population.Initialize(nyears, nseasons, nages);
    } catch (std::exception& e) {
        std::cout << e.what() << "\n";
    }

    for (int i = 0; i < nages; i++) {
      population.ages[i] = i + 1;
    }

    // log_naa
    double log_init_naa_min = 10.0;
    double log_init_naa_max = 12.0;
    std::uniform_real_distribution<double> log_naa_distribution(
        log_init_naa_min, log_init_naa_max);
    for (int i = 0; i < nages; i++) {
      population.log_init_naa[i] = log_naa_distribution(generator);
    }

    // prop_female
    double prop_female_min = 0.1;
    double prop_female_max = 0.9;
    std::uniform_real_distribution<double> prop_female_distribution(
        prop_female_min, prop_female_max);
    for (int i = 0; i < nages; i++) {
      population.proportion_female[i] = prop_female_distribution(generator);
    }

    // log_M
    double log_M_min = fims_math::log(0.1);
    double log_M_max = fims_math::log(0.3);
    std::uniform_real_distribution<double> log_M_distribution(log_M_min,
                                                              log_M_max);
    for (int i = 0; i < nyears * nages; i++) {
      population.log_M[i] = log_M_distribution(generator);
    }

    // numbers_at_age
    double numbers_at_age_min = fims_math::exp(10.0);
    double numbers_at_age_max = fims_math::exp(12.0);
    std::uniform_real_distribution<double> numbers_at_age_distribution(
        numbers_at_age_min, numbers_at_age_max);
    for (int i = 0; i < (nyears + 1) * nages; i++) {
      population.numbers_at_age[i] = numbers_at_age_distribution(generator);
    }

    // weight_at_age
    double weight_at_age_min = 0.5;
    double weight_at_age_max = 12.0;

    std::shared_ptr<fims_popdy::EWAAgrowth<double>> growth =
        std::make_shared<fims_popdy::EWAAgrowth<double>>();
    std::uniform_real_distribution<double> weight_at_age_distribution(
        weight_at_age_min, weight_at_age_max);
    for (int i = 0; i < nages; i++) {
      growth->ewaa[static_cast<double>(population.ages[i])] =
          weight_at_age_distribution(generator);
    }

    population.growth = growth;
    population.Prepare();

    auto maturity = std::make_shared<fims_popdy::LogisticMaturity<double>>();
    maturity->inflection_point.resize(1);
    maturity->inflection_point[0] = 6;
    maturity->slope.resize(1);
    maturity->slope[0] = 0.15;
    population.maturity = maturity;

    auto recruitment = std::make_shared<fims_popdy::SRBevertonHolt<double>>();
    recruitment->logit_steep.resize(1);
    recruitment->log_rzero.resize(1);
    recruitment->logit_steep[0] = fims_math::logit(0.2, 1.0, 0.75);
    recruitment->log_rzero[0] = fims_math::log(1000000.0);
    /*the log_recruit_dev vector does not include a value for year == 0
    and is of length nyears - 1 where the first position of the vector
    corresponds to the second year of the time series.*/
    recruitment->log_recruit_devs.resize(nyears - 1);
    for (int i = 0; i < recruitment->log_recruit_devs.size(); i++) {
      recruitment->log_recruit_devs[i] = 0.0;
    }
    population.recruitment = recruitment;

    int year = 4;
    int age = 6;
    int i_age_year = year * population.nages + age;
    int i_agem1_yearm1 = (year - 1) * population.nages + age - 1;

    population.CalculateMortality(i_age_year, year, age);
    population.CalculateNumbersAA(i_age_year, i_agem1_yearm1, age);
  }

  virtual void TearDown() {}

  fims_popdy::Population<double> population;
  int id_g = 0;
  int nyears = 30;
  int nseasons = 1;
  int nages = 12;
  int nfleets = 2;

  int year = 4;
  int age = 6;
  int i_age_year = year * nages + age;
  int i_agem1_yearm1 = (year - 1) * nages + age - 1;
};

class PopulationPrepareTestFixture : public testing::Test {
 protected:
  void SetUp() override {
    population.id_g = id_g;
    population.nyears = nyears;
    population.nseasons = nseasons;
    population.nages = nages;
    population.nfleets = nfleets;

    // C++ code to set up true values for log_Fmort, and log_q:
    int seed = 1234;
    std::default_random_engine generator(seed);

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

    // Make a shared pointer to selectivity and fleet because
    // fleet object needs a shared pointer in fleet.hpp
    // (std::shared_ptr<fims::SelectivityBase<Type> > selectivity;)
    // and population object needs a shared pointer in population.hpp
    // (std::vector<std::shared_ptr<fims::Fleet<Type> > > fleets;)

    for (int i = 0; i < nfleets; i++) {
      auto fleet = std::make_shared<fims_popdy::Fleet<double>>();
      auto selectivity =
          std::make_shared<fims_popdy::LogisticSelectivity<double>>();
      selectivity->inflection_point.resize(1);
      selectivity->slope.resize(1);
      selectivity->inflection_point[0] = 7;
      selectivity->slope[0] = 0.5;
      
      fleet->expected_catch.resize(nyears);
      fleet->expected_index.resize(nyears);  
      fleet->catch_numbers_at_age.resize(nyears * nages);
      fleet->log_q.resize(1);
      fleet->Initialize(nyears, nages);
      fleet->selectivity = selectivity;
      fleet->log_q[0] = log_q_distribution(generator);
      for (int year = 0; year < nyears; year++) {
        fleet->log_Fmort[year] = log_Fmort_distribution(generator);
      }
      if (i == 0) {
        fleet->is_survey = true;
      }
      fleet->Prepare();
      population.fleets.push_back(fleet);
    }

    population.numbers_at_age.resize((nyears + 1) * nages);
    population.Initialize(nyears, nseasons, nages);

    for (int i = 0; i < nages; i++) {
      population.ages[i] = i + 1;
    }

    // log_naa
    double log_init_naa_min = 10.0;
    double log_init_naa_max = 12.0;
    std::uniform_real_distribution<double> log_naa_distribution(
        log_init_naa_min, log_init_naa_max);
    for (int i = 0; i < nages; i++) {
      population.log_init_naa[i] = log_naa_distribution(generator);
    }

    // prop_female
    double prop_female_min = 0.1;
    double prop_female_max = 0.9;
    std::uniform_real_distribution<double> prop_female_distribution(
        prop_female_min, prop_female_max);
    for (int i = 0; i < nages; i++) {
      population.proportion_female[i] = prop_female_distribution(generator);
    }

    // log_M
    double log_M_min = fims_math::log(0.1);
    double log_M_max = fims_math::log(0.3);
    std::uniform_real_distribution<double> log_M_distribution(log_M_min,
                                                              log_M_max);
    for (int i = 0; i < nyears * nages; i++) {
      population.log_M[i] = log_M_distribution(generator);
    }

    // numbers_at_age
    double numbers_at_age_min = fims_math::exp(10.0);
    double numbers_at_age_max = fims_math::exp(12.0);
    std::uniform_real_distribution<double> numbers_at_age_distribution(
        numbers_at_age_min, numbers_at_age_max);
    for (int i = 0; i < (nyears + 1) * nages; i++) {
      population.numbers_at_age[i] = numbers_at_age_distribution(generator);
    }

    // weight_at_age
    double weight_at_age_min = 0.5;
    double weight_at_age_max = 12.0;

    std::shared_ptr<fims_popdy::EWAAgrowth<double>> growth =
        std::make_shared<fims_popdy::EWAAgrowth<double>>();
    std::uniform_real_distribution<double> weight_at_age_distribution(
        weight_at_age_min, weight_at_age_max);
    for (int i = 0; i < nages; i++) {
      growth->ewaa[static_cast<double>(population.ages[i])] =
          weight_at_age_distribution(generator);
    }

    population.growth = growth;

    population.Prepare();
  }

  virtual void TearDown() {}

  fims_popdy::Population<double> population;
  int id_g = 0;
  int nyears = 30;
  int nseasons = 1;
  int nages = 12;
  int nfleets = 2;
};
}  // namespace