#include "gtest/gtest.h"
#include "population_dynamics/selectivity/functors/double_logistic3.hpp"

namespace {

TEST(DoubleLogistic3Selectivity_Evaluate, HandlesCorrectInput) {
  fims_popdy::DoubleLogistic3Selectivity<double> fishery_selectivity;
  fishery_selectivity.p1.resize(1);
  fishery_selectivity.p2.resize(1);
  fishery_selectivity.p3.resize(1);
  fishery_selectivity.p1[0] = 2.0;
  fishery_selectivity.p2[0] = 4.0;
  fishery_selectivity.p3[0] = 2.5;

  EXPECT_NEAR(fishery_selectivity.evaluate(9.0), 0.9350173, 0.0001);
}

TEST(DoubleLogistic3Selectivity_Evaluate, HandlesThreeTimeSteps) {
  fims_popdy::DoubleLogistic3Selectivity<double> fishery_selectivity;
  fishery_selectivity.p1.resize(3);
  fishery_selectivity.p2.resize(1);
  fishery_selectivity.p3.resize(1);
  fishery_selectivity.p1[0] = 2.0;
  fishery_selectivity.p1[1] = 1.0;
  fishery_selectivity.p1[2] = 3.0;
  fishery_selectivity.p2[0] = 4.0;
  fishery_selectivity.p3[0] = 2.5;

  double expected_fishery[3] = {0.9350173, 0.3954430, 0.9560853};
  for (size_t pos = 0; pos < 3; ++pos) {
    EXPECT_NEAR(fishery_selectivity.evaluate(9.0, pos),
                expected_fishery[pos], 0.0001);
  }
}

}  // namespace
