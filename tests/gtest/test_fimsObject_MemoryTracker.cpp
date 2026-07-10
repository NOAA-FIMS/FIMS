#include "gtest/gtest.h"
#include "model_object.hpp"
#include "variable_object.hpp"

TEST(FIMSMemoryTracker, DetectsLeakedObject) {
  fims_model_object::FIMSMemoryTracker::total_active_objects = 0;

  std::shared_ptr<fims_variable_object::VariableObject<double>> leak =
      std::make_shared<fims_variable_object::VariableObject<double>>(1);

  EXPECT_EQ(fims_model_object::FIMSMemoryTracker::total_active_objects, 1);

  leak.reset();  // release it

  EXPECT_EQ(fims_model_object::FIMSMemoryTracker::total_active_objects, 0);
}

