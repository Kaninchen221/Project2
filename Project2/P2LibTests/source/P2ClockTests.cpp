#pragma once

#include "P2Clock.hpp"
#include "P2Time.hpp"

#include <gtest/gtest.h>

namespace P2::tests
{

    class ClockTests : public ::testing::Test
    {
    protected:

        ClockTests()
        {
        }

        ~ClockTests() override
        {
        }

        void SetUp() override
        {
        }

        void TearDown() override
        {
        }

        Clock clock;
    };

    TEST_F(ClockTests, StartTest)
    {
        Time::NumericType greaterThan = 0u;
        Time::NumericType lessThan = 1000u;

		clock.start();

		while (clock.getElapsedTime().getAsMicroseconds() == 0) {}

        Time time = clock.getElapsedTime();
        Time::NumericType microseconds = time.getAsMicroseconds();

        ASSERT_GT(microseconds, greaterThan);
        ASSERT_LT(microseconds, lessThan);
    }

    TEST_F(ClockTests, RestartTest)
    {
		clock.start();

		Time::NumericType greaterThan = 0u;
		Time::NumericType lessThan = 1000u;

        while (clock.getElapsedTime().getAsMicroseconds() == 0) {}

        Time time = clock.restart();
        Time::NumericType microseconds = time.getAsMicroseconds();

        ASSERT_GT(microseconds, greaterThan);
        ASSERT_LT(microseconds, lessThan);
    }

}