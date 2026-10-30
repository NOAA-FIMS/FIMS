#include "gtest/gtest.h"
#include "test_population_test_fixture.hpp"

TEST(Subpopulation, ValidatesConservationAndNames) {
  auto groups = fims_popdy::MakeTwoSexSubpopulations(0.6, 0.4);
  EXPECT_EQ(groups[1].reproductive_contribution, 0.0);
  EXPECT_THROW(fims_popdy::MakeTwoSexSubpopulations(-0.1, 0.5),
               std::invalid_argument);
  groups[1].name = "female";
  EXPECT_THROW(fims_popdy::ValidateSubpopulations(groups),
               std::invalid_argument);
  groups = fims_popdy::MakeTwoSexSubpopulations();
  groups[1].recruitment_fraction = 0.4;
  EXPECT_THROW(fims_popdy::ValidateSubpopulations(groups),
               std::invalid_argument);
  groups[1].recruitment_fraction = std::numeric_limits<double>::quiet_NaN();
  EXPECT_THROW(fims_popdy::ValidateSubpopulations(groups),
               std::invalid_argument);
}

class ExplicitSubpopulationTest : public CAAEvaluateTestFixture {
 protected:
  void Configure(bool unequal = false) {
    population->subpopulations = fims_popdy::MakeTwoSexSubpopulations(0.6, 0.4);
    const size_t plane = n_years * n_ages;
    population->log_M.resize(2 * plane);
    for (size_t i = 0; i < plane; ++i) {
      population->log_M[i] = std::log(0.2);
      population->log_M[plane + i] = std::log(unequal ? 0.5 : 0.2);
    }
    catch_at_age_model->Initialize();
    catch_at_age_model->Prepare();
  }
  void Run() { catch_at_age_model->EvaluateSubpopulations(population); }
};

TEST_F(ExplicitSubpopulationTest, SurvivalChangesSexRatioAndConservesTotals) {
  Configure(true);
  Run();
  auto& dq =
      catch_at_age_model->GetPopulationDerivedQuantities(population->GetId());
  const fims_popdy::SubpopulationLayout layout{size_t(n_years + 1),
                                               size_t(n_ages)};
  const auto& n = dq.at("numbers_at_age_by_subpopulation");
  const double female = n[layout.index(0, 1, 1)];
  const double male = n[layout.index(1, 1, 1)];
  EXPECT_NEAR(female / male, 1.5 * std::exp(0.3), 1e-12);
  for (int y = 0; y <= n_years; ++y) {
    for (int a = 0; a < n_ages; ++a) {
      EXPECT_NEAR(dq["numbers_at_age"][y * n_ages + a],
                  n[layout.index(0, y, a)] + n[layout.index(1, y, a)], 1e-8);
    }
  }
  EXPECT_NEAR(n[layout.index(0, 1, 0)] / dq["expected_recruitment"][1], 0.4,
              1e-12);
}

TEST_F(ExplicitSubpopulationTest, PlusGroupUsesEachGroupsSurvival) {
  Configure(true);
  Run();
  auto& dq =
      catch_at_age_model->GetPopulationDerivedQuantities(population->GetId());
  const fims_popdy::SubpopulationLayout state{size_t(n_years + 1),
                                              size_t(n_ages)};
  const fims_popdy::SubpopulationLayout rates{size_t(n_years), size_t(n_ages)};
  for (size_t g = 0; g < 2; ++g) {
    const auto& n = dq.at("numbers_at_age_by_subpopulation");
    const auto& z = dq.at("mortality_Z_by_subpopulation");
    const size_t previous = state.index(g, 0, n_ages - 2);
    const size_t rate = rates.index(g, 0, n_ages - 2);
    EXPECT_NEAR(n[state.index(g, 1, n_ages - 1)],
                n[previous] * std::exp(-z[rate]) +
                    n[previous + 1] * std::exp(-z[rate + 1]),
                1e-8);
  }
}

TEST_F(ExplicitSubpopulationTest, UnfishedEquilibriumAndSBPRAgree) {
  Configure(true);
  Run();
  auto& dq =
      catch_at_age_model->GetPopulationDerivedQuantities(population->GetId());
  const double r0 = std::exp(population->recruitment->log_rzero[0]);
  EXPECT_NEAR(dq["unfished_spawning_biomass"][0] / r0,
              catch_at_age_model->CalculateSBPR0(population), 1e-10);
  const fims_popdy::SubpopulationLayout layout{size_t(n_years + 1),
                                               size_t(n_ages)};
  const auto& n = dq.at("unfished_numbers_at_age_by_subpopulation");
  for (size_t g = 0; g < 2; ++g) {
    const double m = g == 0 ? 0.2 : 0.5;
    const double fraction = g == 0 ? 0.4 : 0.6;
    EXPECT_NEAR(n[layout.index(g, 0, n_ages - 1)] / r0,
                fraction * std::exp(-m * (n_ages - 1)) / (1 - std::exp(-m)),
                1e-12);
    for (int a = 0; a < n_ages; ++a) {
      EXPECT_NEAR(n[layout.index(g, 0, a)], n[layout.index(g, 1, a)], 1e-8);
    }
  }
}

