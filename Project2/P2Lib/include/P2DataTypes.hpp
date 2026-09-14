#pragma once

#include "P2LibConfig.hpp"
#include "P2BoostTypes.hpp"

#include <chrono>

#include <SFML/Graphics.hpp>

namespace P2
{
	namespace ecs
	{
		struct Graph;
	}

	struct GameplayDataPerChannel
	{
		std::string name;
		int32_t clickStrength = 1;
		int32_t currentExperience = 0;
	};

	struct GameplayData
	{
		GameplayDataPerChannel r{ "Red" };
		GameplayDataPerChannel g{ "Green" };
		GameplayDataPerChannel b{ "Blue" };

		int64_t totalExperience = 0;
	};

	struct WorldConfig
	{
		constexpr static int32_t MaxChannelValue = 255;

		float elementSize = 2048;
		// Count of entities in the world, in X and Y dimensions
		sf::Vector2<int32_t> worldSize;
		int64_t entitiesCount = 0;
		sf::Vector2f originalWindowSizePixels;
		sf::Vector2f currentWindowSizePixels;
		sf::Vector2f windowSizeRatio;
		int32_t currentLevel = 1;
		int32_t avaiableChannelsCountPerEntity = 0;
		int256_t requiredExperienceToFinishCurrentLevel = 0;
		bool needsRecreateWorld = false;
		bool needsRecreateRenderData = false;
	};

	struct DeltaTime
	{
		sf::Time value;
	};

	struct Position
	{
		sf::Vector2f value;
	};

	struct Color
	{
		sf::Color value;
	};

	struct RenderData
	{
		static constexpr int32_t VerticesPerObject = 6;

		// TODO (very high): use something different than sf::VertexBuffer
		// It's so unoptimized, memory and drawing speed
		sf::VertexBuffer vertexBuffer;
		bool isVertexBufferCreated = false;

		std::vector<int32_t> dirtyEntityIndices;
	};

	struct WindowEvents
	{
		std::vector<std::optional<sf::Event>> events;
	};

	struct GameplayWindowParamPack
	{
		const DeltaTime& deltaTime;
		GameplayData& gameplayData;
		WorldConfig& worldConfig; // TODO (low): it's a future candidate to be const,
		const ecs::Graph& scheduleGraph;
	};

	struct GameplayWindowData
	{
		std::function<void(GameplayWindowParamPack&)> currentWindow;
	};
}