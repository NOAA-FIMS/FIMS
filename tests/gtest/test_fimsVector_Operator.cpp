#include "gtest/gtest.h"
#include "fims_vector.hpp"

namespace {

TEST(fimsVector_Operator, ReportsDetailedBoundsError) {
  fims::Vector<double> values(2, 0.0);
  values.set_variable_name("Recruitment.1.log_devs");

  try {
    (void)values[2];
    FAIL() << "Expected std::invalid_argument";
  } catch (const std::invalid_argument &error) {
    const std::string message = error.what();
    EXPECT_NE(message.find("fims::Vector out of bounds"), std::string::npos);
    EXPECT_NE(message.find("Recruitment.1.log_devs"), std::string::npos);
    EXPECT_NE(message.find("index 3"), std::string::npos);
    EXPECT_NE(message.find("size 2"), std::string::npos);
  }
}

TEST(fimsVector_Operator, ReportsDetailedConstBoundsError) {
  fims::Vector<double> mutable_values(4, 0.0);
  mutable_values.set_variable_name("Selectivity.9.slope");
  const fims::Vector<double> &values = mutable_values;

  try {
    (void)values[4];
    FAIL() << "Expected std::invalid_argument";
  } catch (const std::invalid_argument &error) {
    const std::string message = error.what();
    EXPECT_NE(message.find("fims::Vector out of bounds"), std::string::npos);
    EXPECT_NE(message.find("Selectivity.9.slope"), std::string::npos);
    EXPECT_NE(message.find("index 5"), std::string::npos);
    EXPECT_NE(message.find("size 4"), std::string::npos);
  }
}

}  // namespace
