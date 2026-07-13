#include <cstddef>
#include <memory>
#include <cmath>
#include <vector>

#include "gtest/gtest.h"
#include "test_stubs.hpp"

#include "common/fims_math.hpp"
#include "common/fims_vector.hpp"
#include "population_dynamics/alk/functors/fixed_matrix_alk.hpp"
#include "population_dynamics/alk/functors/growth_derived_alk.hpp"
#include "population_dynamics/alk/functors/alk_runtime.hpp"
#include "population_dynamics/fleet/fleet.hpp"
#include "population_dynamics/growth/growth_model_adapter.hpp"
#include "population_dynamics/growth/growth_products.hpp"
#include "../../inst/include/models/functors/catch_at_age.hpp"
#include "population_dynamics/size/functors/size_grid_builder.hpp"
#include "population_dynamics/size/growth_derived_size_provider.hpp"

namespace {

/**
 * @brief Minimal growth-derived observation used to control cached products in
 * ALK unit tests.
 */
struct FakeGrowthDerivedObservation
    : public fims_popdy::GrowthDerivedObservationBase<double> {
  fims_popdy::GrowthProducts<double> products;
  bool supports_growth_derived_alk = true;
  std::size_t prepare_calls = 0;
  bool products_prepared = false;

  void SetAgeOffset(double) override {}

  void Initialize(std::size_t n_years,
                  std::size_t n_ages,
                  std::size_t n_sexes = 1) override {
    products.Resize(n_years, n_ages, n_sexes);
    products_prepared = false;
  }

  bool SupportsGrowthDerivedALK() const override {
    return supports_growth_derived_alk;
  }

  void PrepareGrowthProducts() override {
    products_prepared = true;
    prepare_calls++;
  }
  const fims_popdy::GrowthProducts<double>* TryGetPreparedGrowthProducts()
      const override {
    if (!products_prepared) {
      return nullptr;
    }
    return &products;
  }

  double EvaluateWeightAtLength(const double& length) const override {
    return length;
  }

  const double evaluate(int year, const double& age) override {
    (void)year;
    return age;
  }
};

/**
 * @brief Fill a Von Bertalanffy adapter with a consistent single-sex test
 * parameter set.
 */
void ConfigureAdapter(
    fims_popdy::VonBertalanffyGrowthModelAdapter<double>& adapter,
    double length_at_ref_age_1,
    double length_at_ref_age_2,
    double growth_coefficient_K,
    double reference_age_for_length_1,
    double reference_age_for_length_2,
    double length_weight_a,
    double length_weight_b,
    double length_at_age_sd_at_reference_age_1,
    double length_at_age_sd_at_reference_age_2) {
  adapter.LengthAtRefAge1Vector().resize(1);
  adapter.LengthAtRefAge2Vector().resize(1);
  adapter.GrowthCoefficientKVector().resize(1);
  adapter.ReferenceAgeForLength1Vector().resize(1);
  adapter.ReferenceAgeForLength2Vector().resize(1);
  adapter.LengthWeightAVector().resize(1);
  adapter.LengthWeightBVector().resize(1);
  adapter.LengthAtAgeSdAtRefAgesVector().resize(2);

  adapter.LengthAtRefAge1Vector()[0] = fims_math::log(length_at_ref_age_1);
  adapter.LengthAtRefAge2Vector()[0] = fims_math::log(length_at_ref_age_2);
  adapter.GrowthCoefficientKVector()[0] = fims_math::log(growth_coefficient_K);
  adapter.ReferenceAgeForLength1Vector()[0] = reference_age_for_length_1;
  adapter.ReferenceAgeForLength2Vector()[0] = reference_age_for_length_2;
  adapter.LengthWeightAVector()[0] = fims_math::log(length_weight_a);
  adapter.LengthWeightBVector()[0] = fims_math::log(length_weight_b);
  adapter.LengthAtAgeSdAtRefAgesVector()[0] =
      fims_math::log(length_at_age_sd_at_reference_age_1);
  adapter.LengthAtAgeSdAtRefAgesVector()[1] =
      fims_math::log(length_at_age_sd_at_reference_age_2);
}

/**
 * @brief Build a fleet with caller-supplied observation-bin centers.
 */
std::shared_ptr<fims_popdy::Fleet<double>> MakeFleet(
    const std::vector<double>& length_centers) {
  auto fleet = std::make_shared<fims_popdy::Fleet<double>>();
  fleet->n_years = 1;
  fleet->n_ages = 3;
  fleet->n_lengths = length_centers.size();
  fleet->requires_age_length_mapping = true;
  fleet->lengths.resize(length_centers.size());

  for (std::size_t i = 0; i < length_centers.size(); ++i) {
    fleet->lengths[i] = length_centers[i];
  }

  fleet->length_bin_edges =
      fims_popdy::SizeGridBuilder::BuildObservationEdgesFromCenters(
          fleet->lengths);
  return fleet;
}

/**
 * @brief Build a fleet with explicit test length bins.
 */
std::shared_ptr<fims_popdy::Fleet<double>> MakeFleet() {
  return MakeFleet({200.0, 300.0, 400.0, 500.0});
}

/**
 * @brief Build a canonical population size grid for tests from prepared fleet
 * observation-bin edges.
 */
fims_popdy::SizeGrid MakePopulationSizeGridForFleet(
    const std::shared_ptr<fims_popdy::Fleet<double>>& fleet) {
  fims::Vector<fims::Vector<double>> fleet_edges;
  fleet_edges.emplace_back(fleet->length_bin_edges);
  return fims_popdy::SizeGridBuilder::BuildDefaultFromFleetEdges(fleet_edges);
}

/**
 * @brief Build a configured growth-derived size provider for ALK tests.
 */
std::shared_ptr<fims_popdy::SizeDistributionProviderBase<double>>
MakeConfiguredSizeProvider(
    const std::shared_ptr<fims_popdy::GrowthDerivedObservationBase<double>>&
        growth,
    const fims_popdy::SizeGrid* size_grid,
    std::size_t n_years,
    std::size_t n_ages) {
  auto size_provider =
      std::make_shared<fims_popdy::GrowthDerivedSizeProvider<double>>(growth);
  size_provider->SetPopulationSizeGrid(size_grid);
  size_provider->SetPopulationDimensions(n_years, n_ages);
  return size_provider;
}

/**
 * @brief Attach canonical population size support to a test population.
 */
void ConfigurePopulationSizeSupport(
    const std::shared_ptr<fims_popdy::Population<double>>& population,
    const std::shared_ptr<fims_popdy::Fleet<double>>& fleet,
    const std::shared_ptr<fims_popdy::GrowthDerivedObservationBase<double>>&
        growth) {
  population->size_grid = MakePopulationSizeGridForFleet(fleet);
  population->size_distribution_provider =
      MakeConfiguredSizeProvider(
          growth,
          &population->size_grid,
          fleet->n_years,
          fleet->n_ages);
}

/**
 * @brief Fill a fleet with a fixed age-to-length conversion matrix.
 */
void FillFixedMatrix(const std::shared_ptr<fims_popdy::Fleet<double>>& fleet) {
  fleet->age_to_length_conversion.resize(fleet->n_ages * fleet->n_lengths);

  fleet->age_to_length_conversion[0] = 0.1;
  fleet->age_to_length_conversion[1] = 0.2;
  fleet->age_to_length_conversion[2] = 0.3;
  fleet->age_to_length_conversion[3] = 0.4;
  fleet->age_to_length_conversion[4] = 0.4;
  fleet->age_to_length_conversion[5] = 0.3;
  fleet->age_to_length_conversion[6] = 0.2;
  fleet->age_to_length_conversion[7] = 0.1;
  fleet->age_to_length_conversion[8] = 0.25;
  fleet->age_to_length_conversion[9] = 0.25;
  fleet->age_to_length_conversion[10] = 0.25;
  fleet->age_to_length_conversion[11] = 0.25;
}

TEST(FixedMatrixALK, IsActiveWithValidMatrix) {
  auto fleet = MakeFleet();
  FillFixedMatrix(fleet);

  fims_popdy::FixedMatrixALK<double> alk(fleet);

  EXPECT_TRUE(alk.IsActive());
}

TEST(FixedMatrixALK, BuildALKRowCopiesExpectedAgeRow) {
  auto fleet = MakeFleet();
  FillFixedMatrix(fleet);

  fims_popdy::FixedMatrixALK<double> alk(fleet);
  fims::Vector<double> row;

  EXPECT_TRUE(alk.BuildALKRow(0, 1, row));
  ASSERT_EQ(row.size(), 4u);
  EXPECT_DOUBLE_EQ(row[0], 0.4);
  EXPECT_DOUBLE_EQ(row[1], 0.3);
  EXPECT_DOUBLE_EQ(row[2], 0.2);
  EXPECT_DOUBLE_EQ(row[3], 0.1);
}

TEST(FixedMatrixALK, IsInactiveWithWrongMatrixSize) {
  auto fleet = MakeFleet();
  fleet->age_to_length_conversion.resize(3);

  fims_popdy::FixedMatrixALK<double> alk(fleet);

  EXPECT_FALSE(alk.IsActive());
}

TEST(RuntimeALK, BuildFleetALKReturnsNullWhenGrowthDerivedPrepareFails) {
  auto population = std::make_shared<fims_popdy::Population<double>>();
  auto fleet = MakeFleet();
  FillFixedMatrix(fleet);

  auto growth = std::make_shared<FakeGrowthDerivedObservation>();
  growth->Initialize(1, fleet->n_ages, 2);

  population->growth = growth;
  ConfigurePopulationSizeSupport(population, fleet, growth);

  std::shared_ptr<fims_popdy::ALKBase<double>> alk =
      fims_popdy::BuildFleetALK<double>(population, fleet);

  EXPECT_EQ(alk, nullptr);
}

TEST(RuntimeALK, BuildFleetALKUsesGrowthDerivedWhenPrepareSucceeds) {
  auto population = std::make_shared<fims_popdy::Population<double>>();
  auto fleet = MakeFleet();

  auto growth = std::make_shared<FakeGrowthDerivedObservation>();
  growth->Initialize(1, fleet->n_ages, 1);

  population->growth = growth;
  ConfigurePopulationSizeSupport(population, fleet, growth);

  std::shared_ptr<fims_popdy::ALKBase<double>> alk =
      fims_popdy::BuildFleetALK<double>(population, fleet);

  ASSERT_NE(alk, nullptr);
  EXPECT_NE(
      std::dynamic_pointer_cast<fims_popdy::GrowthDerivedALK<double>>(alk),
      nullptr);
  EXPECT_EQ(
      std::dynamic_pointer_cast<fims_popdy::FixedMatrixALK<double>>(alk),
      nullptr);
  EXPECT_EQ(growth->prepare_calls, 1);
}

TEST(RuntimeALK, EnsureFleetALKThrowsWhenExistingGrowthDerivedPrepareFails) {
  auto population = std::make_shared<fims_popdy::Population<double>>();
  auto fleet = MakeFleet();
  FillFixedMatrix(fleet);

  auto growth = std::make_shared<FakeGrowthDerivedObservation>();
  growth->Initialize(1, fleet->n_ages, 2);

  population->growth = growth;
  ConfigurePopulationSizeSupport(population, fleet, growth);

  fleet->alk =
      std::make_shared<fims_popdy::GrowthDerivedALK<double>>(
          fleet,
          growth,
          population->size_distribution_provider);

  ASSERT_NE(fleet->alk, nullptr);
  EXPECT_NE(
      std::dynamic_pointer_cast<fims_popdy::GrowthDerivedALK<double>>(fleet->alk),
      nullptr);

  EXPECT_THROW(
      fims_popdy::EnsureFleetALK<double>(population, fleet),
      std::runtime_error);
}

TEST(RuntimeALK, EnsureFleetALKDoesNotReuseFixedALKForGrowthDerivedPopulation) {
  auto population = std::make_shared<fims_popdy::Population<double>>();
  auto fleet = MakeFleet();
  FillFixedMatrix(fleet);

  auto growth = std::make_shared<FakeGrowthDerivedObservation>();
  growth->Initialize(1, fleet->n_ages, 2);

  population->growth = growth;
  ConfigurePopulationSizeSupport(population, fleet, growth);

  fleet->alk = std::make_shared<fims_popdy::FixedMatrixALK<double>>(fleet);

  ASSERT_NE(fleet->alk, nullptr);
  EXPECT_NE(
      std::dynamic_pointer_cast<fims_popdy::FixedMatrixALK<double>>(fleet->alk),
      nullptr);

  EXPECT_THROW(
      fims_popdy::EnsureFleetALK<double>(population, fleet),
      std::runtime_error);
}

TEST(RuntimeALK, GrowthDerivedFleetMeanWeightAAThrowsWithoutFleetGrowthDerivedPath) {
  fims_popdy::CatchAtAge<double> model;
  auto fleet = MakeFleet();

  EXPECT_THROW(
      model.GrowthDerivedFleetMeanWeightAA(fleet, 0, 0),
      std::runtime_error);
}

TEST(RuntimeALK, GrowthDerivedFleetMeanWeightAAMatchesALKWeightedBinCenterWeights) {
  fims_popdy::CatchAtAge<double> model;
  auto fleet = MakeFleet();

  auto growth =
      std::make_shared<fims_popdy::VonBertalanffyGrowthModelAdapter<double>>();
  ConfigureAdapter(
      *growth, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0, 73.0);
  growth->SetAgeOffset(1.0);
  growth->Initialize(1, fleet->n_ages, 1);

  const fims_popdy::SizeGrid population_size_grid =
      MakePopulationSizeGridForFleet(fleet);
  std::shared_ptr<fims_popdy::SizeDistributionProviderBase<double>>
      size_provider =
          MakeConfiguredSizeProvider(
              growth,
              &population_size_grid,
              fleet->n_years,
              fleet->n_ages);

  auto growth_alk =
      std::make_shared<fims_popdy::GrowthDerivedALK<double>>(
          fleet,
          growth,
          size_provider);
  ASSERT_TRUE(growth_alk->IsActive());
  ASSERT_TRUE(growth_alk->PrepareForCurrentState());

  fleet->alk = growth_alk;

  fims::Vector<double> row;
  ASSERT_TRUE(growth_alk->BuildALKRow(0, 1, row));
  ASSERT_EQ(row.size(), fleet->n_lengths);

  double expected_mean_weight = 0.0;
  for (std::size_t i = 0; i < row.size(); ++i) {
    expected_mean_weight +=
        row[i] * 2.5e-11 * std::pow(fleet->lengths[i], 3.0);
  }

  const double observed_mean_weight =
      model.GrowthDerivedFleetMeanWeightAA(fleet, 0, 1);

  EXPECT_NEAR(observed_mean_weight, expected_mean_weight, 1e-8);
}

TEST(RuntimeALK, PopulationMeanWeightAAPreparesGrowthProductsWhenNeeded) {
  fims_popdy::CatchAtAge<double> model;
  auto population = std::make_shared<fims_popdy::Population<double>>();

  auto growth = std::make_shared<FakeGrowthDerivedObservation>();
  growth->Initialize(1, 3, 1);
  growth->products.MeanWAA(0, 0, 0) = 1.25;
  growth->products.MeanWAA(0, 1, 0) = 2.5;
  growth->products.MeanWAA(0, 2, 0) = 3.75;

  population->growth = growth;
  population->n_ages = 3;
  population->ages.resize(3);
  population->ages[0] = 1.0;
  population->ages[1] = 2.0;
  population->ages[2] = 3.0;

  EXPECT_EQ(growth->prepare_calls, 0u);

  const double mean_weight = model.PopulationMeanWeightAA(population, 0, 1);

  EXPECT_EQ(growth->prepare_calls, 1u);
  EXPECT_DOUBLE_EQ(mean_weight, 2.5);
}

TEST(GrowthDerivedALK, IsInactiveWithoutPreparedFleetObservationGeometry) {
  auto fleet = std::make_shared<fims_popdy::Fleet<double>>();
  fleet->n_years = 1;
  fleet->n_ages = 3;
  fleet->n_lengths = 4;

  auto growth =
      std::make_shared<fims_popdy::VonBertalanffyGrowthModelAdapter<double>>();
  ConfigureAdapter(
      *growth, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0, 73.0);
  growth->SetAgeOffset(1.0);
  growth->Initialize(1, 3, 1);

  std::shared_ptr<fims_popdy::SizeDistributionProviderBase<double>>
      size_provider;
  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth, size_provider);

