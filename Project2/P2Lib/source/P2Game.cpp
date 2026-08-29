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

	void WindowSystems::BuildRenderDataLabel::BuildRenderData(
		ecs::ConstQuery<Position, Color> drawableQuery,
		ecs::Resource<RenderData> renderDataRes
	)
	{
		if (!renderDataRes)
		{
			Logger->error("renderDataRes is nullptr");
			return;
		}

		auto& renderData = *renderDataRes;
		if (!renderData.isDirty)
		{
			Logger->trace("render data is not dirty");
			return;
		}

		constexpr int32_t verticesPerObject = 4;
		sf::VertexBuffer vertexBuffer;
		const auto vertexCount = drawableQuery.getComponentCount() / drawableQuery.getTypeCount() * verticesPerObject;
		if (!vertexBuffer.create(vertexCount))
		{
			Logger->error("Couldn't create vertex buffer, vertex count: {}", vertexCount);
			return;
		}
		vertexBuffer.setPrimitiveType(sf::PrimitiveType::TriangleStrip);
		vertexBuffer.setUsage(sf::VertexBuffer::Usage::Static);

		uint32_t offset = 0;
		uint32_t index = 0;
		for (auto [positionPtr, colorPtr] : drawableQuery)
		{
			const auto& position = *positionPtr;
			const auto& color = *colorPtr;

			std::array<sf::Vertex, verticesPerObject> vertices;
			vertices[3].position = sf::Vector2f{ 0, ElementSize } + position.value;
			vertices[2].position = sf::Vector2f{ 0, 0 } + position.value;
			vertices[1].position = sf::Vector2f{ ElementSize, 0 } + position.value;
			vertices[0].position = sf::Vector2f{ ElementSize, ElementSize } + position.value;

			vertices[0].color = color.value;
			vertices[1].color = color.value;
			vertices[2].color = color.value;
			vertices[3].color = color.value;

			if (!vertexBuffer.update(vertices.data(), verticesPerObject, offset))
			{
				Logger->error("Couldn't update vertex buffer, offset: {}, index: {}", offset, index);
				return;
			}

			offset += verticesPerObject;
			++index;
		}

		renderData.isDirty = false;
		renderData.vertexBuffer = std::move(vertexBuffer);
	}

	void WindowSystems::RenderLabel::Render(
		ecs::Resource<sf::RenderWindow> renderWindowResource,
		ecs::Resource<RenderData> renderDataRes
	)
	{
		if (!renderWindowResource)
		{
			Logger->error("renderWindowResource is nullptr");
			return;
		}
		auto& window = *renderWindowResource;

		if (!window.isOpen())
		{
			return;
		}

		if (!window.setActive(true))
		{
			Logger->error("Couldn't active window");
			return;
		}

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
		createWindow();

		schedule.addSystem(WindowSystems::PollEventsLabel{}, WindowSystems::PollEventsLabel::PollEvents, ecs::MainThread{}, ecs::Before(WindowSystems::RenderLabel{}));
		schedule.addSystem(WindowSystems::BuildRenderDataLabel{}, WindowSystems::BuildRenderDataLabel::BuildRenderData, ecs::Before(WindowSystems::RenderLabel{}));
		schedule.addSystem(WindowSystems::RenderLabel{}, WindowSystems::RenderLabel::Render, ecs::MainThread{});

		schedule.buildGraph();
		schedule.resolveGraph();

		/// Add RenderData as a resource, it's required by the BuildRenderData system
		world.addResource(RenderData{});

		/// Create world
		const int64_t entitiesCount = 1'000'000;
		const int64_t width = 100;
		auto positionBatcher =
			[width = width](int64_t index) -> Position
			{
				return Position( sf::Vector2f( float(index % width) * ElementSize, float(index / width) * ElementSize ) );
			};

		auto colorBatcher =
			[entitiesCount = entitiesCount]([[maybe_unused]] int64_t index) -> Color
			{
				//const uint8_t singleChannel = static_cast<uint8_t>(255 / entitiesCount * index);
				//return Color( sf::Color( singleChannel, singleChannel, singleChannel) );
				return Color( sf::Color::White );
			};

		world.spawnBatch(entitiesCount, positionBatcher, colorBatcher);

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