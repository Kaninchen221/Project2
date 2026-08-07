
#include <gtest/gtest.h>

/// Simple test
TEST(FirstTest, Test)
{
	EXPECT_TRUE(testing::AssertionSuccess() << "GTest is working!");
}