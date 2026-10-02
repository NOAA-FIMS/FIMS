#include <memory>
#include <cmath>
#include "common/fims_math.hpp"
#include "common/def.hpp"

#include "gtest/gtest.h"
#include "test_stubs.hpp"

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

  void Initialize(std::size_t n_years, std::size_t n_ages,
                  std::size_t n_sexes = 1) override {
    products.Resize(n_years, n_ages, n_sexes);
    products_prepared = false;
  }

  bool SupportsAgeToLengthConversionDerived() const override { return true; }

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

  const double evaluate(int year, const double& age) override {
    (void)year;
    return age;
  }
};

std::shared_ptr<FakeGrowthDerivedObservation> MakePreparedGrowth(
    double mean_laa, double sd_laa) {
  auto growth = std::make_shared<FakeGrowthDerivedObservation>();
  growth->Initialize(1, 1, 1);
  growth->products.MeanLAA(0, 0, 0) = mean_laa;
  growth->products.SdLAA(0, 0, 0) = sd_laa;
  growth->PrepareGrowthProducts();
  return growth;
}

void ConfigureAdapter(
    fims_popdy::VonBertalanffySchnuteGrowthModelAdapter<double>& adapter,
    double mean_length_young, double mean_length_old, double growth_coefficient,
    double reference_age_for_length_young, double reference_age_for_length_old,
    double length_weight_a, double length_weight_b,
    double length_at_age_sd_at_reference_age_young,
    double length_at_age_sd_at_reference_age_old) {
  adapter.MeanLengthYoungVector().resize(1);
  adapter.MeanLengthOldVector().resize(1);
  adapter.GrowthCoefficientVector().resize(1);
  adapter.ReferenceAgeForLengthYoungVector().resize(1);
  adapter.ReferenceAgeForLengthOldVector().resize(1);
  adapter.LengthWeightAVector().resize(1);
  adapter.LengthWeightBVector().resize(1);
  adapter.LengthAtAgeSdAtRefAgesVector().resize(2);

  adapter.MeanLengthYoungVector()[0] = fims_math::log(mean_length_young);
  adapter.MeanLengthOldVector()[0] = fims_math::log(mean_length_old);
  adapter.GrowthCoefficientVector()[0] = fims_math::log(growth_coefficient);
  adapter.ReferenceAgeForLengthYoungVector()[0] =
      reference_age_for_length_young;
  adapter.ReferenceAgeForLengthOldVector()[0] = reference_age_for_length_old;
  adapter.LengthWeightAVector()[0] = fims_math::log(length_weight_a);
  adapter.LengthWeightBVector()[0] = fims_math::log(length_weight_b);
  adapter.LengthAtAgeSdAtRefAgesVector()[0] =
      fims_math::log(length_at_age_sd_at_reference_age_young);
  adapter.LengthAtAgeSdAtRefAgesVector()[1] =
      fims_math::log(length_at_age_sd_at_reference_age_old);
}

// Normal CDF written out here so the expected values do not reuse the
// provider's own probabilities.
double NormalCdf(double x, double mean, double sd) {
  return 0.5 * (1.0 + std::erf((x - mean) / (sd * std::sqrt(2.0))));
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
     PrepareSizeProductsWarnsWhenTerminalPlusGroupBinExceedsThreshold) {
  fims::FIMSLog::fims_log->clear();

  const auto growth = MakePreparedGrowth(10.0, 0.2);
  const fims_popdy::SizeGrid grid =
      fims_popdy::SizeGridBuilder::BuildFromEdges({0.0, 1.0, 2.0, 3.0});

  fims_popdy::GrowthDerivedSizeProvider<double> provider(growth);
  provider.SetPopulationSizeGrid(&grid);
  provider.SetPopulationDimensions(1, 1);

  ASSERT_NO_THROW(provider.PrepareSizeProducts());

  EXPECT_GT(fims::FIMSLog::fims_log->get_warning_count(), 0u);
  EXPECT_NE(fims::FIMSLog::fims_log->get_warnings().find(
                "terminal biological plus-group bin"),
            std::string::npos);

  const std::size_t warning_count_after_first_prepare =
      fims::FIMSLog::fims_log->get_warning_count();

  ASSERT_NO_THROW(provider.PrepareSizeProducts());

  EXPECT_EQ(fims::FIMSLog::fims_log->get_warning_count(),
            warning_count_after_first_prepare);

  fims::FIMSLog::fims_log->clear();
}

TEST(GrowthDerivedSizeProvider,
     PrepareSizeProductsHandlesDifferentGrowthVariability) {
  auto growth = std::make_shared<
      fims_popdy::VonBertalanffySchnuteGrowthModelAdapter<double>>();
  ConfigureAdapter(*growth, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 10.0,
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

    for (std::size_t size_bin_index = 0; size_bin_index < size_grid.n_bins;
         ++size_bin_index) {
      const double prob = provider.ProbSize(0, age_index, size_bin_index);
      EXPECT_TRUE(std::isfinite(prob));
      EXPECT_GE(prob, 0.0);
      row_sum += prob;
    }

    EXPECT_NEAR(row_sum, 1.0, 1e-5);
  }
}