  EXPECT_FALSE(alk.IsActive());
}

TEST(GrowthDerivedALK, PrepareForCurrentStateFailsForMultiSexProducts) {
  auto fleet = MakeFleet();
  auto growth = std::make_shared<FakeGrowthDerivedObservation>();

  growth->Initialize(1, fleet->n_ages, 2);
  const fims_popdy::SizeGrid population_size_grid =
      MakePopulationSizeGridForFleet(fleet);
  std::shared_ptr<fims_popdy::SizeDistributionProviderBase<double>>
      size_provider =
          MakeConfiguredSizeProvider(
              growth,
              &population_size_grid,
              fleet->n_years,
              fleet->n_ages);

  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth, size_provider);

  EXPECT_TRUE(alk.IsActive());
  EXPECT_FALSE(alk.PrepareForCurrentState());
}

TEST(GrowthDerivedALK, IsActiveDoesNotPrepareGrowthProducts) {
  auto fleet = MakeFleet();
  auto growth = std::make_shared<FakeGrowthDerivedObservation>();

  growth->Initialize(1, fleet->n_ages, 1);
  const fims_popdy::SizeGrid population_size_grid =
      MakePopulationSizeGridForFleet(fleet);
  std::shared_ptr<fims_popdy::SizeDistributionProviderBase<double>>
      size_provider =
          MakeConfiguredSizeProvider(
              growth,
              &population_size_grid,
              fleet->n_years,
              fleet->n_ages);

  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth, size_provider);

  EXPECT_TRUE(alk.IsActive());
  EXPECT_EQ(growth->prepare_calls, 0);
}

