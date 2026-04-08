#include <memory>

#include "gtest/gtest.h"

#include "common/fims_math.hpp"
#include "common/fims_vector.hpp"
#include "population_dynamics/alk/functors/fixed_matrix_alk.hpp"
#include "population_dynamics/alk/functors/growth_derived_alk.hpp"
#include "population_dynamics/fleet/fleet.hpp"
#include "population_dynamics/growth/growth_model_adapter.hpp"
#include "population_dynamics/growth/growth_products.hpp"

namespace {

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
  const auto& products = growth->GetProductsForReporting();

  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth, &products);

  EXPECT_FALSE(alk.IsActive());
}

TEST(GrowthDerivedALK, IsInactiveForMultiSexProducts) {
  auto fleet = MakeFleet();

  auto growth =
      std::make_shared<fims_popdy::VonBertalanffyGrowthModelAdapter<double>>();
  ConfigureAdapter(
      *growth, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0, 73.0);
  growth->SetAgeOffset(1.0);
  growth->Initialize(1, fleet->n_ages, 1);
  const auto& products = growth->GetProductsForReporting();

  fims_popdy::GrowthProducts<double> multi_sex_products = products;
  multi_sex_products.n_sexes = 2;

  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth, &multi_sex_products);

  EXPECT_FALSE(alk.IsActive());
}

TEST(GrowthDerivedALK, BuildALKRowReturnsNormalizedRow) {
  auto fleet = MakeFleet();

  auto growth =
      std::make_shared<fims_popdy::VonBertalanffyGrowthModelAdapter<double>>();
  ConfigureAdapter(
      *growth, 275.0, 725.0, 0.18, 1.0, 12.0, 2.5e-11, 3.0, 28.0, 73.0);
  growth->SetAgeOffset(1.0);
  growth->Initialize(1, fleet->n_ages, 1);
  const auto& products = growth->GetProductsForReporting();

  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth, &products);
  fims::Vector<double> row;

  ASSERT_TRUE(alk.IsActive());
  ASSERT_TRUE(alk.BuildALKRow(0, 1, row));
  ASSERT_EQ(row.size(), fleet->n_lengths);

  double row_sum = 0.0;
  for (size_t i = 0; i < row.size(); ++i) {
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
  const auto& products = growth->GetProductsForReporting();

  fims_popdy::GrowthDerivedALK<double> alk(fleet, growth, &products);
  fims::Vector<double> row;

  EXPECT_FALSE(alk.BuildALKRow(0, fleet->n_ages, row));
}

}  // namespace