TEST_F(ExplicitSubpopulationTest, CatchAndIndexSumGroupPredictions) {
  Configure(true);
  Run();
  auto& pdq =
      catch_at_age_model->GetPopulationDerivedQuantities(population->GetId());
  const auto& fleet = population->fleets[0];
  auto& dq = catch_at_age_model->GetFleetDerivedQuantities(fleet->GetId());
  const size_t plane = n_years * n_ages;
  for (const std::string name :
       {"catch_numbers_at_age", "catch_weight_at_age", "index_numbers_at_age",
        "index_weight_at_age"}) {
    for (size_t i = 0; i < plane; ++i) {
      EXPECT_NEAR(dq[name][i],
                  dq[name + "_by_subpopulation"][i] +
                      dq[name + "_by_subpopulation"][plane + i],
                  1e-8);
    }
  }
  const double f = fleet->Fmort[0] * population->f_multiplier[0] *
                   fleet->selectivity->evaluate(population->ages[0], 0);
  const double z = pdq["mortality_Z_by_subpopulation"][0];
  EXPECT_NEAR(
      dq["catch_numbers_at_age_by_subpopulation"][0],
      pdq["numbers_at_age_by_subpopulation"][0] * f / z * (1 - std::exp(-z)),
      1e-8);
}

TEST_F(ExplicitSubpopulationTest, EqualBiologyMatchesPooledFishedDynamics) {
  // Constant M and entry ratio reproduce the legacy fished model exactly.
  Configure();
  population->subpopulations = fims_popdy::MakeTwoSexSubpopulations();
  population->proportion_female = fims::Vector<double>(1, 0.5);
  population->log_M.resize(n_years * n_ages);
  catch_at_age_model->Initialize();
  catch_at_age_model->Prepare();
  Run();
  auto& dq =
      catch_at_age_model->GetPopulationDerivedQuantities(population->GetId());
  const auto explicit_numbers = dq["numbers_at_age"];
  const auto explicit_sb = dq["spawning_biomass"];
  const auto explicit_unfished = dq["unfished_numbers_at_age"];
  const auto explicit_unfished_sb = dq["unfished_spawning_biomass"];
  population->subpopulations.clear();
  catch_at_age_model->Initialize();
  catch_at_age_model->Evaluate();
  for (size_t i = 0; i < explicit_numbers.size(); ++i) {
    EXPECT_NEAR(explicit_numbers[i], dq["numbers_at_age"][i], 1e-7);
    EXPECT_NEAR(explicit_unfished[i], dq["unfished_numbers_at_age"][i], 1e-7);
  }
  for (size_t i = 0; i < explicit_sb.size(); ++i) {
    EXPECT_NEAR(explicit_sb[i], dq["spawning_biomass"][i], 1e-7);
    EXPECT_NEAR(explicit_unfished_sb[i], dq["unfished_spawning_biomass"][i],
                1e-7);
  }
}

TEST_F(ExplicitSubpopulationTest, RepeatedEvaluationDoesNotAccumulateState) {
  Configure(true);
  Run();
  const auto before =
      catch_at_age_model->GetPopulationDerivedQuantities(population->GetId());
  catch_at_age_model->Prepare();
  Run();
  auto& after =
      catch_at_age_model->GetPopulationDerivedQuantities(population->GetId());
  for (const auto& pair : before) {
    for (size_t i = 0; i < pair.second.size(); ++i) {
      EXPECT_DOUBLE_EQ(pair.second[i], after[pair.first][i]);
    }
  }
}

TEST_F(ExplicitSubpopulationTest, SupportsThirdGroupAndRejectsInvalidShapes) {
  Configure();
  population->subpopulations = {{"female", 0.4, 0.3, 1.0},
                                {"male", 0.4, 0.4, 0.0},
                                {"other", 0.2, 0.3, 0.0}};
  EXPECT_THROW(catch_at_age_model->Initialize(), std::invalid_argument);
  population->log_M.resize(n_years * n_ages);
  catch_at_age_model->Initialize();
  catch_at_age_model->Prepare();
  Run();
  auto& dq =
      catch_at_age_model->GetPopulationDerivedQuantities(population->GetId());
  EXPECT_EQ(dq["numbers_at_age_by_subpopulation"].size(),
            size_t(3 * (n_years + 1) * n_ages));
}

