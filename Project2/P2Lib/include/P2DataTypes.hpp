#pragma once

#include "P2LibConfig.hpp"

#include <chrono>

#include <SFML/Graphics.hpp>

namespace P2
{	
	inline static float ElementSize = 16;

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
		const uint32_t chunkSize = 64;
		uint32_t chunkToUpdate = 0;
	};

	struct GameplayWindowData
	{
		std::function<void(const DeltaTime&)> currentWindow;
	};
}