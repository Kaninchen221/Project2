#include "P2Game.hpp"

namespace P2
{
	void WindowSystems::PollEventsLabel::PollEvents(ecs::Resource<sf::RenderWindow> renderWindowResource)
	{
		auto& window = *renderWindowResource;

		if (!window.isOpen())
			return;

		if (!window.setActive(true))
		{
			Logger->error("Couldn't active window");
			return;
		}

		while (const auto event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				window.close();
			}
		}
	}

	void WindowSystems::RenderLabel::Render(ecs::Resource<sf::RenderWindow> renderWindowResource)
	{
		auto& window = *renderWindowResource;

		if (!window.isOpen())
			return;

		if (!window.setActive(true))
		{
			Logger->error("Couldn't active window");
			return;
		}

		window.clear();

		

		window.display();
	}

	bool Game::initialize()
	{
		createWindow();

		schedule.addSystem(WindowSystems::PollEventsLabel{}, WindowSystems::PollEventsLabel::PollEvents, ecs::MainThread{}, ecs::Before(WindowSystems::RenderLabel{}));
		schedule.addSystem(WindowSystems::RenderLabel{}, WindowSystems::RenderLabel::Render, ecs::MainThread{});

		schedule.buildGraph();
		schedule.resolveGraph();

		return true;
	}

	bool Game::deinitialize()
	{
		return true;
	}

	bool Game::loop()
	{
		auto windowResource = world.getResource<sf::RenderWindow>();
		auto& window = *windowResource;

		gameClock.start();
		while (window.isOpen())
		{
			loopStep(gameClock.restart());

			if (requestedClose)
			{
				window.close();
				break;
			}
		}

		return true;
	}

	void Game::loopStep(const Time&)
	{
		schedule.runOnce(world);
	}

	void Game::requestClose()
	{
		requestedClose = true;
	}

	void Game::createWindow()
	{
		auto window = world.addOrGetResource<sf::RenderWindow>();

		const auto videoMode = sf::VideoMode::getDesktopMode();
		window->create(videoMode, "Project2", sf::State::Fullscreen);
	}
}