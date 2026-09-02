#pragma once

#include "P2LibConfig.hpp"

#include <chrono>

#include <SFML/Graphics.hpp>

namespace P2
{	
	inline static float ElementSize = 16;

	struct WorldConfig
	{
		sf::Vector2i size;
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
		const uint32_t chunkSize = 128;
		uint32_t chunkToUpdate = 0;
	};

	struct WindowEvents
	{
		std::vector<std::optional<sf::Event>> events;
	};

	struct GameplayWindowData
	{
		std::function<void(const DeltaTime&)> currentWindow;
	};
}