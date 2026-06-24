#include "gtest/gtest.h"
#include "model_object.hpp"
#include "data_object.hpp"

TEST(FIMSMemoryTracker, DetectsLeakedObject) {
  fims_model_object::FIMSMemoryTracker::total_active_objects = 0;
  fims_model_object::FIMSMemoryTracker::active_objects.clear();

  std::shared_ptr<fims_data_object::DataObject<double>> leak =
      std::make_shared<fims_data_object::DataObject<double>>(1);

  EXPECT_EQ(fims_model_object::FIMSMemoryTracker::total_active_objects, 1);

  leak.reset();  // release it

  EXPECT_EQ(fims_model_object::FIMSMemoryTracker::total_active_objects, 0);
}

