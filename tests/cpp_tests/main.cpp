#include <gtest/gtest.h>

// Example test case
TEST(SampleTest, Positive) {
    EXPECT_EQ(1 + 1, 2);
}

TEST(SampleTest, Negative) {
    EXPECT_NE(2 + 2, 5);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