TEST(GrowthDerivedSizeProvider,
     PrepareSizeProductsShiftMassWithDifferentMeanGrowth) {
  const auto lower_growth = MakePreparedGrowth(1.5, 0.4);
  const auto higher_growth = MakePreparedGrowth(3.5, 0.4);
  const fims_popdy::SizeGrid grid = fims_popdy::SizeGridBuilder::BuildFromEdges(
      {0.0, 1.0, 2.0, 3.0, 4.0, 5.0});

  fims_popdy::GrowthDerivedSizeProvider<double> lower_provider(lower_growth);
  lower_provider.SetPopulationSizeGrid(&grid);
  lower_provider.SetPopulationDimensions(1, 1);
  lower_provider.PrepareSizeProducts();

  fims_popdy::GrowthDerivedSizeProvider<double> higher_provider(higher_growth);
  higher_provider.SetPopulationSizeGrid(&grid);
  higher_provider.SetPopulationDimensions(1, 1);
  higher_provider.PrepareSizeProducts();

  double lower_expected_size = 0.0;
  double higher_expected_size = 0.0;
  double lower_upper_tail = 0.0;
  double higher_upper_tail = 0.0;

  for (std::size_t size_bin_index = 0; size_bin_index < grid.n_bins;
       ++size_bin_index) {
    const double lower_prob = lower_provider.ProbSize(0, 0, size_bin_index);
    const double higher_prob = higher_provider.ProbSize(0, 0, size_bin_index);

    lower_expected_size += lower_prob * grid.centers[size_bin_index];
    higher_expected_size += higher_prob * grid.centers[size_bin_index];

    if (grid.centers[size_bin_index] >= 3.5) {
      lower_upper_tail += lower_prob;
      higher_upper_tail += higher_prob;
    }
  }

  EXPECT_GT(higher_expected_size, lower_expected_size);
  EXPECT_GT(higher_upper_tail, lower_upper_tail);
}

TEST(GrowthDerivedSizeProvider,
     PrepareSizeProductsShiftMassWithDifferentPreparedGrowthAdapters) {
  auto lower_growth = std::make_shared<
      fims_popdy::VonBertalanffySchnuteGrowthModelAdapter<double>>();
  ConfigureAdapter(*lower_growth, 225.0, 575.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0,
                   28.0, 73.0);
  lower_growth->SetAgeOffset(1.0);
  lower_growth->Initialize(1, 1, 1);
  ASSERT_NO_THROW(lower_growth->PrepareGrowthProducts());

  auto higher_growth = std::make_shared<
      fims_popdy::VonBertalanffySchnuteGrowthModelAdapter<double>>();
  ConfigureAdapter(*higher_growth, 325.0, 825.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0,
                   28.0, 73.0);
  higher_growth->SetAgeOffset(1.0);
  higher_growth->Initialize(1, 1, 1);
  ASSERT_NO_THROW(higher_growth->PrepareGrowthProducts());

  const fims_popdy::SizeGrid grid = fims_popdy::SizeGridBuilder::BuildFromEdges(
      {0.0, 100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0, 900.0});

  fims_popdy::GrowthDerivedSizeProvider<double> lower_provider(lower_growth);
  lower_provider.SetPopulationSizeGrid(&grid);
  lower_provider.SetPopulationDimensions(1, 1);
  ASSERT_NO_THROW(lower_provider.PrepareSizeProducts());

  fims_popdy::GrowthDerivedSizeProvider<double> higher_provider(higher_growth);
  higher_provider.SetPopulationSizeGrid(&grid);
  higher_provider.SetPopulationDimensions(1, 1);
  ASSERT_NO_THROW(higher_provider.PrepareSizeProducts());

  double lower_expected_size = 0.0;
  double higher_expected_size = 0.0;

  for (std::size_t size_bin_index = 0; size_bin_index < grid.n_bins;
       ++size_bin_index) {
    const double lower_prob = lower_provider.ProbSize(0, 0, size_bin_index);
    const double higher_prob = higher_provider.ProbSize(0, 0, size_bin_index);

    lower_expected_size += lower_prob * grid.centers[size_bin_index];
    higher_expected_size += higher_prob * grid.centers[size_bin_index];
  }

  EXPECT_GT(higher_expected_size, lower_expected_size);
}

