#include "P2Systems.hpp"

#include <algorithm>

#include <imgui-SFML.h>

using namespace std::chrono_literals;

namespace P2
{
	void WindowSystems::PollEventsLabel::PollEvents(ecs::Resource<sf::RenderWindow> renderWindowResource)
	{
		auto& window = *renderWindowResource;

		if (!window.isOpen())
			return;

		/// We assume that we are using the window only from the main thread
		//window.setActive(true);

		while (const auto event = window.pollEvent())
		{
			ImGui::SFML::ProcessEvent(window, *event);

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

	void ImGuiSystems::GameplayWindowLabel::GameplayWindow()
	{
		ImGui::Begin("Game");
		ImGui::Button("Look at this pretty button");
		ImGui::End();
	}

}