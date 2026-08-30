#include "P2Game.hpp"

#include <algorithm>

namespace P2
{
	void WindowSystems::PollEventsLabel::PollEvents(ecs::Resource<sf::RenderWindow> renderWindowResource)
	{
		auto& window = *renderWindowResource;

		if (!window.isOpen())
			return;

		/// We assume that we are using the window only from the main thread
		//if (!window.setActive(true))
		//{
		//	Logger->error("Couldn't active window");
		//	return;
		//}

		while (const auto event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				window.close();
			}
		}
	}

	void WindowSystems::BuildRenderDataLabel::BuildRenderData(
		ecs::ConstQuery<Position, Color> drawableQuery,
		ecs::Resource<RenderData> renderDataRes
	)
	{
		/// TODO (very high): optimize it
		auto& renderData = *renderDataRes;
		if (!renderData.isDirty)
		{
			Logger->trace("render data is not dirty");
			return;
		}

		constexpr int32_t verticesPerObject = 6;
		auto& vertexBuffer = renderData.vertexBuffer;

		/// Create Vertex Buffer if it's not created
		// TODO (mid): Refactor it
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
			vertexBuffer.setUsage(sf::VertexBuffer::Usage::Static);
		}

		const uint32_t firstObject = renderData.chunkToUpdate * renderData.chunkSize;
		auto it = drawableQuery[firstObject];

		const auto rectCountToUpdate =
			[&renderData = renderData, componentPerTypeCount = componentPerTypeCount, firstObject = firstObject]() -> uint32_t
			{ 
				return ::std::min(renderData.chunkSize, componentPerTypeCount - firstObject);
			}();

		std::vector<sf::Vertex> vertices;
		vertices.reserve(renderData.chunkSize * verticesPerObject);

		size_t index = 0;
		while (index != rectCountToUpdate)
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

			//Logger->info("Vertex offset: {}", vertexOffset);

			++index;
			++it;
		}

		const uint32_t vertexOffset = firstObject * verticesPerObject;
		if (!vertexBuffer.update(vertices.data(), vertices.size(), vertexOffset))
		{
			Logger->error("Couldn't update vertex buffer, offset: {}, index: {}", vertexOffset, index);
			return;
		}

		++renderData.chunkToUpdate;

		const auto chunkCount = componentPerTypeCount / renderData.chunkSize;
		if (renderData.chunkToUpdate == chunkCount)
		{
			renderData.chunkToUpdate = 0;
		}

		//renderData.isDirty = false;
		renderData.vertexBuffer = std::move(vertexBuffer);
	}

	void WindowSystems::RenderLabel::Render(
		ecs::Resource<sf::RenderWindow> renderWindowResource,
		ecs::Resource<RenderData> renderDataRes
	)
	{
		auto& window = *renderWindowResource;
		if (!window.isOpen())
		{
			return;
		}

		/// We assume that we are using the window only from the main thread
		//if (!window.setActive(true))
		//{
		//	Logger->error("Couldn't active window");
		//	return;
		//}

		window.clear();

		if (renderDataRes)
		{
			window.draw(renderDataRes->vertexBuffer);
		}
		else
		{
			Logger->error("renderDataRes is nullptr");
			return;
		}

		window.display();
	}

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