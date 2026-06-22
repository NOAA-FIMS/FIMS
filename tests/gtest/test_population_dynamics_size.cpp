#include <memory>
#include <cmath>
#include "common/fims_math.hpp"

#include "gtest/gtest.h"

#include "population_dynamics/growth/growth_model_adapter.hpp"
#include "population_dynamics/size/functors/size_grid_builder.hpp"
#include "population_dynamics/size/growth_derived_size_provider.hpp"

namespace {

/**
 * @brief Minimal growth-derived observation used to control prepared growth
 * products in size-subsystem unit tests.
 */
struct FakeGrowthDerivedObservation
    : public fims_popdy::GrowthDerivedObservationBase<double> {
  fims_popdy::GrowthProducts<double> products;
  bool products_prepared = false;

  void SetAgeOffset(double) override {}

  void Initialize(std::size_t n_years,
                  std::size_t n_ages,
                  std::size_t n_sexes = 1) override {
    products.Resize(n_years, n_ages, n_sexes);
    products_prepared = false;
  }

  bool SupportsGrowthDerivedALK() const override { return true; }

  void PrepareGrowthProducts() override { products_prepared = true; }

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

  const double evaluate(const double& age) const override { return age; }
};

std::shared_ptr<FakeGrowthDerivedObservation> MakePreparedGrowth(
    double mean_laa,
    double sd_laa) {
  auto growth = std::make_shared<FakeGrowthDerivedObservation>();
  growth->Initialize(1, 1, 1);
  growth->products.MeanLAA(0, 0, 0) = mean_laa;
  growth->products.SdLAA(0, 0, 0) = sd_laa;
  growth->PrepareGrowthProducts();
  return growth;
}

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

TEST(SizeGridBuilder, BuildObservationEdgesFromCentersUsesAdjacentMidpoints) {
  fims::Vector<double> centers = {1.0, 2.0, 4.0};

  const fims::Vector<double> edges =
      fims_popdy::SizeGridBuilder::BuildObservationEdgesFromCenters(centers);

  ASSERT_EQ(edges.size(), 4u);
  EXPECT_DOUBLE_EQ(edges[0], 0.5);
  EXPECT_DOUBLE_EQ(edges[1], 1.5);
  EXPECT_DOUBLE_EQ(edges[2], 3.0);
  EXPECT_DOUBLE_EQ(edges[3], 5.0);
}

TEST(SizeGridBuilder, BuildObservationEdgesFromCentersRejectsSingleCenter) {
  fims::Vector<double> centers = {3.0};

  EXPECT_THROW(
      fims_popdy::SizeGridBuilder::BuildObservationEdgesFromCenters(centers),
      std::runtime_error);
}

TEST(SizeGridBuilder,
     BuildDefaultFromFleetEdgesBuildsOneUnitBinsAnchoredToLowestLowerEdge) {
  fims::Vector<fims::Vector<double>> fleet_edges = {
      fims::Vector<double>{0.5, 1.5, 2.5},
      fims::Vector<double>{1.0, 2.0, 3.0, 4.0}};

  const fims_popdy::SizeGrid grid =
      fims_popdy::SizeGridBuilder::BuildDefaultFromFleetEdges(fleet_edges);

  ASSERT_TRUE(grid.IsConsistent());
  ASSERT_EQ(grid.n_bins, 5u);
  ASSERT_EQ(grid.edges.size(), 6u);
  EXPECT_DOUBLE_EQ(grid.edges[0], 0.5);
  EXPECT_DOUBLE_EQ(grid.edges[1], 1.5);
  EXPECT_DOUBLE_EQ(grid.edges[2], 2.5);
  EXPECT_DOUBLE_EQ(grid.edges[3], 3.5);
  EXPECT_DOUBLE_EQ(grid.edges[4], 4.5);
  EXPECT_DOUBLE_EQ(grid.edges[5], 5.5);
}

TEST(GrowthDerivedSizeProvider,
     PrepareSizeProductsBuildsNonnegativeNormalizedProbSizeRows) {
  const auto growth = MakePreparedGrowth(2.5, 0.4);
  const fims_popdy::SizeGrid grid =
      fims_popdy::SizeGridBuilder::BuildFromEdges({0.0, 1.0, 2.0, 3.0, 4.0});

  fims_popdy::GrowthDerivedSizeProvider<double> provider(growth);
  provider.SetPopulationSizeGrid(&grid);
  provider.SetPopulationDimensions(1, 1);
  provider.PrepareSizeProducts();

  double row_sum = 0.0;
  for (std::size_t size_bin_index = 0; size_bin_index < grid.n_bins;
       ++size_bin_index) {
    const double prob = provider.ProbSize(0, 0, size_bin_index);
    EXPECT_GE(prob, 0.0);
    row_sum += prob;
  }

  EXPECT_NEAR(row_sum, 1.0, 1e-10);
}

TEST(GrowthDerivedSizeProvider,
     PrepareSizeProductsPlacesUpperTailMassInTerminalPlusGroupBin) {
  const auto growth = MakePreparedGrowth(10.0, 0.2);
  const fims_popdy::SizeGrid grid =
      fims_popdy::SizeGridBuilder::BuildFromEdges({0.0, 1.0, 2.0, 3.0});

  fims_popdy::GrowthDerivedSizeProvider<double> provider(growth);
  provider.SetPopulationSizeGrid(&grid);
  provider.SetPopulationDimensions(1, 1);
  provider.PrepareSizeProducts();

  EXPECT_GT(provider.ProbSize(0, 0, grid.n_bins - 1), 0.99);
}

TEST(GrowthDerivedSizeProvider,
     PrepareSizeProductsHandlesDifferentGrowthVariability) {
  auto growth =
      std::make_shared<fims_popdy::VonBertalanffyGrowthModelAdapter<double>>();
  ConfigureAdapter(
      *growth,
      275.0,
      725.0,
      0.18,
      1.0,
      12.0,
      2.5e-11,
      3.0,
      10.0,
      120.0);
  growth->SetAgeOffset(1.0);
  growth->Initialize(1, 3, 1);
ASSERT_NO_THROW(growth->PrepareGrowthProducts());

  fims::Vector<fims::Vector<double>> fleet_edges;
  fleet_edges.emplace_back(
      fims::Vector<double>{150.0, 250.0, 350.0, 450.0, 550.0});
  const fims_popdy::SizeGrid size_grid =
      fims_popdy::SizeGridBuilder::BuildDefaultFromFleetEdges(fleet_edges);

  fims_popdy::GrowthDerivedSizeProvider<double> provider(growth);
  provider.SetPopulationSizeGrid(&size_grid);
  provider.SetPopulationDimensions(1, 3);

  ASSERT_NO_THROW(provider.PrepareSizeProducts());

  for (std::size_t age_index = 0; age_index < 3; ++age_index) {
    double row_sum = 0.0;

    for (std::size_t size_bin_index = 0;
         size_bin_index < size_grid.n_bins;
         ++size_bin_index) {
      const double prob =
          provider.ProbSize(0, age_index, size_bin_index);
      EXPECT_TRUE(std::isfinite(prob));
      EXPECT_GE(prob, 0.0);
      row_sum += prob;
    }

    EXPECT_NEAR(row_sum, 1.0, 1e-5);
  }
}

}  // namespace
