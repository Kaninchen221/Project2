#include "P2Game.hpp"

#include <algorithm>

#include "P2Systems.hpp"

#include <imgui-SFML.h>
#include <imgui.h>

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

		schedule.addSystem(ImGuiSystems::ImGuiUpdateLabel{}, ImGuiSystems::ImGuiUpdateLabel::ImGuiUpdate, ecs::MainThread{}, ecs::Before(WindowSystems::RenderLabel{}));
		schedule.addSystem(ImGuiSystems::GameplayWindowLabel{}, ImGuiSystems::GameplayWindowLabel::GameplayWindow, ecs::MainThread{}, ecs::After(ImGuiSystems::ImGuiUpdateLabel{}), ecs::Before(WindowSystems::RenderLabel{}));

		schedule.buildGraph();
		schedule.resolveGraph();

		/// Add RenderData as a resource, it's required by the BuildRenderData system
		world.addResource(RenderData{});

		createGameWorld();

		const auto elapsedTime = clock.getElapsedTime();
		Logger->info("Game initialized: {}ms", elapsedTime.getAsMilliseconds().count());
		return true;
	}

	bool Game::deinitialize()
	{
		ImGui::SFML::Shutdown();

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
		/// TODO (mid): Skip the "runOnce" if the delta time is too high
		// Update the DeltaTime
		auto timeResource = world.addOrGetResource<DeltaTime>();
		if (!timeResource)
		{
			Logger->critical("Couldn't create or get DeltaTime resource");
			return;
		}
		timeResource->value = deltaClock.restart();

		// Run the schedule
		schedule.runOnce(world);
	}

	void Game::requestClose()
	{
		requestedClose = true;
	}

	void Game::createWindow()
	{
		auto window = world.addOrGetResource<sf::RenderWindow>();
		if (!window)
		{
			Logger->critical("Couldn't create or get window resource");
			return;
		}

		auto videoMode = sf::VideoMode::getDesktopMode();
		window->create(videoMode, "Project2", sf::State::Windowed);

		if (!ImGui::SFML::Init(*window))
		{
			Logger->critical("Couldn't initialize ImGui-SFML");
			return;
		}

		ImGuiIO& io = ImGui::GetIO();

		// Enable docking
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	}

	void Game::createGameWorld()
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
	}
}