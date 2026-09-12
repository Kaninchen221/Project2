#include "P2Game.hpp"

#include <algorithm>

#include "P2Systems.hpp"
#include "P2GameplayUtils.hpp"

#include <imgui-SFML.h>
#include <imgui.h>

namespace P2
{
	bool Game::initialize()
	{
		Clock clock;
		clock.start();

		createWindow();

		createGameWorldConfig();

		schedule.addSystem(GameplaySystems::CreateWorldLabel{}, GameplaySystems::CreateWorldLabel::CreateWorld);

		schedule.addSystem(WindowSystems::RecreateRenderDataLabel{}, WindowSystems::RecreateRenderDataLabel::RecreateRenderData, ecs::After(GameplaySystems::CreateWorldLabel{}), ecs::Before(WindowSystems::RefreshDirtyRenderDataLabel{}));

		schedule.addSystem(WindowSystems::PollEventsLabel{}, WindowSystems::PollEventsLabel::PollEvents, ecs::MainThread{}, ecs::Before(WindowSystems::RenderLabel{}));
		schedule.addSystem(WindowSystems::RefreshDirtyRenderDataLabel{}, WindowSystems::RefreshDirtyRenderDataLabel::RefreshDirtyRenderData, ecs::Before(WindowSystems::RenderLabel{}));
		schedule.addSystem(WindowSystems::RenderLabel{}, WindowSystems::RenderLabel::Render, ecs::MainThread{});

		schedule.addSystem(ImGuiSystems::ImGuiUpdateLabel{}, ImGuiSystems::ImGuiUpdateLabel::ImGuiUpdate, ecs::MainThread{}, ecs::Before(WindowSystems::RenderLabel{}));
		schedule.addSystem(ImGuiSystems::GameplayWindowLabel{}, ImGuiSystems::GameplayWindowLabel::GameplayWindow, ecs::MainThread{}, ecs::After(ImGuiSystems::ImGuiUpdateLabel{}), ecs::Before(WindowSystems::RenderLabel{}));

		schedule.addSystem(GameplaySystems::ProcessClickLabel{}, GameplaySystems::ProcessClickLabel::ProcessClick, ecs::After(WindowSystems::PollEventsLabel{}));

		schedule.buildGraph();
		schedule.resolveGraph();

		/// Required by the BuildRenderData system
		world.addResource(RenderData{});

		/// Required by the PollEvents system
		world.addResource(WindowEvents{});

		/// Required by the GameplayWindow system
		world.addResource(GameplayWindowData{});

		/// Required by the gameplay systems
		world.addResource(GameplayData{});

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
		// Update the DeltaTime
		auto timeResource = world.addOrGetResource<DeltaTime>();
		if (!timeResource)
		{
			Logger->critical("Couldn't create or get DeltaTime resource");
			return;
		}

		timeResource->value = deltaClock.restart();
		if (timeResource->value.asMilliseconds() > 1)
		{
			Logger->warn("Big delta time: {}us, skip it", timeResource->value.asMicroseconds());
			return;
		}

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

	void Game::createGameWorldConfig()
	{
		// Prepare params
		auto window = world.getResource<sf::RenderWindow>();
		if (!window)
		{
			Logger->error("Window resource is invalid");
			return;
		}

		const auto windowSize = window->getView().getSize();

		// Create the world config resource, to share the world data with systems
		auto worldConfig = world.addOrGetResource<WorldConfig>();
		worldConfig->worldSize = GetWorldSizeFromWindowSize(windowSize, worldConfig->elementSize);
		worldConfig->entitiesCount = GetEntitiesCountFromWorldSize(worldConfig->worldSize);
		worldConfig->originalWindowSizePixels = sf::Vector2f(static_cast<float>(window->getSize().x), static_cast<float>(window->getSize().y));
		worldConfig->currentWindowSizePixels = worldConfig->originalWindowSizePixels;
		worldConfig->windowSizeRatio = sf::Vector2f(1.0f, 1.0f);
		worldConfig->avaiableChannelsCountPerEntity = 1;
		worldConfig->requiredExperienceToFinishCurrentLevel = GetRequiredExperienceToFinishCurrentLevel(worldConfig->entitiesCount, worldConfig->avaiableChannelsCountPerEntity);
		worldConfig->needsRecreateWorld = true;
	}
}