TEST_F(ExplicitSubpopulationTest, MortalityUsesGroupYearAgeOrder) {
  Configure(true);
  const size_t plane = n_years * n_ages;
  for (size_t g = 0; g < 2; ++g) {
    for (int y = 0; y < n_years; ++y) {
      for (int a = 0; a < n_ages; ++a) {
        population->log_M[g * plane + y * n_ages + a] =
            std::log(0.1 + 0.2 * g + 0.01 * y + 0.001 * a);
      }
    }
  }
  catch_at_age_model->Prepare();
  Run();
  auto& dq =
      catch_at_age_model->GetPopulationDerivedQuantities(population->GetId());
  for (size_t i = 0; i < 2 * plane; ++i) {
    EXPECT_DOUBLE_EQ(dq["mortality_M_by_subpopulation"][i],
                     std::exp(population->log_M[i]));
  }
}

TEST_F(ExplicitSubpopulationTest, SeparateInitialAbundanceIgnoresAllocation) {
  Configure();
  population->log_init_naa.resize(2 * n_ages);
  for (int a = 0; a < n_ages; ++a) {
    population->log_init_naa[a] = std::log(100.0 + a);
    population->log_init_naa[n_ages + a] = std::log(200.0 + a);
  }
  catch_at_age_model->Initialize();
  catch_at_age_model->Prepare();
  Run();
  auto& dq =
      catch_at_age_model->GetPopulationDerivedQuantities(population->GetId());
  for (int a = 0; a < n_ages; ++a) {
    EXPECT_NEAR(dq["numbers_at_age"][a], 300.0 + 2 * a, 1e-10);
  }
}

TEST_F(ExplicitSubpopulationTest, UsesTimeVaryingMaturityThroughTerminalYear) {
  Configure(true);
  auto maturity = std::make_shared<fims_popdy::LogisticMaturity<double>>();
  maturity->inflection_point.resize(n_years);
  maturity->slope = fims::Vector<double>(1, 0.5);
  for (int y = 0; y < n_years; ++y)
    maturity->inflection_point[y] = 2.0 + y * 0.1;
  population->maturity = maturity;
  Run();
  auto& dq =
      catch_at_age_model->GetPopulationDerivedQuantities(population->GetId());
  for (int y = 0; y <= n_years; ++y) {
    double spawning = 0.0;
    for (int a = 0; a < n_ages; ++a) {
      const double mature =
          maturity->evaluate(population->ages[a], std::min(y, n_years - 1));
      EXPECT_DOUBLE_EQ(dq["proportion_mature_at_age"][y * n_ages + a], mature);
      spawning += dq["numbers_at_age_by_subpopulation"][y * n_ages + a] *
                  mature *
                  catch_at_age_model->PopulationMeanWeightAA(population, y, a);
    }
    EXPECT_NEAR(dq["spawning_biomass"][y], spawning, 1e-7);
  }
}

TEST_F(ExplicitSubpopulationTest, FleetWeightsAffectObservationsNotBiomass) {
  Configure(true);
  auto& fleet = population->fleets[0];
  fleet->weight_at_age.resize(n_years * n_ages);
  for (int i = 0; i < n_years * n_ages; ++i)
    fleet->weight_at_age[i] = 10.0 + 0.01 * i;
  Run();
  auto& dq = catch_at_age_model->GetFleetDerivedQuantities(fleet->GetId());
  const size_t plane = n_years * n_ages;
  for (size_t i = 0; i < plane; ++i) {
    EXPECT_NEAR(dq["catch_weight_at_age"][i],
                dq["catch_numbers_at_age"][i] * fleet->weight_at_age[i], 1e-7);
    EXPECT_NEAR(dq["index_weight_at_age"][i],
                dq["index_numbers_at_age"][i] * fleet->weight_at_age[i], 1e-7);
    for (size_t g = 0; g < 2; ++g) {
      EXPECT_DOUBLE_EQ(
          dq["catch_weight_at_age_by_subpopulation"][g * plane + i],
          dq["catch_numbers_at_age_by_subpopulation"][g * plane + i] *
              fleet->weight_at_age[i]);
      EXPECT_DOUBLE_EQ(
          dq["index_weight_at_age_by_subpopulation"][g * plane + i],
          dq["index_numbers_at_age_by_subpopulation"][g * plane + i] *
              fleet->weight_at_age[i]);
    }
  }
  auto& pdq =
      catch_at_age_model->GetPopulationDerivedQuantities(population->GetId());
  double biomass = 0.0;
  for (int a = 0; a < n_ages; ++a)
    biomass += pdq["numbers_at_age"][a] *
               catch_at_age_model->PopulationMeanWeightAA(population, 0, a);
  EXPECT_NEAR(pdq["biomass"][0], biomass, 1e-7);
}
