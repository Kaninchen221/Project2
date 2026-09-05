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

		createGameWorld();

		createRenderData();

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
		//Logger->info("Delta time: {}ms", timeResource->value.asMilliseconds());

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

		const int64_t entitiesCount = static_cast<int64_t>(std::ceil((windowSize.x / ElementSize) * (windowSize.y / ElementSize)));
		const auto width = static_cast<int32_t>(windowSize.x / ElementSize);
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
				// TODO (mid): We should first give the player one channel of color, 
				// and then the other channels will be unlocked as the player progresses

				/// We are using the sf::Color(uint32_t) constructor to optimize it
				return Color( 
					sf::Color::White
				);
			};
		
		world.spawnBatch(entitiesCount, positionBatcher, colorBatcher);

		Logger->info("Game world created with {} entities", world.getEntitiesCount());

		// Create the world config resource, to share the world size with systems
		auto worldConfig = world.addOrGetResource<WorldConfig>();
		worldConfig->entitiesCount = sf::Vector2i(width, static_cast<int>(windowSize.y / ElementSize));

		worldConfig->originalWindowSizePixels = sf::Vector2f(static_cast<float>(window->getSize().x), static_cast<float>(window->getSize().y));
		worldConfig->currentWindowSizePixels = worldConfig->originalWindowSizePixels;
		worldConfig->windowSizeRatio = sf::Vector2f(1.0f, 1.0f);
	}

	void Game::createRenderData()
	{
		auto renderDataResource = world.addOrGetResource<RenderData>();
		if (!renderDataResource)
		{
			Logger->critical("Couldn't create or get RenderData resource");
			return;
		}
		auto& renderData = *renderDataResource;

		auto drawableQuery = ecs::Query<Position, Color>(world);

		constexpr int32_t verticesPerObject = 6;
		auto& vertexBuffer = renderData.vertexBuffer;

		/// Create Vertex Buffer if it's not created
		// TODO (mid): Refactor the componentPerTypeCount, create a function in the query
		const auto componentPerTypeCount = static_cast<uint32_t>(drawableQuery.getComponentCount() / drawableQuery.getTypeCount());
		if (!renderData.isVertexBufferCreated)
		{
			const auto vertexCount = componentPerTypeCount * verticesPerObject;
			if (!vertexBuffer.create(vertexCount))
			{
				Logger->error("Couldn't create vertex buffer, vertex count: {}", vertexCount);
				return;
			}
			vertexBuffer.setPrimitiveType(sf::PrimitiveType::Triangles);
			vertexBuffer.setUsage(sf::VertexBuffer::Usage::Stream);
		}

		const auto vertexCount = componentPerTypeCount * verticesPerObject;
		std::vector<sf::Vertex> vertices;
		vertices.reserve(vertexCount);

		auto it = drawableQuery.begin();
		while (it != drawableQuery.end())
		{
			auto [positionPtr, colorPtr] = *it;
			const auto& position = *positionPtr;
			const auto& color = *colorPtr;

			std::array<sf::Vertex, verticesPerObject> singleObjectVertices;
			singleObjectVertices[0].position = sf::Vector2f{ 0, 0 } + position.value;
			singleObjectVertices[1].position = sf::Vector2f{ ElementSize, 0 } + position.value;
			singleObjectVertices[2].position = sf::Vector2f{ ElementSize, ElementSize } + position.value;
			singleObjectVertices[3].position = sf::Vector2f{ ElementSize, ElementSize } + position.value;
			singleObjectVertices[4].position = sf::Vector2f{ 0, ElementSize } + position.value;
			singleObjectVertices[5].position = sf::Vector2f{ 0, 0 } + position.value;

			singleObjectVertices[0].color = color.value;
			singleObjectVertices[1].color = color.value;
			singleObjectVertices[2].color = color.value;
			singleObjectVertices[3].color = color.value;
			singleObjectVertices[4].color = color.value;
			singleObjectVertices[5].color = color.value;

			vertices.append_range(singleObjectVertices);

			++it;
		}

		if (!vertexBuffer.update(vertices.data()))
		{
			Logger->error("Couldn't update vertex buffer");
			return;
		}

		renderData.isVertexBufferCreated = true;
	}
}