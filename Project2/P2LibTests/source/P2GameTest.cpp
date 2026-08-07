
#include <gtest/gtest.h>

#include "P2Game.hpp"

namespace P2
{

	class GameFixture : public ::testing::Test
	{
	protected:

		Game game;

		void SetUp() override
		{
			game.initialize();
		}

		void TearDown() override
		{
			game.deinitialize();
		}

	};

	TEST_F(GameFixture, Pass)
	{

	}

}