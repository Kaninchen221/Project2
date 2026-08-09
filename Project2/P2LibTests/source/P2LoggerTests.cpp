#pragma once

#include <gtest/gtest.h>

#include "P2Logger.hpp"

namespace P2::tests
{

	class ConsoleLoggerTests : public ::testing::Test
	{
	protected:

		static inline ConsoleLogger logger = ConsoleLogger::CreateOrGet("LoggerTests");

		static_assert(std::is_base_of_v<Logger, ConsoleLogger>);

	};

	TEST_F(ConsoleLoggerTests, Create)
	{
		ASSERT_TRUE(logger);
	}

	TEST_F(ConsoleLoggerTests, TurnOffOn)
	{
		auto level = logger->level();
		ASSERT_EQ(level, spdlog::level::info);

		logger.turnOff();
		ASSERT_EQ(logger->level(), spdlog::level::off);

		logger.turnOn();
		ASSERT_EQ(logger->level(), level);
	}
}