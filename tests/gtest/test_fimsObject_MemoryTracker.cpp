#include "gtest/gtest.h"
#include "model_object.hpp"
#include "data_object.hpp"

TEST(FIMSMemoryTracker, DetectsLeakedObject) {
  fims::model_object::FIMSMemoryTracker::total_active_objects = 0;

  std::shared_ptr<fims::data_object::DataObject<double>> leak =
      std::make_shared<fims::data_object::DataObject<double>>(1);

  EXPECT_EQ(fims::model_object::FIMSMemoryTracker::total_active_objects, 1);

  leak.reset();  // release it

  EXPECT_EQ(fims::model_object::FIMSMemoryTracker::total_active_objects, 0);
}
