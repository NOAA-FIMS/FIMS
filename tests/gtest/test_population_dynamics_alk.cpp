#include <cstddef>
#include <memory>

#include "gtest/gtest.h"

#include "common/fims_math.hpp"
#include "common/fims_vector.hpp"
#include "population_dynamics/alk/functors/fixed_matrix_alk.hpp"
#include "population_dynamics/alk/functors/growth_derived_alk.hpp"
#include "population_dynamics/alk/functors/alk_runtime.hpp"
#include "population_dynamics/fleet/fleet.hpp"
#include "population_dynamics/growth/growth_model_adapter.hpp"
#include "population_dynamics/growth/growth_products.hpp"

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

  const double evaluate(const double& age) const override {
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
 * @brief Build a fleet with explicit test length bins.
 */
std::shared_ptr<fims_popdy::Fleet<double>> MakeFleet() {
  auto fleet = std::make_shared<fims_popdy::Fleet<double>>();
  fleet->n_years = 1;
  fleet->n_ages = 3;
  fleet->n_lengths = 4;
  fleet->lengths.resize(4);
  fleet->lengths[0] = 200.0;
  fleet->lengths[1] = 300.0;
  fleet->lengths[2] = 400.0;
  fleet->lengths[3] = 500.0;
  return fleet;
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

TEST(RuntimeALK, BuildFleetALKFallsBackToFixedMatrixWhenGrowthDerivedPrepareFails) {
  auto population = std::make_shared<fims_popdy::Population<double>>();
  auto fleet = MakeFleet();
  FillFixedMatrix(fleet);

  auto growth = std::make_shared<FakeGrowthDerivedObservation>();
  growth->Initialize(1, fleet->n_ages, 2);

  population->growth = growth;

  std::shared_ptr<fims_popdy::ALKBase<double>> alk =
      fims_popdy::BuildFleetALK<double>(population, fleet);

  ASSERT_NE(alk, nullptr);
  EXPECT_NE(
      std::dynamic_pointer_cast<fims_popdy::FixedMatrixALK<double>>(alk),
      nullptr);

  fims::Vector<double> row;
  ASSERT_TRUE(alk->BuildALKRow(0, 1, row));
  ASSERT_EQ(row.size(), fleet->n_lengths);
  EXPECT_DOUBLE_EQ(row[0], 0.4);
  EXPECT_DOUBLE_EQ(row[1], 0.3);
  EXPECT_DOUBLE_EQ(row[2], 0.2);
  EXPECT_DOUBLE_EQ(row[3], 0.1);
}

TEST(RuntimeALK, BuildFleetALKUsesGrowthDerivedWhenPrepareSucceeds) {
  auto population = std::make_shared<fims_popdy::Population<double>>();
  auto fleet = MakeFleet();

  auto growth = std::make_shared<FakeGrowthDerivedObservation>();
  growth->Initialize(1, fleet->n_ages, 1);

  population->growth = growth;

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

TEST(RuntimeALK, EnsureFleetALKRebuildsToFixedMatrixWhenExistingGrowthDerivedPrepareFails) {
  auto population = std::make_shared<fims_popdy::Population<double>>();
  auto fleet = MakeFleet();
  FillFixedMatrix(fleet);

  auto growth = std::make_shared<FakeGrowthDerivedObservation>();
  growth->Initialize(1, fleet->n_ages, 2);

  population->growth = growth;

  fleet->alk = std::make_shared<fims_popdy::GrowthDerivedALK<double>>(fleet, growth);

  ASSERT_NE(fleet->alk, nullptr);
  EXPECT_NE(
      std::dynamic_pointer_cast<fims_popdy::GrowthDerivedALK<double>>(fleet->alk),
      nullptr);

  fims_popdy::EnsureFleetALK<double>(population, fleet);

  ASSERT_NE(fleet->alk, nullptr);
  EXPECT_NE(
      std::dynamic_pointer_cast<fims_popdy::FixedMatrixALK<double>>(fleet->alk),
      nullptr);

  fims::Vector<double> row;
  ASSERT_TRUE(fleet->alk->BuildALKRow(0, 1, row));
  ASSERT_EQ(row.size(), fleet->n_lengths);
  EXPECT_DOUBLE_EQ(row[0], 0.4);
  EXPECT_DOUBLE_EQ(row[1], 0.3);
  EXPECT_DOUBLE_EQ(row[2], 0.2);
  EXPECT_DOUBLE_EQ(row[3], 0.1);
}

TEST(GrowthDerivedALK, IsInactiveWithoutExplicitLengthBins) {
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

  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth);

  EXPECT_FALSE(alk.IsActive());
}

TEST(GrowthDerivedALK, PrepareForCurrentStateFailsForMultiSexProducts) {
  auto fleet = MakeFleet();
  auto growth = std::make_shared<FakeGrowthDerivedObservation>();

  growth->Initialize(1, fleet->n_ages, 2);

  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth);

  EXPECT_TRUE(alk.IsActive());
  EXPECT_FALSE(alk.PrepareForCurrentState());
}

TEST(GrowthDerivedALK, IsActiveDoesNotPrepareGrowthProducts) {
  auto fleet = MakeFleet();
  auto growth = std::make_shared<FakeGrowthDerivedObservation>();

  growth->Initialize(1, fleet->n_ages, 1);

  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth);

  EXPECT_TRUE(alk.IsActive());
  EXPECT_EQ(growth->prepare_calls, 0);
}