TEST(GrowthDerivedALK, BuildALKRowFailsBeforePrepareForCurrentState) {
  auto fleet = MakeFleet();
  auto growth = std::make_shared<FakeGrowthDerivedObservation>();

  growth->Initialize(1, fleet->n_ages, 1);
  const fims_popdy::SizeGrid population_size_grid =
      MakePopulationSizeGridForFleet(fleet);
  std::shared_ptr<fims_popdy::SizeDistributionProviderBase<double>>
      size_provider =
          MakeConfiguredSizeProvider(
              growth,
              &population_size_grid,
              fleet->n_years,
              fleet->n_ages);

  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth, size_provider);
  fims::Vector<double> row;

  ASSERT_TRUE(alk.IsActive());
  EXPECT_FALSE(alk.BuildALKRow(0, 1, row));
  EXPECT_EQ(growth->prepare_calls, 0);
}

TEST(GrowthDerivedALK, PrepareForCurrentStateEnablesRowBuilding) {
  auto fleet = MakeFleet();
  auto growth = std::make_shared<FakeGrowthDerivedObservation>();

  growth->Initialize(1, fleet->n_ages, 1);
  const fims_popdy::SizeGrid population_size_grid =
      MakePopulationSizeGridForFleet(fleet);
  std::shared_ptr<fims_popdy::SizeDistributionProviderBase<double>>
      size_provider =
          MakeConfiguredSizeProvider(
              growth,
              &population_size_grid,
              fleet->n_years,
              fleet->n_ages);

  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth, size_provider);
  fims::Vector<double> row;

  ASSERT_TRUE(alk.IsActive());
  ASSERT_TRUE(alk.PrepareForCurrentState());
  EXPECT_EQ(growth->prepare_calls, 1);
  ASSERT_TRUE(alk.BuildALKRow(0, 1, row));
  ASSERT_EQ(row.size(), fleet->n_lengths);
}

