#pragma once

#include "P2LibConfig.hpp"
#include "P2Clock.hpp"
#include "P2Logger.hpp"
#include "P2DataTypes.hpp"

#include "ECS\P2World.hpp"
#include "ECS\P2Schedule.hpp"
#include "ECS\P2Resource.hpp"
#include "ECS\P2Query.hpp"
#include "ECS\P2WorldCommands.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

#include <imgui.h>

namespace P2
{
	struct WindowSystems
	{
		struct PollEventsLabel 
		{
			inline static auto Logger = ConsoleLogger::CreateOrGet("PollEvents");

			static void PollEvents(ecs::Resource<sf::RenderWindow> renderWindowResource);
		};

		struct BuildRenderDataLabel
		{
			inline static auto Logger = ConsoleLogger::CreateOrGet("BuildRenderData");

			static void BuildRenderData(
				ecs::ConstQuery<Position, Color> drawableQuery,
				ecs::Resource<RenderData> renderDataRes
			);
		};

		struct RenderLabel
		{
			inline static auto Logger = ConsoleLogger::CreateOrGet("Render");

			static void Render(
				ecs::Resource<sf::RenderWindow> renderWindowResource,
				ecs::Resource<RenderData> renderDataRes
			);
		};
	};

	struct ImGuiSystems
	{
		inline static ImGuiID MainDockspaceID = 0;

		struct ImGuiUpdateLabel
		{
			static void ImGuiUpdate(
				ecs::Resource<sf::RenderWindow> renderWindowResource,
				ecs::ConstResource<DeltaTime> deltaTimeResource
			);
		};

		struct GameplayWindowLabel
		{
			static void GameplayWindow();
		};
	};
}