TEST(GrowthDerivedALK, BuildALKRowFailsBeforePrepareForCurrentState) {
  auto fleet = MakeFleet();
  auto growth = std::make_shared<FakeGrowthDerivedObservation>();

  growth->Initialize(1, fleet->n_ages, 1);

  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth);
  fims::Vector<double> row;

  ASSERT_TRUE(alk.IsActive());
  EXPECT_FALSE(alk.BuildALKRow(0, 1, row));
  EXPECT_EQ(growth->prepare_calls, 0);
}

TEST(GrowthDerivedALK, PrepareForCurrentStateEnablesRowBuilding) {
  auto fleet = MakeFleet();
  auto growth = std::make_shared<FakeGrowthDerivedObservation>();

  growth->Initialize(1, fleet->n_ages, 1);

  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth);
  fims::Vector<double> row;

  ASSERT_TRUE(alk.IsActive());
  ASSERT_TRUE(alk.PrepareForCurrentState());
  EXPECT_EQ(growth->prepare_calls, 1);
  ASSERT_TRUE(alk.BuildALKRow(0, 1, row));
  ASSERT_EQ(row.size(), fleet->n_lengths);
}

TEST(GrowthDerivedALK, BuildALKRowReturnsNormalizedRow) {
  auto fleet = MakeFleet();

  auto growth =
      std::make_shared<fims_popdy::VonBertalanffyGrowthModelAdapter<double>>();
  ConfigureAdapter(
      *growth, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0, 73.0);
  growth->SetAgeOffset(1.0);
  growth->Initialize(1, fleet->n_ages, 1);

  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth);
  fims::Vector<double> row;

  ASSERT_TRUE(alk.IsActive());
  ASSERT_TRUE(alk.PrepareForCurrentState());
  ASSERT_TRUE(alk.BuildALKRow(0, 1, row));
  ASSERT_EQ(row.size(), fleet->n_lengths);

  double row_sum = 0.0;
  for (std::size_t i = 0; i < row.size(); ++i) {
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

  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth);
  fims::Vector<double> row;

  ASSERT_TRUE(alk.IsActive());
  ASSERT_TRUE(alk.PrepareForCurrentState());
  EXPECT_FALSE(alk.BuildALKRow(0, fleet->n_ages, row));
}

}  // namespace