#pragma once

#include "P2LibConfig.hpp"

#include "P2DataTypes.hpp"

namespace P2
{
	static sf::Vector2<int32_t> GetWorldSizeFromWindowSize(const sf::Vector2f& windowSize, const float elementSize)
	{
		return 
			sf::Vector2<int32_t>(
				static_cast<int32_t>(std::round(windowSize.x / elementSize)),
				static_cast<int32_t>(std::round(windowSize.y / elementSize))
			);
	}

	static int64_t GetEntitiesCountFromWorldSize(const sf::Vector2<int32_t>& worldSize)
	{
		return static_cast<int64_t>(worldSize.x) * static_cast<int64_t>(worldSize.y);
	}

	static int256_t GetRequiredExperienceToFinishCurrentLevel(const int64_t entitiesCount, const int64_t availableChannelsCountPerEntity)
	{
		return int256_t(entitiesCount) * availableChannelsCountPerEntity * WorldConfig::MaxChannelValue * 1'000'000;
	}

	inline static WorldConfig GetNextLevelWorldConfig(const WorldConfig& worldConfig)
	{
		const float elementSize = std::clamp(worldConfig.elementSize / 2.f, 1.f, std::numeric_limits<float>::max());

		const auto worldSize = GetWorldSizeFromWindowSize(worldConfig.originalWindowSizePixels, elementSize);
		const int64_t entitiesCount = GetEntitiesCountFromWorldSize(worldSize);
		const int32_t availableChannelsCountPerEntity = std::clamp(worldConfig.avaiableChannelsCountPerEntity + 1, 1, 3);
		const auto requiredExperienceToFinishCurrentLevel = 
			GetRequiredExperienceToFinishCurrentLevel(entitiesCount, availableChannelsCountPerEntity) + worldConfig.requiredExperienceToFinishCurrentLevel;
		const auto requiredExperienceToFinishCurrentLevelAsString = requiredExperienceToFinishCurrentLevel.str();

		const WorldConfig newWorldConfig
		{
			.elementSize = elementSize,
			.worldSize = worldSize,
			.entitiesCount = entitiesCount,
			.originalWindowSizePixels = worldConfig.originalWindowSizePixels,
			.currentWindowSizePixels = worldConfig.currentWindowSizePixels,
			.windowSizeRatio = worldConfig.windowSizeRatio,
			.currentLevel = worldConfig.currentLevel + 1,
			.avaiableChannelsCountPerEntity = availableChannelsCountPerEntity,
			.requiredExperienceToFinishCurrentLevel = requiredExperienceToFinishCurrentLevel,
			.needsRecreateWorld = true,
			.needsRecreateRenderData = worldConfig.needsRecreateRenderData
		};

		return newWorldConfig;
	}

	inline static bool IsFinalLevel(float elementSize)
	{
		return std::floor(elementSize) == 1.f;
	}
}
