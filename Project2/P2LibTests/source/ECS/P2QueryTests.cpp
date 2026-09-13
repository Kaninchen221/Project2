#pragma once

#include <gtest/gtest.h>

#include "P2TypeLessVector.hpp"

#include "ECS/P2Query.hpp"

#include "P2TestTypes.hpp"

namespace P2::ecs::tests
{
	using namespace P2::tests;

	class ECSQueryTests : public ::testing::Test
	{
	protected:

		World world;
		std::vector<Entity> entities;

		size_t expectedSpritesCount = 0;
		size_t expectedVelocitiesCount = 0;
		size_t expectedPositionsCount = 0;

		size_t expectedSpriteVelocitiePairsCount = 0;
		size_t expectedSpritePositionPairsCount = 0;

		void SetUp() override
		{
			fillWorldWithEntites();
		}

		void fillWorldWithEntites();
	};

	TEST_F(ECSQueryTests, GetComponentsCountTest)
	{
		const Query<Sprite> querySprites{ world };
		EXPECT_EQ(querySprites.getComponentCount(), expectedSpritesCount);

		const Query<Velocity> queryVelocities{ world };
		EXPECT_EQ(queryVelocities.getComponentCount(), expectedVelocitiesCount);

		const Query<Position> queryPositions{ world };
		EXPECT_EQ(queryPositions.getComponentCount(), expectedPositionsCount);

		const Query<Sprite, Velocity> querySpriteVelocity{ world };
		EXPECT_EQ(querySpriteVelocity.getTypeCount(), 2); /* Bacuse we have 2 component types in the query */
		EXPECT_EQ(
			querySpriteVelocity.getComponentCount(), 
			expectedSpriteVelocitiePairsCount * querySpriteVelocity.getTypeCount());

		const Query<Sprite, Position> querySpritePosition{ world };
		EXPECT_EQ(querySpriteVelocity.getTypeCount(), 2); /* Bacuse we have 2 component types in the query */
		EXPECT_EQ(
			querySpritePosition.getComponentCount(), 
			expectedSpritePositionPairsCount * querySpriteVelocity.getTypeCount());
	}

	TEST_F(ECSQueryTests, GetComponentPerTypeCountTest)
	{
		const Query<Velocity> queryVelocities{ world };
		EXPECT_EQ(
			queryVelocities.getComponentPerTypeCount(),
			expectedVelocitiesCount);

		const Query<Sprite, Velocity> querySpriteVelocity{ world };
		EXPECT_EQ(
			querySpriteVelocity.getComponentPerTypeCount(),
			expectedSpriteVelocitiePairsCount);
	}

	TEST_F(ECSQueryTests, AsterixOperatorTest)
	{
		// Take the first sprite from a const query for later check
		const ConstQuery<Sprite> constQuery{ world };
		const auto constBegin = constQuery.begin();
		const auto constSprite = *constBegin;

		// Take the first sprite from a non-const query
		const Query<Sprite> query{ world };
		const auto begin = query.begin();
		ASSERT_NE(begin, query.end());

		auto sprite = *begin;
		std::get<0>(sprite)->id = 600; // Modify the id value with random value

		// Check the id value to be sure that "operator *" is not returning a copy
		EXPECT_EQ(std::get<0>(sprite)->id, std::get<0>(*begin)->id);

		// Now check the constSprite to be sure that we didn't get a copy of the sprite
		EXPECT_EQ(std::get<0>(constSprite)->id, std::get<0>(*begin)->id);
	}

	TEST_F(ECSQueryTests, IteratorsSingleComponentTypeTest)
	{
		const Query<Sprite> query{ world };

		const QueryIterator<Sprite> begin = query.begin();
		const QueryIterator<Sprite> end = query.end();

		ASSERT_NE(begin, end);

		auto it = begin;
		size_t index = 0;
		for (index; index < expectedSpritesCount; ++index)
		{
			ASSERT_NE(it, end);

			auto [sprite] = *it;
			ASSERT_TRUE(sprite);
			EXPECT_EQ(sprite->id, index);
			++it;
		}

		++it;
		ASSERT_EQ(it, end);
	}

	TEST_F(ECSQueryTests, IteratorsMultipleComponentsTypesTest)
	{
		const Query<Sprite, Velocity> query{ world };

		const QueryIterator<Sprite, Velocity> begin = query.begin();
		const QueryIterator<Sprite, Velocity> end = query.end();

		ASSERT_NE(begin, end);

		for (auto it = begin; it != end; ++it)
		{
			auto [sprite, velocity] = *it;
			ASSERT_TRUE(sprite);
			ASSERT_TRUE(velocity);
		}
	}

	TEST_F(ECSQueryTests, ForRangeLoopTest)
	{
		const Query<Sprite> query{ world };
		size_t counter = 0;

		for (auto [sprite] : query)
		{
			++counter;
		}

		ASSERT_EQ(counter, expectedSpritesCount);
	}

	TEST_F(ECSQueryTests, GetTypeCountTest)
	{
		const Query<Sprite> spriteQuery{ world };
		ASSERT_EQ(spriteQuery.getTypeCount(), 1);

		const Query<Sprite, Velocity> secondQuery{ world };
		ASSERT_EQ(secondQuery.getTypeCount(), 2);
	}

