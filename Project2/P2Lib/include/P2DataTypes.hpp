#pragma once

#include "P2LibConfig.hpp"

#include <chrono>

#include <SFML/Graphics.hpp>

namespace P2
{	
	// TODO (mid): Move this to the WorldConfig resource
	inline static float ElementSize = 16;

	struct GameplayData
	{
		int32_t clickStrength = 1;
		int32_t currentLevel = 1;
		int32_t currentExperience = 0;
	};

	struct WorldConfig
	{
		sf::Vector2i entitiesCount;
		sf::Vector2f originalWindowSizePixels;
		sf::Vector2f currentWindowSizePixels;
		sf::Vector2f windowSizeRatio;
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
		bool isDirty = true;
		sf::VertexBuffer vertexBuffer;
		bool isVertexBufferCreated = false;

		/// Single chunk has a shape related to how the entities are located
		const uint32_t chunkSize = 4096;
		uint32_t chunkToUpdate = 0;
	};

	struct WindowEvents
	{
		std::vector<std::optional<sf::Event>> events;
	};

	struct GameplayWindowData
	{
		std::function<void(const DeltaTime&, GameplayData&)> currentWindow;
	};
}