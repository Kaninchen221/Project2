
#include <gtest/gtest.h>

#include "P2Game.hpp"
#include <future>

namespace P2
{

	class GameFixture : public ::testing::Test
	{
	protected:

		Game game;

		void SetUp() override
		{
			game.initialize();

			auto& world = game.getWorld();
			ASSERT_TRUE(world.getResource<sf::RenderWindow>());
		}

		void TearDown() override
		{
			game.deinitialize();
		}

	};

	//TEST_F(GameFixture, Loop)
	//{
	//	using namespace std::chrono_literals;

	//	auto requestClose = std::async([&game = game]()
	//	{
	//		std::this_thread::sleep_for(500ms);
	//		game.requestClose();
	//	});

	//	game.loop();
	//}

	//TEST_F(GameFixture, LoopStep)
	//{
	//	using namespace std::chrono_literals;

	//	game.loopStep(Time::FromMilliseconds(1ms));
	//}

}