#include "P2Systems.hpp"

#include <algorithm>

#include <imgui-SFML.h>

#include "P2ImGuiUtils.hpp"

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
		auto& renderData = *renderDataRes;
		if (!renderData.isVertexBufferCreated)
		{
			Logger->critical("Vertex buffer is not created");
			return;
		}

		for (const auto entityIndex : renderData.dirtyEntityIndices)
		{
			auto [posPtr, colorPtr] = drawableQuery[entityIndex].operator*();

			// Update the vertex buffer with the new color
			const unsigned int offset = entityIndex * RenderData::VerticesPerObject;
			const std::array<sf::Vertex, RenderData::VerticesPerObject> vertices =
			{
				sf::Vertex(posPtr->value + sf::Vector2f(0, 0),						colorPtr->value),
				sf::Vertex(posPtr->value + sf::Vector2f(ElementSize, 0),			colorPtr->value),
				sf::Vertex(posPtr->value + sf::Vector2f(ElementSize, ElementSize),	colorPtr->value),
				sf::Vertex(posPtr->value + sf::Vector2f(ElementSize, ElementSize),	colorPtr->value),
				sf::Vertex(posPtr->value + sf::Vector2f(0, ElementSize),			colorPtr->value),
				sf::Vertex(posPtr->value + sf::Vector2f(0, 0),						colorPtr->value)
			};

			const bool result =
				renderData.vertexBuffer.update(
					vertices.data(),
					vertices.size(),
					offset
				);

			if (!result)
			{
				Logger->error("Couldn't update vertex buffer at offset: {}", offset);
			}
		}

		renderData.dirtyEntityIndices.clear();
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

	void WindowSystems::RecreateRenderDataLabel::RecreateRenderData(
		ecs::Resource<WorldConfig> worldConfigResource,
		DrawableConstQuery drawableQuery, 
		ecs::Resource<RenderData> renderDataRes
	)
	{
		auto& worldConfig = *worldConfigResource;
		auto& renderData = *renderDataRes;

		if (!worldConfig.needsRecreateRenderData)
		{
			return;
		}

		constexpr int32_t verticesPerObject = 6;
		auto& vertexBuffer = renderData.vertexBuffer;

		/// Create Vertex Buffer if it's not created
		// TODO (mid): Refactor the componentPerTypeCount, create a function in the query
		const auto componentPerTypeCount = static_cast<uint32_t>(drawableQuery.getComponentCount() / drawableQuery.getTypeCount());
		
		// TODO (mid): Remove this check and 'isVertexBufferCreated' var
		if (!renderData.isVertexBufferCreated)
		{
			vertexBuffer = sf::VertexBuffer{}; // To make sure that the underlying buffer was properly released
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
		worldConfig.needsRecreateRenderData = false;
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
		ecs::Resource<GameplayData> gameplayDataResource,
		ecs::ConstResource<WorldConfig> worldConfigResource
	)
	{
		auto& gameplayWindowData = *gameplayWindowDataResource;

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
				
				if (ImGui::MenuItem("Tips"))
				{
					gameplayWindowData.currentWindow = GameplayWindowLabel::ShowTips;
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

		GameplayWindowParamPack paramPack {
			*deltaTimeResource,
			*gameplayDataResource,
			*worldConfigResource
		};
		std::invoke(gameplayWindowData.currentWindow, paramPack);

		ImGui::End();
	}

	void ImGuiSystems::GameplayWindowLabel::ShowUpgradeWindow(GameplayWindowParamPack& gameplayWindowData)
	{
		auto& gameplayData = gameplayWindowData.gameplayData;
		auto& worldConfig = gameplayWindowData.worldConfig;

		SubWindowTitle("Upgrade Window");

		UpgradeWindowPerChannel(gameplayData.r);
		ImGui::Separator();

		if (worldConfig.avaiableChannelsCountPerEntity > 1)
		{
			UpgradeWindowPerChannel(gameplayData.g);
			ImGui::Separator();
		}

		if (worldConfig.avaiableChannelsCountPerEntity > 2)
		{
			UpgradeWindowPerChannel(gameplayData.b);
			ImGui::Separator();
		}
		//ImGui::Separator();

		ShowGameplayStats(worldConfig, gameplayData);
	}

	void ImGuiSystems::GameplayWindowLabel::UpgradeWindowPerChannel(GameplayDataPerChannel& gameplayDataPerChannel)
	{
		// Resolve the problem with doubled label of upgrade buttons
		ImGui::PushID(gameplayDataPerChannel.name.c_str());

		ImGui::Text(gameplayDataPerChannel.name.c_str());
		ImGui::Text("Current Experience: %d", gameplayDataPerChannel.currentExperience);
		ImGui::Text("Click Strength: %d", gameplayDataPerChannel.clickStrength);

		const int32_t upgradeCost = gameplayDataPerChannel.clickStrength * gameplayDataPerChannel.clickStrength;
		if (ImGui::Button("Upgrade Click Strength"))
		{
			if (gameplayDataPerChannel.currentExperience >= upgradeCost)
			{
				gameplayDataPerChannel.currentExperience -= upgradeCost;
				gameplayDataPerChannel.clickStrength += 1;
			}
		}
		ImGui::SameLine();
		ImGui::Text("Upgrade Cost: %d", upgradeCost);

		ImGui::PopID();
	}

	void ImGuiSystems::GameplayWindowLabel::ShowGameplayStats(const WorldConfig& worldConfig, const GameplayData& gameplayData)
	{
		ImGui::NewLine(); // Horizontal spacing
		SubWindowTitle("GameplayStats");

		ImGui::Text("Available Channels: %d", worldConfig.avaiableChannelsCountPerEntity);
		ImGui::SetItemTooltip("Every quad has channels that define its color, at the start only the red channel is available");
		ImGui::Text("Required Experience: %d", worldConfig.requiredExperienceToFinishCurrentLevel);
		ImGui::SetItemTooltip("Required experience to finish the current level");
		ImGui::Text("Total Experience: %d", gameplayData.totalExperience);
		const float completePercentage = std::roundf(static_cast<float>(gameplayData.totalExperience) / static_cast<float>(worldConfig.requiredExperienceToFinishCurrentLevel) * 100.f);
		ImGui::Text("Complete percentage: %.0f%%", completePercentage);

		if (completePercentage >= 100.f)
		{
			if (ImGui::Button("Next Level"))
			{
				// TODO (high): Request next level
			}
		}
	}

	void ImGuiSystems::GameplayWindowLabel::ShowTips(GameplayWindowParamPack& gameplayWindowData)
	{
		auto& gameplayData = gameplayWindowData.gameplayData;

		SubWindowTitle("Tips");

		ImGui::Text("- Use auto clicker");
		ImGui::Text("- There is no sense in what are you doing");
		ImGui::Text("- Suffer");

		if (gameplayData.r.clickStrength > 1)
		{
			if (ImGui::Button("Don't click this button"))
			{
				gameplayData.r.clickStrength = 1;
				gameplayData.g.clickStrength = 1;
				gameplayData.b.clickStrength = 1;
			}
		}
	}

	void ImGuiSystems::GameplayWindowLabel::ShowDebugStatsWindow(GameplayWindowParamPack& gameplayWindowData)
	{
		auto& deltaTime = gameplayWindowData.deltaTime;

		SubWindowTitle("Debug Stats Window");

		const float deltaTimeAsMS = deltaTime.value.asSeconds() * 1000.f;
		ImGui::Text("Delta time: %.3f ms", deltaTimeAsMS);
		ImGui::Text("FPS: %.3f", 1000.f /*1 second as ms*/ / deltaTimeAsMS);
	}

	void GameplaySystems::ProcessClickLabel::ProcessClick(
		DrawableQuery drawableQuery,
		ecs::ConstResource<WindowEvents> windowEventsResource,
		ecs::ConstResource<WorldConfig> worldConfigResource,
		ecs::Resource<GameplayData> gameplayDataResource,
		ecs::Resource<RenderData> renderDataResource
	)
	{
		auto& windowEvents = *windowEventsResource;
		auto& worldConfig = *worldConfigResource;
		auto& gameplayData = *gameplayDataResource;
		auto& renderData = *renderDataResource;

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

				const auto clickedEntityIndex = clickedAt.x + (worldConfig.worldSize.x * clickedAt.y);
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

				sf::Vector3<int32_t> experience;
				experience.x = -affectColorChannel(colorPtr->value.r, gameplayData.r.clickStrength);
				experience.y = -affectColorChannel(colorPtr->value.g, gameplayData.g.clickStrength);
				experience.z = -affectColorChannel(colorPtr->value.b, gameplayData.b.clickStrength);

				// Mark the entity as dirty in the render data
				renderData.dirtyEntityIndices.emplace_back(clickedEntityIndex);

				// Affect gameplay data
				// Add experience
				auto affectExperience = 
					[&totalExperience = gameplayData.totalExperience](GameplayDataPerChannel& gameplayDataPerChannel, const int32_t experience)
					{
						gameplayDataPerChannel.currentExperience += experience;
						totalExperience += experience;
					};
				affectExperience(gameplayData.r, experience.x);
				affectExperience(gameplayData.g, experience.y);
				affectExperience(gameplayData.b, experience.z);
			}
		}
	}

	void GameplaySystems::CreateWorldLabel::CreateWorld(
		ecs::Resource<WorldConfig> worldConfigResource,
		ecs::WorldCommands worldCommands
	)
	{
		// TODO (high): Erase all drawable entities first

		auto& worldConfig = *worldConfigResource;
		if (!worldConfig.needsRecreateWorld)
		{
			return;
		}

		auto positionBatcher =
			[worldSizeX = worldConfig.worldSize.x](int64_t index) -> Position
			{
				return Position(sf::Vector2f(float(index % worldSizeX) * ElementSize, float(index / worldSizeX) * ElementSize));
			};

		auto colorBatcher =
			[]([[maybe_unused]] int64_t index) -> Color
			{
				// TODO (mid): We should first give the player one channel of color, 
				// and then the other channels will be unlocked as the player progresses

				return Color(
					sf::Color::Red
				);
			};

		worldCommands.spawnBatch(worldConfig.entitiesCount, positionBatcher, colorBatcher);

		worldConfig.needsRecreateWorld = false;
		worldConfig.needsRecreateRenderData = true;
	}

}