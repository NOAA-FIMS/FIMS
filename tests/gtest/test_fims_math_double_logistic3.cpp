#include "common/fims_math.hpp"
#include "gtest/gtest.h"

namespace {

TEST(DoubleLogistic3, UseMultipleInputValues) {
  double p1_value[3] = {2.0, 2.0, 1.0};
  double p2_value[3] = {4.0, 4.0, 4.0};
  double p3_value[3] = {2.5, 2.5, 2.5};
  double x_value[3] = {9.0, 5.0, 8.0};
  double expect_value[3] = {0.9350173, 0.2064477, 0.7124833};

  for (size_t i = 0; i < 3; i++) {
    EXPECT_NEAR(fims_math::double_logistic3(p1_value[i], p2_value[i],
                                            p3_value[i], x_value[i]),
                expect_value[i], 0.0001);
  }
}

}  // namespace
