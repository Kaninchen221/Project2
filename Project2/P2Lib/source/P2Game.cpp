#include "P2Game.hpp"

#include <algorithm>

#include "P2Systems.hpp"

namespace P2
{
	bool Game::initialize()
	{
		Clock clock;
		clock.start();

		createWindow();

		schedule.addSystem(WindowSystems::PollEventsLabel{}, WindowSystems::PollEventsLabel::PollEvents, ecs::MainThread{}, ecs::Before(WindowSystems::RenderLabel{}));
		schedule.addSystem(WindowSystems::BuildRenderDataLabel{}, WindowSystems::BuildRenderDataLabel::BuildRenderData, ecs::Before(WindowSystems::RenderLabel{}));
		schedule.addSystem(WindowSystems::RenderLabel{}, WindowSystems::RenderLabel::Render, ecs::MainThread{});

		schedule.buildGraph();
		schedule.resolveGraph();

		/// Add RenderData as a resource, it's required by the BuildRenderData system
		world.addResource(RenderData{});

		/// Create world
		[&world = world]()
			{
				auto window = world.getResource<sf::RenderWindow>();
				if (!window)
				{
					Logger->error("Window resource is invalid");
					return;
				}

				const auto windowSize = window->getView().getSize();

				const int64_t entitiesCount = static_cast<int64_t>(windowSize.x * windowSize.y / ElementSize);
				const auto width = static_cast<int64_t>(windowSize.x / ElementSize);
				auto positionBatcher =
					[width = width](int64_t index) -> Position
					{
						return Position(sf::Vector2f(float(index % width) * ElementSize, float(index / width) * ElementSize));
					};

				std::random_device rd;
				std::mt19937 gen(rd());

				std::uniform_int_distribution<uint32_t> dist(0, std::numeric_limits<uint32_t>::max());

				auto colorBatcher =
					[entitiesCount = entitiesCount, &gen = gen, &dist = dist]
					([[maybe_unused]] int64_t index) -> Color
					{
						/// We are using the sf::Color(uint32_t) constructor to optimize it
						return Color( 
							sf::Color(dist(gen))
						);
					};

				world.spawnBatch(entitiesCount, positionBatcher, colorBatcher);
			}();

		const auto elapsedTime = clock.getElapsedTime();
		Logger->info("Game initialized: {}ms", elapsedTime.getAsMilliseconds().count());
		return true;
	}

	bool Game::deinitialize()
	{
		Logger->info("Game deinitialized");
		return true;
	}

	bool Game::loop()
	{
		Logger->info("Start looping");

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

		auto videoMode = sf::VideoMode::getDesktopMode();
		window->create(videoMode, "Project2", sf::State::Windowed);
	}
}