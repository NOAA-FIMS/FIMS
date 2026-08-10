// Tests for the native R/C++ interface registries.

#include "gtest/gtest.h"

#include "interface/call/fleet_registry.hpp"
#include "interface/call/growth_registry.hpp"
#include "interface/call/maturity_registry.hpp"
#include "interface/call/population_registry.hpp"
#include "interface/call/recruitment_registry.hpp"
#include "interface/call/selectivity_registry.hpp"
#include "test_stubs.hpp"

namespace {

class NativeRegistries : public ::testing::Test {
 protected:
  void SetUp() override {
    information = fims_info::Information<double>::GetInstance();
    information->Clear();
    NativeFleetRegistry<double>::GetInstance()->Clear();
    NativeGrowthRegistry<double>::GetInstance()->Clear();
    NativeMaturityRegistry<double>::GetInstance()->Clear();
    NativePopulationRegistry<double>::GetInstance()->Clear();
    NativeRecruitmentRegistry<double>::GetInstance()->Clear();
    NativeSelectivityRegistry<double>::GetInstance()->Clear();
  }

  void TearDown() override { SetUp(); }

  std::shared_ptr<fims_info::Information<double>> information;
};

TEST_F(NativeRegistries, ShareInformationSingleton) {
  EXPECT_EQ(NativeFleetRegistry<double>::GetInstance()->GetInformation(),
            information);
  EXPECT_EQ(NativeGrowthRegistry<double>::GetInstance()->GetInformation(),
            information);
  EXPECT_EQ(NativeMaturityRegistry<double>::GetInstance()->GetInformation(),
            information);
  EXPECT_EQ(NativePopulationRegistry<double>::GetInstance()->GetInformation(),
            information);
  EXPECT_EQ(NativeRecruitmentRegistry<double>::GetInstance()->GetInformation(),
            information);
  EXPECT_EQ(NativeSelectivityRegistry<double>::GetInstance()->GetInformation(),
            information);
}

TEST_F(NativeRegistries, CreateAndRetrieveBackendObjects) {
  auto fleet = NativeFleetRegistry<double>::GetInstance()->CreateFleet(11);
  auto growth =
      NativeGrowthRegistry<double>::GetInstance()->CreateEWAAGrowth(12);
  auto maturity =
      NativeMaturityRegistry<double>::GetInstance()->CreateLogisticMaturity(13);
  auto population =
      NativePopulationRegistry<double>::GetInstance()->CreatePopulation(14);
  auto recruitment = NativeRecruitmentRegistry<double>::GetInstance()
                         ->CreateBevertonHoltRecruitment(15);
  auto selectivity = NativeSelectivityRegistry<double>::GetInstance()
                         ->CreateLogisticSelectivity(16);
  auto double_selectivity = NativeSelectivityRegistry<double>::GetInstance()
                                ->CreateDoubleLogisticSelectivity(17);

  EXPECT_EQ(fleet, NativeFleetRegistry<double>::GetInstance()->GetFleet(11));
  EXPECT_EQ(growth,
            NativeGrowthRegistry<double>::GetInstance()->GetEWAAGrowth(12));
  EXPECT_EQ(
      maturity,
      NativeMaturityRegistry<double>::GetInstance()->GetLogisticMaturity(13));
  EXPECT_EQ(population,
            NativePopulationRegistry<double>::GetInstance()->GetPopulation(14));
  EXPECT_EQ(recruitment, NativeRecruitmentRegistry<double>::GetInstance()
                             ->GetBevertonHoltRecruitment(15));
  EXPECT_EQ(selectivity, NativeSelectivityRegistry<double>::GetInstance()
                             ->GetLogisticSelectivity(16));
  EXPECT_EQ(double_selectivity, NativeSelectivityRegistry<double>::GetInstance()
                                    ->GetDoubleLogisticSelectivity(17));

  EXPECT_EQ(information->fleets.at(11), fleet);
  EXPECT_EQ(information->growth_models.at(12), growth);
  EXPECT_EQ(information->maturity_models.at(13), maturity);
  EXPECT_EQ(information->populations.at(14), population);
  EXPECT_EQ(information->recruitment_models.at(15), recruitment);
  EXPECT_EQ(information->selectivity_models.at(16), selectivity);
  EXPECT_EQ(information->selectivity_models.at(17), double_selectivity);
}

TEST_F(NativeRegistries, MissingIdsReturnNull) {
  EXPECT_EQ(NativeFleetRegistry<double>::GetInstance()->GetFleet(999), nullptr);
  EXPECT_EQ(NativeGrowthRegistry<double>::GetInstance()->GetEWAAGrowth(999),
            nullptr);
  EXPECT_EQ(
      NativeMaturityRegistry<double>::GetInstance()->GetLogisticMaturity(999),
      nullptr);
  EXPECT_EQ(NativePopulationRegistry<double>::GetInstance()->GetPopulation(999),
            nullptr);
  EXPECT_EQ(NativeRecruitmentRegistry<double>::GetInstance()
                ->GetBevertonHoltRecruitment(999),
            nullptr);
  EXPECT_EQ(
      NativeSelectivityRegistry<double>::GetInstance()->GetLogisticSelectivity(
          999),
      nullptr);
  EXPECT_EQ(NativeSelectivityRegistry<double>::GetInstance()
                ->GetDoubleLogisticSelectivity(999),
            nullptr);
}

TEST_F(NativeRegistries, ClearReleasesRegistryLookups) {
  auto fleet_registry = NativeFleetRegistry<double>::GetInstance();
  auto selectivity_registry = NativeSelectivityRegistry<double>::GetInstance();
  fleet_registry->CreateFleet(21);
  selectivity_registry->CreateLogisticSelectivity(22);
  selectivity_registry->CreateDoubleLogisticSelectivity(23);

  fleet_registry->Clear();
  selectivity_registry->Clear();

  EXPECT_EQ(fleet_registry->GetFleet(21), nullptr);
  EXPECT_EQ(selectivity_registry->GetLogisticSelectivity(22), nullptr);
  EXPECT_EQ(selectivity_registry->GetDoubleLogisticSelectivity(23), nullptr);
}

TEST_F(NativeRegistries, RecruitmentCreatesLogDevsProcess) {
  auto process =
      NativeRecruitmentRegistry<double>::GetInstance()->CreateLogDevsProcess();
  ASSERT_NE(process, nullptr);
  process->log_recruit_devs.resize(2);
  process->log_recruit_devs[0] = -0.1;
  process->log_recruit_devs[1] = 0.2;
  EXPECT_DOUBLE_EQ(process->log_recruit_devs[0], -0.1);
  EXPECT_DOUBLE_EQ(process->log_recruit_devs[1], 0.2);
}

}  // namespace
