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

	namespace WindowSystems
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

		/// TODO (mid): Rename to refresh dirty render data
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

		struct RecreateRenderDataLabel
		{
			inline static auto Logger = ConsoleLogger::CreateOrGet("RecreateRenderDataLabel");

			static void RecreateRenderData(
				ecs::Resource<WorldConfig> worldConfigResource,
				DrawableConstQuery drawableQuery,
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
				ecs::Resource<GameplayData> gameplayDataResource,
				ecs::ConstResource<WorldConfig> worldConfigResource
			);

			/// Gameplay windows
			static void ShowUpgradeWindow(GameplayWindowParamPack& gameplayWindowData);
			static void UpgradeWindowPerChannel(GameplayDataPerChannel& gameplayDataPerChannel);
			static void ShowGameplayStats(const WorldConfig& worldConfig, const GameplayData& gameplayData);

			static void ShowTips(GameplayWindowParamPack& gameplayWindowData);
				
			/// Debug windows
			static void ShowDebugStatsWindow(GameplayWindowParamPack& gameplayWindowData);
			// TODO (very high): Add window to show how much time needs every system
		};
	};

	namespace GameplaySystems
	{
		struct ProcessClickLabel
		{
			inline static auto Logger = ConsoleLogger::CreateOrGet("ProcessClick");

			static void ProcessClick(
				DrawableQuery drawableQuery,
				ecs::ConstResource<WindowEvents> windowEventsResource,
				ecs::ConstResource<WorldConfig> worldConfigResource,
				ecs::Resource<GameplayData> gameplayDataResource,
				ecs::Resource<RenderData> renderDataResource
			);
		};

		struct CreateWorldLabel
		{
			inline static auto Logger = ConsoleLogger::CreateOrGet("CreateWorld");

			static void CreateWorld(
				ecs::Resource<WorldConfig> worldConfigResource,
				ecs::WorldCommands worldCommands
			);
		};
	};
}