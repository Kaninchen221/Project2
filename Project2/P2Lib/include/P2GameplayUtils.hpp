#pragma once

#include "P2LibConfig.hpp"

#include "P2DataTypes.hpp"

namespace P2
{
	static sf::Vector2<int32_t> GetWorldSizeFromWindowSize(const sf::Vector2f& windowSize, const float elementSize)
	{
		return 
			sf::Vector2<int32_t>(
				static_cast<int32_t>(std::ceil(windowSize.x / elementSize)),
				static_cast<int32_t>(std::ceil(windowSize.y / elementSize))
			);
	}

	static int64_t GetEntitiesCountFromWorldSize(const sf::Vector2<int32_t>& worldSize)
	{
		return static_cast<int64_t>(worldSize.x) * static_cast<int64_t>(worldSize.y);
	}

	static int64_t GetRequiredExperienceToFinishCurrentLevel(const int64_t entitiesCount, const int32_t availableChannelsCountPerEntity)
	{
		return entitiesCount * availableChannelsCountPerEntity * WorldConfig::MaxChannelValue;
	}

	inline static WorldConfig GetNextLevelWorldConfig(const WorldConfig& worldConfig)
	{
		ElementSize = std::clamp(ElementSize / 2.f, 1.f, std::numeric_limits<float>::max());

		const auto worldSize = GetWorldSizeFromWindowSize(worldConfig.originalWindowSizePixels, ElementSize);
		const int64_t entitiesCount = GetEntitiesCountFromWorldSize(worldSize);
		//const int32_t availableChannelsCountPerEntity = std::clamp(worldConfig.avaiableChannelsCountPerEntity + 1, 1, 3); // TODO (mid): Handle more channels
		const int32_t availableChannelsCountPerEntity = 1;
		const auto requiredExperienceToFinishCurrentLevel = 
			GetRequiredExperienceToFinishCurrentLevel(entitiesCount, availableChannelsCountPerEntity) + worldConfig.requiredExperienceToFinishCurrentLevel;

		const WorldConfig newWorldConfig
		{
			.worldSize = worldSize,
			.entitiesCount = entitiesCount,
			.originalWindowSizePixels = worldConfig.originalWindowSizePixels,
			.currentWindowSizePixels = worldConfig.currentWindowSizePixels,
			.windowSizeRatio = worldConfig.windowSizeRatio,
			.avaiableChannelsCountPerEntity = availableChannelsCountPerEntity,
			.requiredExperienceToFinishCurrentLevel = requiredExperienceToFinishCurrentLevel,
			.needsRecreateWorld = true,
			.needsRecreateRenderData = worldConfig.needsRecreateRenderData
		};

		return newWorldConfig;
	}
}