	TEST_F(ECSQueryTests, IsEmptyTest)
	{
		World emptyWorld;
		const Query<Sprite> emptyQuery{ emptyWorld };
		ASSERT_TRUE(emptyQuery.isEmpty());

		const Query<Sprite> query{ world };
		ASSERT_FALSE(query.isEmpty());
	}
}

namespace P2::ecs::tests
{
	void ECSQueryTests::fillWorldWithEntites()
	{
		entities.push_back(world.spawn(Sprite{ 0 }));
		entities.push_back(world.spawn(Sprite{ 1 }));
		entities.push_back(world.spawn(Sprite{ 2 }));
		entities.push_back(world.spawn(Sprite{ -1 }));
		world.remove(entities.back());
		entities.pop_back();

		entities.push_back(world.spawn(Sprite{ 3 }));
		entities.push_back(world.spawn(Sprite{ 4 }));
		entities.push_back(world.spawn(Sprite{ -1 }, Velocity{ -1, -1 }));
		world.remove(entities.back());
		entities.pop_back();

		entities.push_back(world.spawn(Sprite{ 5 }, Velocity{ 5, 5 }));
		entities.push_back(world.spawn(Sprite{ 6 }, Velocity{ 6, 6 }));
		entities.push_back(world.spawn(Sprite{ 7 }, Velocity{ 7, 7 }));
		entities.push_back(world.spawn(Velocity{ -1, -1 })); // Impostor
		entities.push_back(world.spawn(Sprite{ 8 }, Velocity{ 8, 8 }));
		entities.push_back(world.spawn(Sprite{ -1 }, Position{ -1, -1 }));
		world.remove(entities.back());
		entities.pop_back();

		entities.push_back(world.spawn(Sprite{ 9 }, Position{ 9, 9 }));
		entities.push_back(world.spawn(Sprite{ 10 }, Position{ 10, 10 }));
		entities.push_back(world.spawn(Sprite{ -1 }, Position{ -1, -1 }));
		world.remove(entities.back());
		entities.pop_back();

		expectedSpritesCount = 11;
		expectedVelocitiesCount = 5;
		expectedPositionsCount = 2;

		expectedSpriteVelocitiePairsCount = 4;
		expectedSpritePositionPairsCount = 2;
	}

	TEST(ECSQueryTest, QueryComponentsThatDontExistTest)
	{
		World world;
		Query<Sprite> query{ world };
		for (auto [sprite] : query)
		{
			// Query returned non end iterator with empty world
			ASSERT_TRUE(false);
		}
	}
	
	TEST(ECSQueryTest, QueryFromConstWorldTest)
	{
		using IsConstFalse = std::false_type;
		static_assert(std::is_same_v<QueryIterator<Position>, QueryIteratorImpl<IsConstFalse, Position>>);
		static_assert(std::is_same_v<Query<Position>, QueryImpl<IsConstFalse, Position>>);

		using IsConstTrue = std::true_type;
		static_assert(std::is_same_v<ConstQueryIterator<Position>, QueryIteratorImpl<IsConstTrue, Position>>);
		static_assert(std::is_same_v<ConstQuery<Position>, QueryImpl<IsConstTrue, Position>>);

		World world;
		world.spawn(Position{});

		const World& constWorld = world;

		ConstQuery<Position> query{ constWorld };

		ASSERT_TRUE(query.getComponentCount() > 0);

		for (const auto [position] : query)
		{
			ASSERT_TRUE(position);
		}
	}

	TEST(ECSQueryTest, GetComponentsPackTest)
	{
		World world;

		const Position expectedPosition{ 23, 1 };

		world.spawn(Position{ expectedPosition }, Velocity{}, Sprite{});
		world.spawn(Position{ expectedPosition }, Velocity{});

		{ // Non-const query
			Query<Position, Velocity> query{ world };

			ASSERT_EQ(query.getComponentCount(), 4);

			std::vector<TypeLessVector*> componentsPack = query.getComponentsPack<Position>();
			ASSERT_EQ(componentsPack.size(), 2); // Because we have 2 archetypes

			for (auto components : componentsPack)
			{
				ASSERT_EQ(*components->get<Position>(0), expectedPosition);
			}
		}

		{ // ConstQuery
			ConstQuery<Position, Velocity> constQuery{ world };

			std::vector<const TypeLessVector*> componentsPack = constQuery.getComponentsPack<Position>();
			ASSERT_EQ(componentsPack.size(), 2); // Because we have 2 archetypes
		}
	}

	TEST(ECSQueryTest, OperatorSquareBracketsTest)
	{
		World world;
		const int64_t entitiesCount = 10;
		world.spawnBatch(entitiesCount, [](int64_t index) -> Sprite { return Sprite{ static_cast<int32_t>(index) }; });
		ASSERT_EQ(world.getEntitiesCount(), entitiesCount);

		ecs::ConstQuery<Sprite> query{ world };
		EXPECT_EQ(std::get<0>(*query[4])->id, 4);
		EXPECT_EQ(std::get<0>(*query[8])->id, 8);
	}
}