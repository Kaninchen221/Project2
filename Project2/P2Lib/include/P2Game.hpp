#pragma once

#include "P2LibConfig.hpp"
#include "P2Clock.hpp"
#include "P2Logger.hpp"
#include "P2DataTypes.hpp"

#include "ECS\P2World.hpp"
#include "ECS\P2Schedule.hpp"
#include "ECS\P2Resource.hpp"
#include "ECS\P2Query.hpp"
#include "ECS\P2WorldCommands.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

namespace P2
{
	/// Always call one time initialize before calling any loop method
	/// For looping there are two methods: loop (call and forget about it) or loopStep (mainly for tests)
	/// Always after looping call deinitialize
	class P2_API Game
	{
		inline static auto Logger = ConsoleLogger::CreateOrGet("Game");

	public:

		bool initialize();
		bool deinitialize();

		bool loop();
		void loopStep(const Time& deltaTime);

		void requestClose();

		auto& getWorld(this auto& self) { return self.world; }

	private:

		Clock gameClock;

		void createWindow();
		void createGameWorld();

		bool requestedClose = false;

		ecs::World world;
		ecs::Schedule schedule;

	};
}