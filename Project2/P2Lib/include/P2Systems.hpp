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
	using DrawableConstQuery = ecs::ConstQuery<Position, Color>;
	using DrawableQuery = ecs::Query<Position, Color>;

	struct WindowSystems
	{
		struct PollEventsLabel 
		{
			inline static auto Logger = ConsoleLogger::CreateOrGet("PollEvents");

			static void PollEvents(
				ecs::Resource<WorldConfig> worldConfigResource,
				ecs::Resource<sf::RenderWindow> renderWindowResource,
				ecs::Resource<WindowEvents> windowEventsResource
			);
		};

		struct BuildRenderDataLabel
		{
			inline static auto Logger = ConsoleLogger::CreateOrGet("BuildRenderData");

			static void BuildRenderData(
				DrawableConstQuery drawableQuery,
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
			static void GameplayWindow(
				ecs::Resource<GameplayWindowData> gameplayWindowDataResource,
				ecs::ConstResource<DeltaTime> deltaTimeResource,
				ecs::Resource<GameplayData> gameplayDataResource
			);

			/// Gameplay windows
			static void ShowUpgradeWindow(const DeltaTime& deltaTime, GameplayData& gameplayData);

			/// Debug windows
			static void ShowDebugStatsWindow(const DeltaTime& deltaTime, GameplayData&);

			
		};
	};

	struct GameplaySystems
	{
		struct ProcessClickLabel
		{
			inline static auto Logger = ConsoleLogger::CreateOrGet("ProcessClick");

			static void ProcessClick(
				DrawableQuery drawableQuery,
				ecs::ConstResource<WindowEvents> windowEventsResource,
				ecs::ConstResource<WorldConfig> worldConfigResource,
				ecs::Resource<GameplayData> gameplayDataResource
			);
		};
	};
}