#pragma once

#include <gtest/gtest.h>

#include "ecs/P2Entity.hpp"

#include "P2TestTypes.hpp"

namespace P2::ecs::tests
{
	class ECSEntityTests : public ::testing::Test
	{
	protected:

		static_assert(!std::is_default_constructible_v<Entity>);

	};

	TEST_F(ECSEntityTests, CreateTest)
	{
		const ID id = 45;
		const size_t componentsIndex = 230;
		const Entity entity{ id, componentsIndex };

		EXPECT_EQ(id, entity.getID());
		EXPECT_EQ(componentsIndex, entity.getComponentsIndex());
	}

	TEST_F(ECSEntityTests, OperatorCompareTest)
	{
		const Entity entity{ 0, 0 };

		ASSERT_EQ(entity, Entity(0, 0));
		ASSERT_NE(entity, Entity(1, 0));
		ASSERT_NE(entity, Entity(0, 1));
		ASSERT_NE(entity, Entity(1, 1));
	}
}