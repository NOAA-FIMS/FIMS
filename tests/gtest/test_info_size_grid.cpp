#include <memory>
#include <vector>

#include "gtest/gtest.h"

#include "common/def.hpp"
#include "common/information.hpp"
#include "test_stubs.hpp"

namespace {

std::shared_ptr<fims_popdy::Population<double>> MakePopulation() {
  auto population = std::make_shared<fims_popdy::Population<double>>();
  population->n_years = 1;
  population->n_ages = 1;
  population->n_fleets = 0;
  return population;
}

std::shared_ptr<fims_popdy::Fleet<double>> MakeFleetWithCenters(
    const std::vector<double>& centers) {
  auto fleet = std::make_shared<fims_popdy::Fleet<double>>();
  fleet->n_years = 1;
  fleet->n_ages = 1;
  fleet->n_lengths = centers.size();
  fleet->requires_age_length_mapping = true;
  fleet->lengths.resize(centers.size());

  for (std::size_t i = 0; i < centers.size(); ++i) {
    fleet->lengths[i] = centers[i];
  }

  return fleet;
}

TEST(InformationSizeGrid,
     EnsurePopulationSizeGridWarnsWhenBuiltInDefaultCreatesMoreThan200RegularBins) {
  fims::FIMSLog::fims_log->clear();

  fims_info::Information<double> info;
  bool valid_model = true;

  auto population = MakePopulation();
  auto fleet = std::make_shared<fims_popdy::Fleet<double>>();
  fleet->n_years = 1;
  fleet->n_ages = 1;
  fleet->n_lengths = 202;
  fleet->requires_age_length_mapping = true;
  fleet->lengths.resize(202);

  for (std::size_t i = 0; i < 202; ++i) {
    fleet->lengths[i] = static_cast<double>(i) + 0.5;
  }

  population->fleets.push_back(fleet);
  population->n_fleets = population->fleets.size();

  ASSERT_NO_THROW(info.EnsurePopulationSizeGrid(valid_model, population));

  EXPECT_TRUE(valid_model);
  EXPECT_TRUE(population->size_grid.IsConsistent());
  EXPECT_GT(fims::FIMSLog::fims_log->get_warning_count(), 0u);
  EXPECT_NE(
      fims::FIMSLog::fims_log->get_warnings().find("more than 200 regular bins"),
      std::string::npos);

  fims::FIMSLog::fims_log->clear();
}

TEST(InformationSizeGrid,
     EnsurePopulationSizeGridFailsWhenNoResolvableFleetGeometryIsAvailable) {
  fims::FIMSLog::fims_log->clear();

  fims_info::Information<double> info;
  bool valid_model = true;

  auto population = MakePopulation();

  const std::size_t error_count_before =
      fims::FIMSLog::fims_log->get_error_count();

  ASSERT_NO_THROW(info.EnsurePopulationSizeGrid(valid_model, population));

  EXPECT_FALSE(valid_model);
  EXPECT_GT(fims::FIMSLog::fims_log->get_error_count(), error_count_before);
  EXPECT_NE(
      fims::FIMSLog::fims_log->get_errors().find(
          "No resolvable fleet observation-bin geometry was available"),
      std::string::npos);

  fims::FIMSLog::fims_log->clear();
}

TEST(InformationSizeGrid,
     EnsurePopulationSizeGridFailsWhenBuiltInDefaultIsCoarserThanFleetBins) {
  fims::FIMSLog::fims_log->clear();

  fims_info::Information<double> info;
  bool valid_model = true;

  auto population = MakePopulation();
  auto fleet = MakeFleetWithCenters({0.0, 0.5, 1.0});
  population->fleets.push_back(fleet);
  population->n_fleets = population->fleets.size();

  const std::size_t error_count_before =
      fims::FIMSLog::fims_log->get_error_count();

  ASSERT_NO_THROW(info.EnsurePopulationSizeGrid(valid_model, population));

  EXPECT_FALSE(valid_model);
  EXPECT_GT(fims::FIMSLog::fims_log->get_error_count(), error_count_before);
  EXPECT_NE(
      fims::FIMSLog::fims_log->get_errors().find(
          "coarser than an overlapping active fleet observation bin"),
      std::string::npos);

  fims::FIMSLog::fims_log->clear();
}

TEST(InformationSizeGrid,
     EnsurePopulationSizeGridFailsWhenUserOverrideIsCoarserThanFleetBins) {
  fims::FIMSLog::fims_log->clear();

  fims_info::Information<double> info;
  bool valid_model = true;

  auto population = MakePopulation();
  auto fleet = MakeFleetWithCenters({0.5, 1.5, 2.5, 3.5});
  population->fleets.push_back(fleet);
  population->n_fleets = population->fleets.size();

  population->size_grid.n_bins = 2;
  population->size_grid.edges = fims::Vector<double>{0.0, 2.0, 4.0};
  population->size_grid.centers = fims::Vector<double>{1.0, 3.0};

  const std::size_t error_count_before =
      fims::FIMSLog::fims_log->get_error_count();

  ASSERT_NO_THROW(info.EnsurePopulationSizeGrid(valid_model, population));

  EXPECT_FALSE(valid_model);
  EXPECT_GT(fims::FIMSLog::fims_log->get_error_count(), error_count_before);
  EXPECT_NE(
      fims::FIMSLog::fims_log->get_errors().find(
          "coarser than an overlapping active fleet observation bin"),
      std::string::npos);

  fims::FIMSLog::fims_log->clear();
}

TEST(InformationSizeGrid,
     EnsurePopulationSizeGridKeepsUserOverrideWhenItIsNotCoarserThanFleetBins) {
  fims::FIMSLog::fims_log->clear();

  fims_info::Information<double> info;
  bool valid_model = true;

  auto population = MakePopulation();
  auto fleet = MakeFleetWithCenters({0.5, 1.5, 2.5, 3.5});
  population->fleets.push_back(fleet);
  population->n_fleets = population->fleets.size();

  population->size_grid.n_bins = 4;
  population->size_grid.edges = fims::Vector<double>{0.0, 1.0, 2.0, 3.0, 4.0};
  population->size_grid.centers =
      fims::Vector<double>{0.5, 1.5, 2.5, 3.5};

  ASSERT_NO_THROW(info.EnsurePopulationSizeGrid(valid_model, population));

  EXPECT_TRUE(valid_model);
  EXPECT_TRUE(population->size_grid.IsConsistent());
  EXPECT_EQ(population->size_grid.edges.size(), 5u);
  EXPECT_DOUBLE_EQ(population->size_grid.edges[0], 0.0);
  EXPECT_DOUBLE_EQ(population->size_grid.edges[4], 4.0);

  fims::FIMSLog::fims_log->clear();
}

TEST(InformationSizeGrid,
     EnsurePopulationSizeGridFailsWhenUserSuppliesInvalidBiologicalGrid) {
  fims::FIMSLog::fims_log->clear();

  fims_info::Information<double> info;
  bool valid_model = true;

  auto population = MakePopulation();
  auto fleet = MakeFleetWithCenters({1.0, 2.0, 3.0});
  population->fleets.push_back(fleet);
  population->n_fleets = population->fleets.size();

  population->size_grid.n_bins = 2;

  const std::size_t error_count_before =
      fims::FIMSLog::fims_log->get_error_count();

  ASSERT_NO_THROW(info.EnsurePopulationSizeGrid(valid_model, population));

  EXPECT_FALSE(valid_model);
  EXPECT_GT(fims::FIMSLog::fims_log->get_error_count(), error_count_before);
  EXPECT_NE(
      fims::FIMSLog::fims_log->get_errors().find(
          "has an invalid biological size grid"),
      std::string::npos);

  fims::FIMSLog::fims_log->clear();
}

TEST(InformationSizeGrid,
     PrepareFleetObservationBinEdgesFailsWhenCentersDoNotMatchNLengths) {
  fims::FIMSLog::fims_log->clear();

  fims_info::Information<double> info;
  bool valid_model = true;

  auto population = MakePopulation();
  auto fleet = std::make_shared<fims_popdy::Fleet<double>>();
  fleet->n_years = 1;
  fleet->n_ages = 1;
  fleet->n_lengths = 3;
  fleet->requires_age_length_mapping = true;
  fleet->lengths = fims::Vector<double>{1.0, 2.0};
  population->fleets.push_back(fleet);
  population->n_fleets = population->fleets.size();

  const std::size_t error_count_before =
      fims::FIMSLog::fims_log->get_error_count();

  ASSERT_NO_THROW(info.EnsurePopulationSizeGrid(valid_model, population));

  EXPECT_FALSE(valid_model);
  EXPECT_GT(fims::FIMSLog::fims_log->get_error_count(), error_count_before);
  EXPECT_NE(
      fims::FIMSLog::fims_log->get_errors().find(
          "Fleet length_bin centers are not consistent with n_lengths"),
      std::string::npos);

  fims::FIMSLog::fims_log->clear();
}

}  // namespace
