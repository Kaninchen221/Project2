#include "P2Systems.hpp"

#include <algorithm>

#include <imgui-SFML.h>

using namespace std::chrono_literals;

namespace P2
{
	void WindowSystems::PollEventsLabel::PollEvents(
		ecs::Resource<WorldConfig> worldConfigResource,
		ecs::Resource<sf::RenderWindow> renderWindowResource,
		ecs::Resource<WindowEvents> windowEventsResource
	)
	{
		auto& window = *renderWindowResource;
		if (!window.isOpen())
			return;

		/// We assume that we are using the window only from the main thread
		//window.setActive(true);

		auto& windowEvents = *windowEventsResource;
		windowEvents.events.clear();

		while (const auto event = window.pollEvent())
		{
			windowEvents.events.emplace_back(event);

			ImGui::SFML::ProcessEvent(window, *event);

			if (event->is<sf::Event::Closed>())
			{
				Logger->info("Event closed");

				window.close();
			}

			if (const auto* resizedEvent = event->getIf<sf::Event::Resized>())
			{
				Logger->info("Event resized");

				auto& worldConfig = *worldConfigResource;
				worldConfig.currentWindowSizePixels = sf::Vector2f(static_cast<float>(resizedEvent->size.x), static_cast<float>(resizedEvent->size.y));

				worldConfig.windowSizeRatio = 
					sf::Vector2f(
						worldConfig.currentWindowSizePixels.x / worldConfig.originalWindowSizePixels.x,
						worldConfig.currentWindowSizePixels.y / worldConfig.originalWindowSizePixels.y
					);

				Logger->info("Window size ratio: {}, {}", worldConfig.windowSizeRatio.x, worldConfig.windowSizeRatio.y);
			}
		}
	}

	void WindowSystems::BuildRenderDataLabel::BuildRenderData(
		ecs::ConstQuery<Position, Color> drawableQuery,
		ecs::Resource<RenderData> renderDataRes
	)
	{
		// TODO (high): Optimize it, perhaps when we change the color of an element then we want to somehow mark it as dirty

		auto& renderData = *renderDataRes;
		if (!renderData.isDirty)
		{
			Logger->trace("render data is not dirty");
			return;
		}

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
			vertexBuffer.setUsage(sf::VertexBuffer::Usage::Static);
		}

		const uint32_t firstObject = renderData.chunkToUpdate * renderData.chunkSize;
		auto it = drawableQuery[firstObject];

		const auto rectCountToUpdate =
			[&renderData = renderData, componentPerTypeCount = componentPerTypeCount, firstObject = firstObject]() -> uint32_t
			{ 
				return std::min(renderData.chunkSize, componentPerTypeCount - firstObject);
			}();

		const auto vertexCount = renderData.chunkSize * verticesPerObject;
		std::vector<sf::Vertex> vertices;
		vertices.reserve(vertexCount);

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
		//window.setActive(true);

		window.clear();

		window.draw(renderDataRes->vertexBuffer);

		ImGui::SFML::Render(window);

