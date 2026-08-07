#include "P2Game.hpp"

namespace P2
{
	bool Game::initialize()
	{
		createWindow();

		return true;
	}

	bool Game::deinitialize()
	{
		return true;
	}

	bool Game::loop()
	{
		gameClock.start();
		while (window.isOpen())
		{
			loopStep(gameClock.restart());

			if (requestedClose)
			{
				window.close();
			}
		}

		return true;
	}

	void Game::loopStep(const Time& DeltaTime)
	{
		while (const auto event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				window.close();
			}
		}

		window.clear();

		Logger->info("{}", DeltaTime.getAsSeconds());

		/// Inform objects. systems etc. about DeltaTime

		window.display();
	}

	void Game::requestClose()
	{
		requestedClose = true;
	}

	void Game::createWindow()
	{
		const auto videoMode = sf::VideoMode::getDesktopMode();
		
		window.create(videoMode, "Project2", sf::State::Fullscreen);
	}
}