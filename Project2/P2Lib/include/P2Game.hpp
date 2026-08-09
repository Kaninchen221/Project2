#pragma once

#include "P2LibConfig.hpp"
#include "P2Clock.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

namespace P2
{
	/// Always call one time initialize before calling any loop method
	/// For looping there are two methods: loop (call and forget about it) or loopStep (mainly for tests)
	/// Always after looping call deinitialize
	class P2_API Game
	{
		inline static auto Logger = spdlog::stdout_color_mt("Game");

	public:

		bool initialize();
		bool deinitialize();

		bool loop();
		void loopStep(const Time& deltaTime);

		void requestClose();

	private:

		Clock gameClock;

		void createWindow();

		bool requestedClose = false;

		sf::RenderWindow window;

	};
}