#include "models/functors/catch_at_age.hpp"
#include "population_dynamics/age_to_length_conversion/functors/fixed.hpp"
#include "population_dynamics/growth/functors/ewaa.hpp"
#include "population_dynamics/recruitment/functors/log_devs.hpp"
#include "population_dynamics/recruitment/functors/sr_beverton_holt.hpp"
#include "test_stubs.hpp"
#include "gtest/gtest.h"

namespace {
struct CountingMean : fims_popdy::SRBevertonHolt<double> {
  int calls = 0;
  const double evaluate_mean(const double &sb, const double &phi) override {
    ++calls;
    return fims_popdy::SRBevertonHolt<double>::evaluate_mean(sb, phi);
  }
};
struct CountingProcess : fims_popdy::LogDevs<double> {
  int calls = 0;
  const double evaluate_process(size_t year) override {
    ++calls;
    return fims_popdy::LogDevs<double>::evaluate_process(year);
  }
};

class LinkedRecruitmentTest : public testing::Test {
protected:
  fims_popdy::CatchAtAge<double> model;
  std::shared_ptr<CountingMean> mean = std::make_shared<CountingMean>();
  std::shared_ptr<CountingProcess> process =
      std::make_shared<CountingProcess>();
  std::shared_ptr<fims_popdy::Population<double>> female, male;