TEST(GrowthDerivedALK, PrepareForCurrentStateReusesAlreadyPreparedProducts) {
  auto fleet = MakeFleet();
  auto growth = std::make_shared<FakeGrowthDerivedObservation>();

  growth->Initialize(1, fleet->n_ages, 1);
  const fims_popdy::SizeGrid population_size_grid =
      MakePopulationSizeGridForFleet(fleet);
  std::shared_ptr<fims_popdy::SizeDistributionProviderBase<double>>
      size_provider =
          MakeConfiguredSizeProvider(
              growth,
              &population_size_grid,
              fleet->n_years,
              fleet->n_ages);
  growth->PrepareGrowthProducts();
  ASSERT_EQ(growth->prepare_calls, 1u);

  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth, size_provider);

  ASSERT_TRUE(alk.IsActive());
  ASSERT_TRUE(alk.PrepareForCurrentState());
  EXPECT_EQ(growth->prepare_calls, 1u);
}

TEST(GrowthDerivedALK, BuildALKRowReturnsFiniteNonnegativeNormalizedRow) {
  auto fleet = MakeFleet();

  auto growth =
      std::make_shared<fims_popdy::VonBertalanffyGrowthModelAdapter<double>>();
  ConfigureAdapter(
      *growth, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0, 73.0);
  growth->SetAgeOffset(1.0);
  growth->Initialize(1, fleet->n_ages, 1);
  const fims_popdy::SizeGrid population_size_grid =
      MakePopulationSizeGridForFleet(fleet);
  std::shared_ptr<fims_popdy::SizeDistributionProviderBase<double>>
      size_provider =
          MakeConfiguredSizeProvider(
              growth,
              &population_size_grid,
              fleet->n_years,
              fleet->n_ages);

  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth, size_provider);
  fims::Vector<double> row;

  ASSERT_TRUE(alk.IsActive());
  ASSERT_TRUE(alk.PrepareForCurrentState());
  ASSERT_TRUE(alk.BuildALKRow(0, 1, row));
  ASSERT_EQ(row.size(), fleet->n_lengths);

  double row_sum = 0.0;
  for (std::size_t i = 0; i < row.size(); ++i) {
    EXPECT_TRUE(std::isfinite(row[i]));
    EXPECT_GE(row[i], 0.0);
    row_sum += row[i];
  }

  EXPECT_NEAR(row_sum, 1.0, 1e-5);
}