		window.display();
	}

	void ImGuiSystems::ImGuiUpdateLabel::ImGuiUpdate(
		ecs::Resource<sf::RenderWindow> renderWindowResource,
		ecs::ConstResource<DeltaTime> deltaTimeResource
	)
	{
		ImGui::SFML::Update(*renderWindowResource, deltaTimeResource->value);
		
		ImGui::DockSpaceOverViewport(
			MainDockspaceID,
			ImGui::GetMainViewport(),
			ImGuiDockNodeFlags_PassthruCentralNode // Allow the background to be visible
		);
	}

	void ImGuiSystems::GameplayWindowLabel::GameplayWindow(
		ecs::Resource<GameplayWindowData> gameplayWindowDataResource,
		ecs::ConstResource<DeltaTime> deltaTimeResource,
		ecs::Resource<GameplayData> gameplayDataResource
	)
	{
		auto& gameplayWindowData = *gameplayWindowDataResource;
		auto& deltaTime = *deltaTimeResource;
		auto& gameplayData = *gameplayDataResource;

		//ImGui::ShowDemoWindow();
		if (!gameplayWindowData.currentWindow)
		{
			gameplayWindowData.currentWindow = GameplayWindowLabel::ShowUpgradeWindow;
		}

		ImGui::Begin("MainWindow", nullptr, ImGuiWindowFlags_MenuBar);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Game"))
			{
				if (ImGui::MenuItem("Upgrade")) 
				{
					gameplayWindowData.currentWindow = GameplayWindowLabel::ShowUpgradeWindow;
				}

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Debug"))
			{
				if (ImGui::MenuItem("Stats")) 
				{
					gameplayWindowData.currentWindow = GameplayWindowLabel::ShowDebugStatsWindow;
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		std::invoke(gameplayWindowData.currentWindow, deltaTime, gameplayData);

		ImGui::End();
	}

	void ImGuiSystems::GameplayWindowLabel::ShowUpgradeWindow(const DeltaTime&, GameplayData& gameplayData)
	{
		ImGui::Text("Upgrade window");

		ImGui::Text("Current Level: %d", gameplayData.currentLevel);
		ImGui::Text("Current Experience: %d", gameplayData.currentExperience);
		ImGui::Text("Click Strength: %d", gameplayData.clickStrength);

		const int32_t upgradeCost = gameplayData.clickStrength * gameplayData.clickStrength;
		if (ImGui::Button("Upgrade Click Strength"))
		{
			if (gameplayData.currentExperience >= upgradeCost)
			{
				gameplayData.currentExperience -= upgradeCost;
				gameplayData.clickStrength += 1;
			}
		}
		ImGui::SameLine();
		ImGui::Text("Upgrade Cost: %d", upgradeCost);
	}

	void ImGuiSystems::GameplayWindowLabel::ShowDebugStatsWindow(const DeltaTime& deltaTime, GameplayData&)
	{
		ImGui::Text("Debug Stats window");
		ImGui::Text("Delta time: %.3f ms", deltaTime.value.asSeconds() * 1000.f);
	}

	void GameplaySystems::ProcessClickLabel::ProcessClick(
		DrawableQuery drawableQuery,
		ecs::ConstResource<WindowEvents> windowEventsResource,
		ecs::ConstResource<WorldConfig> worldConfigResource,
		ecs::Resource<GameplayData> gameplayDataResource
	)
	{
		auto& windowEvents = *windowEventsResource;
		auto& worldConfig = *worldConfigResource;
		auto& gameplayData = *gameplayDataResource;

		for (const auto& event : windowEvents.events)
		{
			Logger->trace("Events count: {}", windowEvents.events.size());

			if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>())
			{
				// Convert mouse viewport position to entity index

				const auto clickedAt = 
					sf::Vector2i(
						static_cast<int>(mouseEvent->position.x / (ElementSize * worldConfig.windowSizeRatio.x)),
						static_cast<int>(mouseEvent->position.y / (ElementSize * worldConfig.windowSizeRatio.y))
					);

				Logger->trace("Mouse clicked at element: {}, {}", clickedAt.x, clickedAt.y);

				const auto clickedEntityIndex = clickedAt.x + (worldConfig.entitiesCount.x * clickedAt.y);
				Logger->trace("Mouse clicked at element: {}", clickedEntityIndex);
				
				// Check bounds
				const auto entitiesCount = drawableQuery.getComponentCount() / drawableQuery.getTypeCount();
				if (clickedEntityIndex < 0 || clickedEntityIndex >= entitiesCount)
				{
					Logger->trace("Player clicked not at any entity");
					continue;
				}

				// Affect the clicked entity's color
				auto [posPtr, colorPtr] = drawableQuery.operator[](clickedEntityIndex).operator*();

				auto affectColorChannel = 
					[](uint8_t& channel, int32_t clickStrength)
					{
						const auto newChannelValue = static_cast<int32_t>(channel) - clickStrength;
						const auto diff = std::clamp<int32_t>(newChannelValue, 0, 255) - static_cast<int32_t>(channel);
						channel = static_cast<uint8_t>(std::clamp<int32_t>(newChannelValue, 0, 255));
						return diff;
					};

				sf::Vector3i experience;
				experience.x = -affectColorChannel(colorPtr->value.r, gameplayData.clickStrength);
				experience.y = -affectColorChannel(colorPtr->value.g, gameplayData.clickStrength);
				experience.z = -affectColorChannel(colorPtr->value.b, gameplayData.clickStrength);

				// Affect gameplay data
				// Add experience
				[](GameplayData& gameplayData, const sf::Vector3i& experience)
				{
					gameplayData.currentExperience += experience.x + experience.y + experience.z;
				}(gameplayData, experience);
			}
		}
	}

}