  auto MakePopulation(double fraction, double mortality, double maturity) {
    auto pop = std::make_shared<fims_popdy::Population<double>>();
    pop->n_years = 3;
    pop->n_ages = 3;
    pop->n_fleets = 0;
    pop->years = {2000, 2001, 2002};
    pop->ages = {1, 2, 3};
    pop->log_M = fims::Vector<double>(9, std::log(mortality));
    pop->log_f_multiplier = fims::Vector<double>(3, 0.0);
    pop->log_init_naa = {std::log(fraction * 1000),
                         std::log(fraction * 1000) - mortality,
                         std::log(fraction * 1000) - 2 * mortality -
                             std::log(1 - std::exp(-mortality))};
    pop->recruitment = mean;
    auto maturity_module =
        std::make_shared<fims_popdy::ConstantMaturity<double>>();
    maturity_module->proportion = maturity;
    pop->maturity = maturity_module;
    auto growth = std::make_shared<fims_popdy::EWAAGrowth<double>>();
    for (int y = 0; y <= 3; ++y)
      for (double age : {1.0, 2.0, 3.0})
        growth->ewaa[y][age] = 2.0;
    pop->growth = growth;
    model.InitializePopulationDerivedQuantities(pop->GetId());
    auto &dq = model.GetPopulationDerivedQuantities(pop->GetId());
    for (const auto &key : {"numbers_at_age", "unfished_numbers_at_age",
                            "proportion_mature_at_age"})
      dq[key] = fims::Vector<double>(12, 0.0);
    for (const auto &key :
         {"mortality_F", "mortality_M", "mortality_Z", "sum_selectivity"})
      dq[key] = fims::Vector<double>(9, 0.0);
    for (const auto &key :
         {"biomass", "unfished_biomass", "spawning_biomass",
          "unfished_spawning_biomass", "expected_recruitment"})
      dq[key] = fims::Vector<double>(4, 0.0);
    dq["total_catch_numbers"] = fims::Vector<double>(3, 0.0);
    dq["total_catch_weight"] = fims::Vector<double>(3, 0.0);
    model.populations.push_back(pop);
    return pop;
  }
  void SetUp() override {
    mean->log_rzero = {std::log(1000.0)};
    mean->logit_steep = {fims_math::logit(0.2, 1.0, 0.75)};
    mean->log_expected_recruitment = fims::Vector<double>(2, 0.0);
    mean->log_recruit_devs = {0.0, 0.0};
    mean->process = process;
    process->recruitment = mean;
    female = MakePopulation(0.4, 0.2, 1.0);
    male = MakePopulation(0.6, 0.3, 0.0);
    model.recruitment_links = {
        {{{female->GetId(), 0.4}, {male->GetId(), 0.6}}}};
    model.Initialize();
  }
  auto MakeFleet(const std::shared_ptr<fims_popdy::Population<double>> &pop,
                 double fishing, double catchability, bool male_mapping) {
    auto fleet = std::make_shared<fims_popdy::Fleet<double>>();
    fleet->n_years = 3;
    fleet->n_ages = 3;
    fleet->n_lengths = 2;
    fleet->lengths = {10, 20};
    fleet->length_bin_edges = {5, 15, 25};
    fleet->log_Fmort = fims::Vector<double>(3, std::log(fishing));
    fleet->log_q = {std::log(catchability)};
    fleet->observed_catch_units = "weight";
    fleet->observed_index_units = "number";
    if (pop) {
      auto selectivity =
          std::make_shared<fims_popdy::LogisticSelectivity<double>>();
      selectivity->inflection_point = {male_mapping ? 2.0 : 1.0};
      selectivity->slope = {1.0};
      fleet->selectivity = selectivity;
      fleet->requires_age_length_mapping = true;
      fleet->age_to_length_conversion =
          male_mapping ? fims::Vector<double>{0, 1, 0, 1, 0, 1}
                       : fims::Vector<double>{1, 0, 0.5, 0.5, 0, 1};
      fleet->age_to_length_conversion_model =
          std::make_shared<fims_popdy::AgeToLengthConversionFixed<double>>(
              fleet);
      pop->fleets.push_back(fleet);
      pop->n_fleets = pop->fleets.size();
    }
    model.fleets[fleet->GetId()] = fleet;
    model.InitializeFleetDerivedQuantities(fleet->GetId());
    auto &dq = model.GetFleetDerivedQuantities(fleet->GetId());
    for (const auto &key :
         {"catch_numbers_at_age", "catch_weight_at_age", "index_numbers_at_age",
          "index_weight_at_age", "agecomp_expected", "agecomp_proportion"})
      dq[key] = fims::Vector<double>(9, 0.0);
    for (const auto &key :
         {"catch_numbers_at_length", "index_numbers_at_length",
          "lengthcomp_expected", "lengthcomp_proportion"})
      dq[key] = fims::Vector<double>(6, 0.0);
    for (const auto &key : {"catch_numbers", "catch_weight", "index_numbers",
                            "index_weight", "catch_expected", "index_expected",
                            "log_catch_expected", "log_index_expected"})
      dq[key] = fims::Vector<double>(3, 0.0);
    return fleet;
  }
  double Recruitment(const std::shared_ptr<fims_popdy::Population<double>> &pop,
                     size_t year) {
    return model.GetPopulationDerivedQuantities(
        pop->GetId())["numbers_at_age"][year * pop->n_ages];
  }
};

// Description: one stock process conserves recruits, including terminal-year
// mean recruitment; unfished reference points use total-stock recruits.
TEST_F(LinkedRecruitmentTest, ConservesRecruitmentAndUnfishedNormalization) {
  model.Evaluate();
  EXPECT_EQ(mean->calls, 3);
  EXPECT_EQ(process->calls, 2);
  auto &fdq = model.GetPopulationDerivedQuantities(female->GetId());
  auto &mdq = model.GetPopulationDerivedQuantities(male->GetId());
  for (size_t y = 0; y <= 3; ++y) {
    EXPECT_NEAR(Recruitment(female, y), 400.0, 1e-8);
    EXPECT_NEAR(Recruitment(male, y), 600.0, 1e-8);
    EXPECT_DOUBLE_EQ(mdq["spawning_biomass"][y], 0.0);
    EXPECT_NEAR(female->spawning_biomass_ratio[y], 1.0, 1e-10);
    EXPECT_NEAR(male->spawning_biomass_ratio[y], 1.0, 1e-10);
    EXPECT_NEAR(fdq["unfished_numbers_at_age"][y * 3] +
                    mdq["unfished_numbers_at_age"][y * 3],
                1000.0, 1e-8);
  }
  EXPECT_NEAR(fdq["unfished_spawning_biomass"][0],
              1000.0 * 0.4 * model.CalculateSBPR0(female), 1e-8);
}

// Description: linked results do not depend on population ordering or stale
// state.
TEST_F(LinkedRecruitmentTest, IsOrderIndependentAndRepeatable) {
  model.Evaluate();
  auto expected =
      model.GetPopulationDerivedQuantities(female->GetId())["numbers_at_age"];
  std::reverse(model.populations.begin(), model.populations.end());
  model.Evaluate();
  EXPECT_EQ(mean->calls, 6);
  EXPECT_EQ(process->calls, 4);
  EXPECT_EQ(expected, model.GetPopulationDerivedQuantities(
                          female->GetId())["numbers_at_age"]);
}

// Description: female spawning output drives both sexes, with one shared
// deviation.
TEST_F(LinkedRecruitmentTest, SharesFeedbackAndRecruitmentDeviations) {
  for (auto &value : female->log_init_naa)
    value -= std::log(2.0);
  for (auto &value : male->log_init_naa)
    value += std::log(10.0);
  model.Evaluate();
  double total = Recruitment(female, 1) + Recruitment(male, 1);
  EXPECT_LT(total, 1000.0);
  EXPECT_NEAR(Recruitment(female, 1) / total, 0.4, 1e-12);
  mean->log_recruit_devs[0] = std::log(1.2);
  model.Evaluate();
  EXPECT_NEAR(Recruitment(female, 1) + Recruitment(male, 1), 1.2 * total, 1e-8);
}

// Description: malformed links fail before recruitment can be counted twice.
TEST_F(LinkedRecruitmentTest, RejectsInvalidConfiguration) {
  auto valid = model.recruitment_links;
  model.recruitment_links[0].allocations[0].fraction = 0.5;
  EXPECT_THROW(model.PrepareRecruitmentLinks(), std::invalid_argument);
  model.recruitment_links = valid;
  model.recruitment_links.push_back(valid[0]);
  EXPECT_THROW(model.PrepareRecruitmentLinks(), std::invalid_argument);
  model.recruitment_links = valid;
  male->recruitment = std::make_shared<CountingMean>();
  EXPECT_THROW(model.PrepareRecruitmentLinks(), std::invalid_argument);
  male->recruitment = mean;
  male->years[0] = 1999;
  EXPECT_THROW(model.PrepareRecruitmentLinks(), std::invalid_argument);
  male->years[0] = 2000;
  auto fleet = std::make_shared<fims_popdy::Fleet<double>>();
  female->fleets.push_back(fleet);
  male->fleets.push_back(fleet);
  EXPECT_THROW(model.PrepareRecruitmentLinks(), std::invalid_argument);
}
// Description: a stock with no reproductive contribution fails explicitly.
TEST_F(LinkedRecruitmentTest, RejectsZeroSpawningStock) {
  std::dynamic_pointer_cast<fims_popdy::ConstantMaturity<double>>(
      female->maturity)
      ->proportion = 0.0;
  EXPECT_THROW(model.Evaluate(), std::invalid_argument);
  EXPECT_EQ(process->calls, 0);
}

// Description: a linked module cannot overwrite expectations through an
// outsider.
TEST_F(LinkedRecruitmentTest, RejectsRecruitmentSharedOutsideItsLink) {
  auto other = MakePopulation(0.1, 0.2, 1.0);
  EXPECT_THROW(model.PrepareRecruitmentLinks(), std::invalid_argument);
}

// Description: pooled observations sum independently weighted component counts
// before normalization, including sex-specific length mappings and repeat
// evaluations.
TEST_F(LinkedRecruitmentTest, PoolsFleetCountsBeforeObservationNormalization) {
  auto f = MakeFleet(female, 0.2, 0.7, false);
  auto m = MakeFleet(male, 0.35, 0.4, true);
  auto target = MakeFleet(nullptr, 1.0, 1.0, false);
  auto growth =
      std::dynamic_pointer_cast<fims_popdy::EWAAGrowth<double>>(male->growth);
  for (auto &year : growth->ewaa)
    for (auto &weight : year.second)
      weight.second = 4.0;
  target->fleet_observed_agecomp_data_id_m = 42;
  target->observed_agecomp_data =
      std::make_shared<fims_data_object::DataObject<double>>(3, 3);
  target->observed_agecomp_data->data = {10, 20, 30, 10, 20, 30, 10, 20, 30};
  target->fleet_observed_lengthcomp_data_id_m = 43;
  target->observed_lengthcomp_data =
      std::make_shared<fims_data_object::DataObject<double>>(3, 2);
  target->observed_lengthcomp_data->data = {12, 18, 12, 18, 12, 18};
  model.observation_fleet_ids.insert(target->GetId());
  model.observation_links = {{target->GetId(), {f->GetId(), m->GetId()}, true}};
  model.Initialize();
  model.Evaluate();
  auto expected = model.GetFleetDerivedQuantities(target->GetId());
  auto &fdq = model.GetFleetDerivedQuantities(f->GetId());
  auto &mdq = model.GetFleetDerivedQuantities(m->GetId());
  for (const auto &key :
       {"catch_numbers_at_age", "catch_weight_at_age", "index_numbers_at_age",
        "index_weight_at_age", "catch_numbers_at_length",
        "index_numbers_at_length", "catch_numbers", "catch_weight",
        "index_numbers", "index_weight"}) {
    for (size_t i = 0; i < expected[key].size(); ++i)
      EXPECT_NEAR(expected[key][i], fdq[key][i] + mdq[key][i], 1e-8);
  }
  for (size_t y = 0; y < 3; ++y) {
    EXPECT_NEAR(expected["catch_expected"][y], expected["catch_weight"][y],
                1e-8);
    EXPECT_NEAR(expected["index_expected"][y], expected["index_numbers"][y],
                1e-8);
    for (size_t a = 0; a < 3; ++a) {
      size_t i = y * 3 + a;
      double proportion =
          expected["catch_numbers_at_age"][i] / expected["catch_numbers"][y];
      EXPECT_NEAR(expected["agecomp_proportion"][i], proportion, 1e-12);
      EXPECT_NEAR(expected["agecomp_expected"][i], 60 * proportion, 1e-10);
    }
    for (size_t l = 0; l < 2; ++l) {
      size_t i = y * 2 + l;
      double proportion =
          expected["catch_numbers_at_length"][i] / expected["catch_numbers"][y];
      EXPECT_NEAR(expected["lengthcomp_proportion"][i], proportion, 1e-12);
      EXPECT_NEAR(expected["lengthcomp_expected"][i], 30 * proportion, 1e-10);
    }
  }
  std::reverse(model.populations.begin(), model.populations.end());
  std::reverse(model.observation_links[0].source_ids.begin(),
               model.observation_links[0].source_ids.end());
  model.Evaluate();
  auto &again = model.GetFleetDerivedQuantities(target->GetId());
  EXPECT_EQ(expected["catch_expected"], again["catch_expected"]);
  EXPECT_EQ(expected["lengthcomp_proportion"], again["lengthcomp_proportion"]);
  model.observation_links[0].use_catch = false;
  model.Evaluate();
  for (size_t a = 0; a < 3; ++a)
    EXPECT_NEAR(again["agecomp_proportion"][a],
                again["index_numbers_at_age"][a] / again["index_numbers"][0],
                1e-12);
  for (size_t l = 0; l < 2; ++l)
    EXPECT_NEAR(again["lengthcomp_proportion"][l],
                again["index_numbers_at_length"][l] / again["index_numbers"][0],
                1e-12);
}

// Description: incompatible observation grids and feedback cycles are rejected.
TEST_F(LinkedRecruitmentTest, RejectsInvalidObservationLinks) {
  auto f = MakeFleet(female, 0.2, 0.7, false);
  auto m = MakeFleet(male, 0.3, 0.4, true);
  auto target = MakeFleet(nullptr, 1, 1, false);
  model.observation_fleet_ids.insert(target->GetId());
  model.observation_links = {{target->GetId(), {f->GetId(), m->GetId()}, true}};
  EXPECT_NO_THROW(model.PrepareObservationLinks());
  target->lengths[0] = 9;
  EXPECT_THROW(model.PrepareObservationLinks(), std::invalid_argument);
  target->lengths[0] = 10;
  model.observation_links[0].source_ids[1] = target->GetId();
  EXPECT_THROW(model.PrepareObservationLinks(), std::invalid_argument);
  model.observation_links[0].source_ids[1] = f->GetId();
  EXPECT_THROW(model.PrepareObservationLinks(), std::invalid_argument);
}
} // namespace
