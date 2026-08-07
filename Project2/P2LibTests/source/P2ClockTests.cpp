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

    using namespace std::chrono_literals;

    TEST_F(ClockTests, StartTest)
    {
        const auto greaterThan = 0us;
        const auto lessThan = 1000us;

		clock.start();

		while (clock.getElapsedTime().getAsMicroseconds() == 0us) {}

        const auto time = clock.getElapsedTime();
        const auto microseconds = time.getAsMicroseconds();

        ASSERT_GT(microseconds, greaterThan);
        ASSERT_LT(microseconds, lessThan);
    }

    TEST_F(ClockTests, RestartTest)
    {
        const auto greaterThan = 0us;
        const auto lessThan = 1000us;

		clock.start();

        while (clock.getElapsedTime().getAsMicroseconds() == 0us) {}

        const auto time = clock.restart();
        const auto microseconds = time.getAsMicroseconds();

        ASSERT_GT(microseconds, greaterThan);
        ASSERT_LT(microseconds, lessThan);
    }

}