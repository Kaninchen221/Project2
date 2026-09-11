#pragma once

#include "P2LibConfig.hpp"

#include <chrono>

#include <SFML/Graphics.hpp>

// TODO (mid): World changing
// After meeting a few requirements we should get a new world with smalled element size
// And the smallest element size should be 1

namespace P2
{	
	// TODO (mid): Move this to the WorldConfig resource
	inline static float ElementSize = 2048;

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

		// Count of entities in the world, in X and Y dimensions
		sf::Vector2<int32_t> worldSize;
		int64_t entitiesCount = 0;
		sf::Vector2f originalWindowSizePixels;
		sf::Vector2f currentWindowSizePixels;
		sf::Vector2f windowSizeRatio;
		// TODO (mid): Count which level is the current level
		int32_t avaiableChannelsCountPerEntity = 0;
		// TODO (high): it's flipping the sign after X levels
		int64_t requiredExperienceToFinishCurrentLevel = 0;
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
		WorldConfig& worldConfig; // TODO (mid): it's a future candidate to be const
	};

	struct GameplayWindowData
	{
		std::function<void(GameplayWindowParamPack&)> currentWindow;
	};
}