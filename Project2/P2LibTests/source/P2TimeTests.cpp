#pragma once

#include "P2Time.hpp"

#include <gtest/gtest.h>

namespace P2::tests
{
    using namespace std::chrono_literals;

    class TimeTests : public ::testing::Test
    {
    protected:

        const Time::Nanoseconds baseNanoseconds = 60000000000000ns;
        Time time;

        TimeTests()
            : time(baseNanoseconds)
        {
        }

        ~TimeTests() override {
        }

        void SetUp() override {
        }

        void TearDown() override {
        }

        static_assert(sizeof(Time) == 8);
    };

    TEST_F(TimeTests, AssignTest)
    {
        const Time::Nanoseconds expectedValue = 69404ns;
        time = expectedValue;
    
        const Time::Nanoseconds nanoseconds = time.getAsNanoseconds();
        EXPECT_EQ(expectedValue, nanoseconds);
    }
    
    TEST_F(TimeTests, NanosecondsTest)
    {
        const Time::Nanoseconds nanoseconds = time.getAsNanoseconds();
        EXPECT_EQ(nanoseconds, baseNanoseconds);
    }
    
    TEST_F(TimeTests, MicrosecondsTest)
    {
        Time::Microseconds microseconds = time.getAsMicroseconds();
    }
    
    TEST_F(TimeTests, MillisecondsTest)
    {
        Time::Milliseconds milliseconds = time.getAsMilliseconds();
    }
    
    TEST_F(TimeTests, SecondsTest)
    {
        Time::Seconds seconds = time.getAsSeconds();
    }
    
    TEST_F(TimeTests, MinutesTest)
    {
        Time::Minutes minutes = time.getAsMinutes();
    }
    
    TEST_F(TimeTests, HoursTest)
    {
        Time::Hours hours = time.getAsHours();
    }
    
    TEST_F(TimeTests, FromNanoseconds)
    {
        const auto expectedNanoseconds = 100ns;
        const Time actualTime = Time::FromNanoseconds(expectedNanoseconds);
        const auto actualNanoseconds = actualTime.getAsNanoseconds();
    
        ASSERT_EQ(expectedNanoseconds, actualNanoseconds);
    }
    
    TEST_F(TimeTests, FromMicroseconds)
    {
        const auto expectedMicroseconds = 100us;
        const Time actualTime = Time::FromMicroseconds(expectedMicroseconds);
        const auto actualMicroseconds = actualTime.getAsMicroseconds();
    
        ASSERT_EQ(expectedMicroseconds, actualMicroseconds);
    }
    
    TEST_F(TimeTests, FromMilliseconds)
    {
        const auto expectedMilliseconds = 100ms;
        const Time actualTime = Time::FromMilliseconds(expectedMilliseconds);
        const auto actualMilliseconds = actualTime.getAsMilliseconds();
    
        ASSERT_EQ(expectedMilliseconds, actualMilliseconds);
    }
    
    TEST_F(TimeTests, FromSeconds)
    {
        const auto expectedSeconds = 100s;
        const Time actualTime = Time::FromSeconds(expectedSeconds);
        const auto actualSeconds = actualTime.getAsSeconds();
    
        ASSERT_EQ(expectedSeconds, actualSeconds);
    }
    
    TEST_F(TimeTests, FromMinutes)
    {
        const auto expectedMinutes = 100min;
        const Time actualTime = Time::FromMinutes(expectedMinutes);
        const auto actualMinutes = actualTime.getAsMinutes();
    
        ASSERT_EQ(expectedMinutes, actualMinutes);
    }
    
    TEST_F(TimeTests, FromHours)
    {
        const auto expectedHours = 100h;
        const Time actualTime = Time::FromHours(expectedHours);
        const auto actualHours = actualTime.getAsHours();
    
        ASSERT_EQ(expectedHours, actualHours);
	}

	TEST_F(TimeTests, CompareOperator)
	{
        Time otherTime{ 2323ms };
        EXPECT_FALSE(time == otherTime);

		otherTime = time;
		EXPECT_TRUE(time == otherTime);
	}

	TEST_F(TimeTests, SubtractOperator)
	{
        time = Time::FromSeconds(1s);
        Time other = Time::FromSeconds(2s);

        Time actual = other - time;
        Time expected = Time::FromSeconds(1s);
        EXPECT_EQ(actual, expected);
	}

	TEST_F(TimeTests, SubtractAssignOperator)
	{
		time = Time::FromSeconds(1s);

        Time actual = Time::FromSeconds(2s);
        actual -= time;

		Time expected = Time::FromSeconds(1s);
        EXPECT_EQ(actual, expected);
	}

	TEST_F(TimeTests, AddOperator)
	{
		time = Time::FromSeconds(1s);
		Time other = Time::FromSeconds(2s);

		Time actual = other + time;
		Time expected = Time::FromSeconds(3s);
        EXPECT_EQ(actual, expected);
	}

	TEST_F(TimeTests, AddAssignOperator)
	{
		time = Time::FromSeconds(1s);

		Time actual = Time::FromSeconds(2s);
		actual += time;

		Time expected = Time::FromSeconds(3s);
        EXPECT_EQ(actual, expected);
	}

    TEST_F(TimeTests, From)
    {
        const auto Expected = Time(5ns);
        const auto Time = Time::From(5ns);

        EXPECT_EQ(Expected, Time);
    }
}