TEST(GrowthDerivedSizeProvider,
     MeanWeightAtAgeAveragesWeightAtBinCentersOverProbSize) {
  // The fake growth object has weight equal to length.
  const auto growth = MakePreparedGrowth(2.5, 0.4);
  const fims_popdy::SizeGrid grid =
      fims_popdy::SizeGridBuilder::BuildFromEdges({0.0, 1.0, 2.0, 3.0, 4.0});

  fims_popdy::GrowthDerivedSizeProvider<double> provider(growth);
  provider.SetPopulationSizeGrid(&grid);
  provider.SetPopulationDimensions(1, 1);
  provider.PrepareSizeProducts();

  // The first and last bins hold the lower and upper tails.
  const double p0 = NormalCdf(1.0, 2.5, 0.4);
  const double p1 = NormalCdf(2.0, 2.5, 0.4) - NormalCdf(1.0, 2.5, 0.4);
  const double p2 = NormalCdf(3.0, 2.5, 0.4) - NormalCdf(2.0, 2.5, 0.4);
  const double p3 = 1.0 - NormalCdf(3.0, 2.5, 0.4);
  const double expected = p0 * 0.5 + p1 * 1.5 + p2 * 2.5 + p3 * 3.5;

  // The provider floors the SD with a smooth ad_max, which raises an SD of
  // 0.4 by about 6e-6 and the mean weight by about 2e-8, well below the
  // 9e-5 gap to weight at the mean length.
  EXPECT_NEAR(provider.MeanWeightAtAge(0, 0), expected, 1e-7);
}

TEST(GrowthDerivedSizeProvider,
     MeanWeightAtAgeWeightsUpperTailAtTerminalBinCenter) {
  const auto growth = MakePreparedGrowth(10.0, 0.2);
  const fims_popdy::SizeGrid grid =
      fims_popdy::SizeGridBuilder::BuildFromEdges({0.0, 1.0, 2.0, 3.0});

  fims_popdy::GrowthDerivedSizeProvider<double> provider(growth);
  provider.SetPopulationSizeGrid(&grid);
  provider.SetPopulationDimensions(1, 1);
  provider.PrepareSizeProducts();

  // All fish are above the grid, so the mean weight is the weight at the
  // terminal bin center rather than at the mean length of 10.
  EXPECT_NEAR(provider.MeanWeightAtAge(0, 0), 2.5, 1e-8);
}

TEST(GrowthDerivedSizeProvider,
     MeanWeightAtAgeMatchesNormalThirdMomentOnWideGrid) {
  auto growth = std::make_shared<
      fims_popdy::VonBertalanffySchnuteGrowthModelAdapter<double>>();
  ConfigureAdapter(*growth, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0,
                   73.0);
  growth->SetAgeOffset(1.0);
  growth->Initialize(1, 12, 1);
  ASSERT_NO_THROW(growth->PrepareGrowthProducts());

  // 1-unit bins covering well beyond every age's length distribution.
  const fims_popdy::SizeGrid grid =
      fims_popdy::SizeGridBuilder::BuildRegularGrid(0.0, 1500.0, 1.0);

  fims_popdy::GrowthDerivedSizeProvider<double> provider(growth);
  provider.SetPopulationSizeGrid(&grid);
  provider.SetPopulationDimensions(1, 12);
  ASSERT_NO_THROW(provider.PrepareSizeProducts());

  const fims_popdy::GrowthProducts<double>* products =
      growth->TryGetPreparedGrowthProducts();
  ASSERT_NE(products, nullptr);

  for (std::size_t age_index = 0; age_index < 12; ++age_index) {
    const double mean = products->MeanLAA(0, age_index, 0);
    const double sd = products->SdLAA(0, age_index, 0);
    // For normal lengths and b = 3, E[a L^3] = a (mu^3 + 3 mu sd^2), which is
    // larger than weight at the mean length, a mu^3.
    const double expected =
        2.5e-11 * (std::pow(mean, 3.0) + 3.0 * mean * sd * sd);
    const double observed = provider.MeanWeightAtAge(0, age_index);

    EXPECT_NEAR(observed / expected, 1.0, 1e-4);
    EXPECT_GT(observed, products->MeanWAA(0, age_index, 0));
  }
}

TEST(GrowthDerivedSizeProvider, MeanWeightAtAgeRequiresPreparedSizeProducts) {
  const auto growth = MakePreparedGrowth(2.5, 0.4);
  const fims_popdy::SizeGrid grid =
      fims_popdy::SizeGridBuilder::BuildFromEdges({0.0, 1.0, 2.0, 3.0, 4.0});

  fims_popdy::GrowthDerivedSizeProvider<double> provider(growth);
  provider.SetPopulationSizeGrid(&grid);
  provider.SetPopulationDimensions(1, 1);

  EXPECT_THROW(provider.MeanWeightAtAge(0, 0), std::runtime_error);

  provider.PrepareSizeProducts();
  EXPECT_THROW(provider.MeanWeightAtAge(1, 0), std::out_of_range);
}

}  // namespace