TEST(GrowthDerivedALK, BuildALKRowFailsForOutOfRangeAge) {
  auto fleet = MakeFleet();

  auto growth =
      std::make_shared<fims_popdy::VonBertalanffyGrowthModelAdapter<double>>();
  ConfigureAdapter(
      *growth, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0, 73.0);
  growth->SetAgeOffset(1.0);
  growth->Initialize(1, fleet->n_ages, 1);
  const fims_popdy::SizeGrid population_size_grid =
      MakePopulationSizeGridForFleet(fleet);
  std::shared_ptr<fims_popdy::SizeDistributionProviderBase<double>>
      size_provider =
          MakeConfiguredSizeProvider(
              growth,
              &population_size_grid,
              fleet->n_years,
              fleet->n_ages);

  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth, size_provider);
  fims::Vector<double> row;

  ASSERT_TRUE(alk.IsActive());
  ASSERT_TRUE(alk.PrepareForCurrentState());
  EXPECT_FALSE(alk.BuildALKRow(0, fleet->n_ages, row));
}

TEST(GrowthDerivedALK, BuildALKRowHandlesDifferentFleetBinLayout) {
  auto fleet = MakeFleet({150.0, 250.0, 450.0, 700.0});

  auto growth =
      std::make_shared<fims_popdy::VonBertalanffyGrowthModelAdapter<double>>();
  ConfigureAdapter(
      *growth, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0, 73.0);
  growth->SetAgeOffset(1.0);
  growth->Initialize(1, fleet->n_ages, 1);

  const fims_popdy::SizeGrid population_size_grid =
      MakePopulationSizeGridForFleet(fleet);
  std::shared_ptr<fims_popdy::SizeDistributionProviderBase<double>>
      size_provider =
          MakeConfiguredSizeProvider(
              growth,
              &population_size_grid,
              fleet->n_years,
              fleet->n_ages);

  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth, size_provider);
  fims::Vector<double> row;

  ASSERT_TRUE(alk.IsActive());
  ASSERT_TRUE(alk.PrepareForCurrentState());
  ASSERT_TRUE(alk.BuildALKRow(0, 1, row));
  ASSERT_EQ(row.size(), fleet->n_lengths);

  double row_sum = 0.0;
  for (std::size_t i = 0; i < row.size(); ++i) {
    EXPECT_TRUE(std::isfinite(row[i]));
    EXPECT_GE(row[i], 0.0);
    row_sum += row[i];
  }

  EXPECT_NEAR(row_sum, 1.0, 1e-5);
}

}  // namespace
