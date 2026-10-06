#include <cmath>
#include <limits>
#include <vector>

#include "gtest/gtest.h"
#include "population_dynamics/reference_points/reference_points.hpp"

namespace {
using fims_popdy::CalculatePerRecruit;
using fims_popdy::CalculateSPR;
using fims_popdy::ReferencePointInputs;
using fims_popdy::SPROptions;
using fims_popdy::SPRStatus;

ReferencePointInputs<double> MakeInputs() {
  ReferencePointInputs<double> inputs;
  inputs.natural_mortality = {0.2, 0.4, 0.6};
  inputs.weight = {1.0, 2.0, 3.0};
  inputs.maturity = {0.0, 0.5, 1.0};
  inputs.proportion_female = {0.5, 0.5, 0.5};
  inputs.fleets.push_back({1.0, {0.2, 0.7, 1.0}, {1.0, 2.0, 3.0}});
  return inputs;
}

// Hand-calculated unfished survival detects use of the wrong age's mortality.
TEST(ReferencePoints, UnfishedAgeVaryingMortality) {
  const auto result = CalculatePerRecruit(MakeInputs(), 0.0);
  const double n1 = std::exp(-0.2);
  const double n2 = std::exp(-0.6) / (1.0 - std::exp(-0.6));
  EXPECT_DOUBLE_EQ(result.numbers[0], 1.0);
  EXPECT_NEAR(result.numbers[1], n1, 1e-12);
  EXPECT_NEAR(result.numbers[2], n2, 1e-12);
  EXPECT_NEAR(result.biomass, 1.0 + 2.0 * n1 + 3.0 * n2, 1e-12);
  EXPECT_NEAR(result.spawning_biomass, 0.5 * n1 + 1.5 * n2, 1e-12);
  EXPECT_DOUBLE_EQ(result.yield, 0.0);
}

// A one-age plus group has analytic abundance and annual catch.
TEST(ReferencePoints, SingleAgePlusGroup) {
  auto inputs = MakeInputs();
  inputs.natural_mortality = {0.2};
  inputs.weight = {2.0};
  inputs.maturity = {1.0};
  inputs.proportion_female = {0.5};
  inputs.fleets = {{1.0, {1.0}, {3.0}}};
  const auto result = CalculatePerRecruit(inputs, 0.3);
  EXPECT_NEAR(result.numbers[0], 1.0 / (1.0 - std::exp(-0.5)), 1e-12);
  EXPECT_NEAR(result.spawning_biomass, result.numbers[0], 1e-12);
  EXPECT_NEAR(result.yield, 3.0 * 0.3 / 0.5, 1e-12);
}

// Iterating annual cohorts independently converges to the calculated numbers.
TEST(ReferencePoints, MatchesLongRunProjectionWithTwoFleets) {
  auto inputs = MakeInputs();
  inputs.fleets[0].share = 0.3;
  inputs.fleets.push_back({0.7, {0.8, 1.0, 0.4}, {1.2, 2.2, 3.2}});
  const double f = 0.5;
  std::vector<double> numbers(3, 0.0);
  const std::vector<double> z = {0.2 + f * (0.3 * 0.2 + 0.7 * 0.8),
                                 0.4 + f * (0.3 * 0.7 + 0.7),
                                 0.6 + f * (0.3 + 0.7 * 0.4)};
  for (size_t year = 0; year < 1000; ++year) {
    numbers = {1.0, numbers[0] * std::exp(-z[0]),
               numbers[1] * std::exp(-z[1]) + numbers[2] * std::exp(-z[2])};
  }
  const auto result = CalculatePerRecruit(inputs, f);
  for (size_t a = 0; a < numbers.size(); ++a) {
    EXPECT_NEAR(result.numbers[a], numbers[a], 1e-12);
  }
  double total = 0.0;
  for (size_t k = 0; k < inputs.fleets.size(); ++k) {
    double yield = 0.0;
    for (size_t a = 0; a < numbers.size(); ++a) {
      // Partition annual deaths into catches from each fleet.
      const double deaths = numbers[a] * (1.0 - std::exp(-z[a]));
      yield += deaths * f * inputs.fleets[k].share *
               inputs.fleets[k].selectivity[a] / z[a] *
               inputs.fleets[k].weight[a];
    }
    EXPECT_NEAR(result.fleet_yield[k], yield, 1e-12);
    total += yield;
  }
  EXPECT_NEAR(result.yield, total, 1e-12);
}

// Splitting a fleet without changing selectivity or weight preserves totals.
TEST(ReferencePoints, FleetSplitPreservesTotals) {
  auto inputs = MakeInputs();
  const auto original = CalculatePerRecruit(inputs, 0.4);
  inputs.fleets[0].share = 0.25;
  auto second = inputs.fleets[0];
  second.share = 0.75;
  inputs.fleets.push_back(second);
  const auto split = CalculatePerRecruit(inputs, 0.4);
  EXPECT_NEAR(split.yield, original.yield, 1e-12);
  EXPECT_NEAR(split.spawning_biomass, original.spawning_biomass, 1e-12);
  EXPECT_NEAR(split.fleet_yield[1], 3.0 * split.fleet_yield[0], 1e-12);
}

// For two ages with maturity only in the plus group, SBPR=exp(-Z)/(1-exp(-Z)).
TEST(ReferencePoints, SPRMatchesAnalyticSolution) {
  ReferencePointInputs<double> inputs;
  inputs.natural_mortality = {0.2, 0.2};
  inputs.weight = {1.0, 1.0};
  inputs.maturity = {0.0, 1.0};
  inputs.proportion_female = {1.0, 1.0};
  inputs.fleets = {{1.0, {1.0, 1.0}, {1.0, 1.0}}};
  const double target = 0.4;
  const double expected_f = std::log(1.0 + std::expm1(0.2) / target) - 0.2;
  const auto result = CalculateSPR(inputs, target);
  EXPECT_EQ(result.status, SPRStatus::converged);
  EXPECT_NEAR(result.fishing_mortality, expected_f, 1e-7);
  EXPECT_NEAR(result.spr, target, 1e-8);
  EXPECT_DOUBLE_EQ(result.residual, result.spr - target);
  EXPECT_GT(result.iterations, 0u);
}

// Endpoints, failed bounds, and iteration exhaustion have explicit outcomes.
TEST(ReferencePoints, SPRSolverDiagnostics) {
  const auto inputs = MakeInputs();
  const auto unfished = CalculateSPR(inputs, 1.0);
  EXPECT_EQ(unfished.status, SPRStatus::converged);
  EXPECT_DOUBLE_EQ(unfished.fishing_mortality, 0.0);
  SPROptions options;
  options.max_f = 0.01;
  auto result = CalculateSPR(inputs, 0.4, options);
  EXPECT_EQ(result.status, SPRStatus::not_bracketed);
  EXPECT_DOUBLE_EQ(result.fishing_mortality, options.max_f);
  EXPECT_GT(result.residual, 0.0);
  const double endpoint =
      CalculatePerRecruit(inputs, options.max_f).spawning_biomass /
      CalculatePerRecruit(inputs, 0.0).spawning_biomass;
  EXPECT_EQ(CalculateSPR(inputs, endpoint, options).status,
            SPRStatus::converged);
  options.max_f = 5.0;
  options.max_iterations = 1;
  result = CalculateSPR(inputs, 0.4, options);
  EXPECT_EQ(result.status, SPRStatus::iteration_limit);
  EXPECT_EQ(result.iterations, 1u);
}

// No fishing vulnerability produces valid per-recruit results but no SPR
// target.
TEST(ReferencePoints, NoFishingAndZeroSpawningBiomass) {
  auto inputs = MakeInputs();
  inputs.fleets.clear();
  EXPECT_EQ(CalculateSPR(inputs, 0.4).status, SPRStatus::not_bracketed);
  EXPECT_DOUBLE_EQ(CalculatePerRecruit(inputs, 1.0).yield, 0.0);
  inputs = MakeInputs();
  inputs.fleets[0].selectivity = {0.0, 0.0, 0.0};
  EXPECT_EQ(CalculateSPR(inputs, 0.4).status, SPRStatus::not_bracketed);
  inputs.maturity = {0.0, 0.0, 0.0};
  EXPECT_DOUBLE_EQ(CalculatePerRecruit(inputs, 0.0).spawning_biomass, 0.0);
  EXPECT_THROW(CalculateSPR(inputs, 0.4), std::domain_error);
}

// Reject bad dimensions, invalid biological values, and invalid solver inputs.
TEST(ReferencePoints, InvalidInputs) {
  EXPECT_THROW(CalculatePerRecruit(ReferencePointInputs<double>(), 0.0),
               std::invalid_argument);
  auto inputs = MakeInputs();
  inputs.weight = {1.0};
  EXPECT_THROW(CalculatePerRecruit(inputs, 0.0), std::invalid_argument);
  inputs = MakeInputs();
  inputs.natural_mortality[0] = 0.0;
  EXPECT_THROW(CalculatePerRecruit(inputs, 0.0), std::invalid_argument);
  inputs = MakeInputs();
  inputs.maturity[0] = 1.1;
  EXPECT_THROW(CalculatePerRecruit(inputs, 0.0), std::invalid_argument);
  inputs = MakeInputs();
  inputs.fleets[0].share = 0.5;
  EXPECT_THROW(CalculatePerRecruit(inputs, 0.0), std::invalid_argument);
  inputs = MakeInputs();
  inputs.fleets[0].selectivity[0] = -1.0;
  EXPECT_THROW(CalculatePerRecruit(inputs, 0.0), std::invalid_argument);
  inputs = MakeInputs();
  inputs.weight[0] = std::numeric_limits<double>::quiet_NaN();
  EXPECT_THROW(CalculatePerRecruit(inputs, 0.0), std::invalid_argument);
  inputs = MakeInputs();
  EXPECT_THROW(CalculatePerRecruit(inputs, -0.1), std::invalid_argument);
  EXPECT_THROW(
      CalculatePerRecruit(inputs, std::numeric_limits<double>::infinity()),
      std::invalid_argument);
  EXPECT_THROW(CalculateSPR(inputs, 0.0), std::invalid_argument);
  EXPECT_THROW(CalculateSPR(inputs, 1.1), std::invalid_argument);
  SPROptions options;
  options.tolerance = 0.0;
  EXPECT_THROW(CalculateSPR(inputs, 0.4, options), std::invalid_argument);
  options = SPROptions();
  options.max_f = 0.0;
  EXPECT_THROW(CalculateSPR(inputs, 0.4, options), std::invalid_argument);
  options = SPROptions();
  options.max_iterations = 0;
  EXPECT_THROW(CalculateSPR(inputs, 0.4, options), std::invalid_argument);
}

// Repeated evaluations are independent and leave the biological snapshot
// intact.
TEST(ReferencePoints, RepeatedCallsDoNotChangeInputs) {
  const auto inputs = MakeInputs();
  const auto before = inputs;
  const auto first = CalculatePerRecruit(inputs, 0.4);
  CalculateSPR(inputs, 0.4);
  const auto second = CalculatePerRecruit(inputs, 0.4);
  EXPECT_DOUBLE_EQ(first.yield, second.yield);
  EXPECT_EQ(inputs.natural_mortality, before.natural_mortality);
  EXPECT_EQ(inputs.weight, before.weight);
  EXPECT_EQ(inputs.maturity, before.maturity);
  EXPECT_EQ(inputs.proportion_female, before.proportion_female);
  EXPECT_EQ(inputs.fleets[0].selectivity, before.fleets[0].selectivity);
  EXPECT_EQ(inputs.fleets[0].weight, before.fleets[0].weight);
  EXPECT_DOUBLE_EQ(inputs.fleets[0].share, before.fleets[0].share);
}

// The positive equilibrium must satisfy the FIMS Beverton--Holt equation.
TEST(ReferencePoints, EquilibriumMatchesRecruitmentFixedPoint) {
  const auto inputs = MakeInputs();
  const double phi0 = CalculatePerRecruit(inputs, 0.0).spawning_biomass;
  const fims_popdy::ReferencePointRecruitment sr{1000.0, 0.75, phi0};
  const auto unfished = fims_popdy::CalculateEquilibrium(inputs, 0.0, sr);
  EXPECT_NEAR(unfished.recruitment, 1000.0, 1e-10);
  EXPECT_NEAR(unfished.spawning_biomass, 1000.0 * phi0, 1e-10);
  const auto result = fims_popdy::CalculateEquilibrium(inputs, 0.4, sr);
  const double spawners = result.spawning_biomass;
  const double next_recruitment =
      0.8 * sr.rzero * sr.steepness * spawners /
      (0.2 * sr.rzero * sr.phi0 * (1.0 - sr.steepness) +
       spawners * (sr.steepness - 0.2));
  EXPECT_FALSE(result.collapsed);
  EXPECT_NEAR(next_recruitment, result.recruitment, 1e-10);
  EXPECT_NEAR(result.yield, result.fleet_yield[0], 1e-10);
  // A different recruitment baseline must not be silently replaced by this
  // year's SBPR.
  const fims_popdy::ReferencePointRecruitment changed{1000.0, 0.75, 2.0 * phi0};
  EXPECT_LT(fims_popdy::CalculateEquilibrium(inputs, 0.0, changed).recruitment,
            1000.0);
}

// Low spawning potential has only the extinction equilibrium.
TEST(ReferencePoints, EquilibriumCollapseAndInvalidRecruitment) {
  const auto inputs = MakeInputs();
  fims_popdy::ReferencePointRecruitment sr{
      1000.0, 0.75, CalculatePerRecruit(inputs, 0.0).spawning_biomass};
  const auto collapsed = fims_popdy::CalculateEquilibrium(inputs, 100.0, sr);
  EXPECT_TRUE(collapsed.collapsed);
  EXPECT_DOUBLE_EQ(collapsed.recruitment, 0.0);
  EXPECT_DOUBLE_EQ(collapsed.spawning_biomass, 0.0);
  EXPECT_DOUBLE_EQ(collapsed.yield, 0.0);
  sr.steepness = 0.2;
  EXPECT_THROW(fims_popdy::CalculateEquilibrium(inputs, 0.0, sr),
               std::invalid_argument);
  sr.steepness = 1.0;
  EXPECT_THROW(fims_popdy::CalculateEquilibrium(inputs, 0.0, sr),
               std::invalid_argument);
  sr.steepness = 0.75;
  sr.phi0 = 0.0;
  EXPECT_THROW(fims_popdy::CalculateEquilibrium(inputs, 0.0, sr),
               std::invalid_argument);
}

// Compare the optimized yield against a dense independent grid of candidate Fs.
TEST(ReferencePoints, MSYDominatesDenseGrid) {
  const auto inputs = MakeInputs();
  const fims_popdy::ReferencePointRecruitment sr{
      1000.0, 0.75, CalculatePerRecruit(inputs, 0.0).spawning_biomass};
  const auto result = fims_popdy::CalculateMSY(inputs, sr);
  EXPECT_EQ(result.status, fims_popdy::MSYStatus::converged);
  EXPECT_GT(result.fishing_mortality, 0.0);
  EXPECT_GT(result.equilibrium.yield, 0.0);
  for (size_t i = 0; i <= 5000; ++i) {
    const double candidate =
        fims_popdy::CalculateEquilibrium(inputs, i * 0.001, sr).yield;
    EXPECT_GE(result.equilibrium.yield + 1e-9, candidate);
  }
}

// Bounds, no fishing, and iteration limits must not masquerade as an optimum.
TEST(ReferencePoints, MSYDiagnostics) {
  auto inputs = MakeInputs();
  const fims_popdy::ReferencePointRecruitment sr{
      1000.0, 0.75, CalculatePerRecruit(inputs, 0.0).spawning_biomass};
  fims_popdy::MSYOptions options;
  options.max_f = 0.001;
  auto result = fims_popdy::CalculateMSY(inputs, sr, options);
  EXPECT_EQ(result.status, fims_popdy::MSYStatus::upper_bound);
  EXPECT_DOUBLE_EQ(result.fishing_mortality, options.max_f);
  options.max_f = 5.0;
  options.max_iterations = 1;
  EXPECT_EQ(fims_popdy::CalculateMSY(inputs, sr, options).status,
            fims_popdy::MSYStatus::iteration_limit);
  options.grid_intervals = 1;
  EXPECT_THROW(fims_popdy::CalculateMSY(inputs, sr, options),
               std::invalid_argument);
  inputs.fleets.clear();
  EXPECT_EQ(fims_popdy::CalculateMSY(inputs, sr).status,
            fims_popdy::MSYStatus::no_positive_yield);
}

// Large selectivity can put the entire productive range below the first grid
// point.
TEST(ReferencePoints, MSYFindsNarrowProductiveRegion) {
  auto inputs = MakeInputs();
  const fims_popdy::ReferencePointRecruitment sr{
      1000.0, 0.75, CalculatePerRecruit(inputs, 0.0).spawning_biomass};
  const auto original = fims_popdy::CalculateMSY(inputs, sr);
  for (size_t a = 0; a < inputs.fleets[0].selectivity.size(); ++a) {
    inputs.fleets[0].selectivity[a] *= 10000.0;
  }
  const auto scaled = fims_popdy::CalculateMSY(inputs, sr);
  EXPECT_EQ(scaled.status, fims_popdy::MSYStatus::converged);
  EXPECT_NEAR(scaled.fishing_mortality * 10000.0, original.fishing_mortality,
              1e-4);
  EXPECT_NEAR(scaled.equilibrium.yield, original.equilibrium.yield, 1e-4);
}
}  // namespace
