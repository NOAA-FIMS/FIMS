#include <cmath>
#include <memory>

#include "gtest/gtest.h"
#include "models/functors/catch_at_age.hpp"
#include "population_dynamics/reference_points/reference_points.hpp"
#include "test_stubs.hpp"

namespace {
class SBPR0Test : public testing::Test {
 protected:
  void SetUp() override {
    population = std::make_shared<fims_popdy::Population<double>>();
    population->n_ages = 3;
    population->n_years = 2;
    population->ages = {1.0, 2.0, 3.0};
    // Later-year mortality must not affect the initial recruitment baseline.
    population->M = {0.2, 0.4, 0.6, 1.0, 1.0, 1.0};
    population->proportion_female = {0.5};
    auto growth = std::make_shared<fims_popdy::EWAAGrowth<double>>();
    growth->ewaa[0] = {{1.0, 1.0}, {2.0, 2.0}, {3.0, 3.0}};
    population->growth = growth;
    model.InitializePopulationDerivedQuantities(population->GetId());
    model.GetPopulationDerivedQuantities(
        population->GetId())["proportion_mature_at_age"] = {0.0, 0.5, 1.0,
                                                            1.0, 1.0, 1.0};
  }

  // Survival through each annual age uses that age's mortality. Summing the
  // geometric tail gives the terminal plus group independently of model code.
  double ExpectedSBPR0() const {
    const double age_two = std::exp(-0.2);
    const double plus_group = std::exp(-0.2 - 0.4) / -std::expm1(-0.6);
    return 0.5 * age_two + 1.5 * plus_group;
  }

  fims_popdy::CatchAtAge<double> model;
  std::shared_ptr<fims_popdy::Population<double>> population;
};

// Detect the off-by-one mortality index, including its effect on the plus
// group.
TEST_F(SBPR0Test, AgeVaryingMortalityMatchesAnalyticSurvival) {
  EXPECT_NEAR(model.CalculateSBPR0(population), ExpectedSBPR0(), 1e-12);
}

// Constant mortality should retain the previous result.
TEST_F(SBPR0Test, ConstantMortalityIsUnchanged) {
  population->M = {0.2, 0.2, 0.2, 1.0, 1.0, 1.0};
  const double expected =
      0.5 * std::exp(-0.2) + 1.5 * std::exp(-0.4) / -std::expm1(-0.2);
  EXPECT_NEAR(model.CalculateSBPR0(population), expected, 1e-12);
}

// The model and reference-point engine must use the same unfished age
// structure.
TEST_F(SBPR0Test, MatchesReferencePointEngine) {
  fims_popdy::ReferencePointInputs<double> inputs;
  inputs.natural_mortality = {0.2, 0.4, 0.6};
  inputs.weight = {1.0, 2.0, 3.0};
  inputs.maturity = {0.0, 0.5, 1.0};
  inputs.proportion_female = {0.5, 0.5, 0.5};
  const auto unfished = fims_popdy::CalculatePerRecruit(inputs, 0.0);
  EXPECT_NEAR(model.CalculateSBPR0(population), unfished.spawning_biomass,
              1e-12);
}

// Correct baseline scaling makes R0 a fixed point at independent unfished S0.
TEST_F(SBPR0Test, BevertonHoltReturnsR0AtUnfishedEquilibrium) {
  fims_popdy::SRBevertonHolt<double> recruitment;
  recruitment.log_rzero = {std::log(1000.0)};
  recruitment.logit_steep = {std::log((0.75 - 0.2) / (1.0 - 0.75))};
  EXPECT_NEAR(recruitment.evaluate_mean(1000.0 * ExpectedSBPR0(),
                                        model.CalculateSBPR0(population)),
              1000.0, 1e-10);
}
}  // namespace
