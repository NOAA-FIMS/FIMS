#include "gtest/gtest.h"
#include "common/fims_math.hpp"

namespace
{

    // Test exp using multiple input values and types
    // Not worth to write many tests when testing thin database wrappers,
    // third-party libraries, or basic variable assignments.

    TEST(Exp, UseMultipleDoubleValues)
    {
        // Test exp using large negative input value
        EXPECT_EQ(fims_math::exp(-1000000.0), std::exp(-1000000.0));
        // Test exp using large positive input value
        EXPECT_EQ(fims_math::exp(1000000.0), std::exp(1000000.0));
        // Test exp using double value 0.0
        EXPECT_EQ(fims_math::exp(0.0), std::exp(0.0));
        // Test exp using double value 1.0
        EXPECT_EQ(fims_math::exp(1.0), std::exp(1.0));
        // Test exp using double value 3.0
        EXPECT_NEAR(fims_math::exp(3.0), 20.08554, 0.0001);
        // Test exp using double value -2.5
        EXPECT_NEAR(fims_math::exp(-2.5), 0.082085, 0.0001);
    }

    TEST(Exp, UseIntegerValues)
    {

        // Test exp using large positive integer value
        int input_value = 1000000;
        EXPECT_EQ(fims_math::exp<double>(input_value), std::exp(input_value));

        // Test exp using integer value 3
        // For fims_math::exp(3): the output value will be an integer if the input value is an integer
        // need to round the output value before using it as expected true value
        EXPECT_EQ(fims_math::exp(3), 20);
    }
}
