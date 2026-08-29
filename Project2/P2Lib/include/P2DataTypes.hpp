#pragma once

#include "P2LibConfig.hpp"

#include <chrono>

#include <SFML/Graphics.hpp>

namespace P2
{	
	inline static float ElementSize = 10;

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
	};
}