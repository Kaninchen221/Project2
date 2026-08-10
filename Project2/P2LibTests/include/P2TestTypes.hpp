#pragma once

#include "P2Time.hpp"

#include "ECS/P2Query.hpp"
#include "ECS/P2Resource.hpp"
#include "ECS/P2WorldCommands.hpp"

#include "P2ExitReason.hpp"

#include <string>
#include <vector>
#include <cstdint>
#include <thread>

#include <gtest/gtest.h>

namespace P2::ecs
{
	class World;
}

namespace P2::tests
{
	struct Sprite
	{
		int id;
		bool operator == (const Sprite& other) const noexcept { return id == other.id; }
	};

	struct Position
	{
		float x;
		float y;

		bool operator == (const Position& other) const noexcept
		{
			return x == other.x && y == other.y;
		}
	};

	struct Velocity
	{
		float x;
		float y;

		bool operator == (const Velocity& other) const noexcept
		{
			return x == other.x && y == other.y;
		}
	};

	struct Counter
	{
		size_t value = 0;
	};

	// System must be a simple function
	// System can't contain any state, data and etc.
	// System can invoke other functions
	namespace TestSystem
	{
		struct Label {}; // Empty struct works as an unique ID for the system

		inline void doSomething() {}

		inline void entryPoint([[maybe_unused]] ecs::World& world) { doSomething();}
	}

	namespace TestSystemIncrementer
	{
		struct Label {};

		void entryPoint(ecs::Query<Counter> counters);
	}

	// Example of a resource class
	struct ResourceTime
	{
		Time time;

		bool operator == (const ResourceTime& other) const noexcept
		{
			return time == other.time;
		}
	};

	// Component/Resource with not trivial data
	class NotTrivialType
	{
		inline static auto Logger = ConsoleLogger::CreateOrGet("P2::ecs::tests::NotTrivialType");
		
		inline static int32_t ObjectsCounter = 0;

	public:

		static int32_t GetObjectsCounter() noexcept { return ObjectsCounter; }
		
		NotTrivialType() noexcept 
		{ 
			Logger->trace("Constructor");
			++ObjectsCounter;
		}

		NotTrivialType(const NotTrivialType& other) noexcept = delete;

		NotTrivialType(NotTrivialType&& other) noexcept 
		{ 
			Logger->trace("Move Constructor");
			*this = std::forward<NotTrivialType>(other); 
			++ObjectsCounter;
		}

		NotTrivialType& operator = (const NotTrivialType& other) = delete;

		NotTrivialType& operator = (NotTrivialType&& other) noexcept
		{
			Logger->trace("Move assign");
			name = std::move(other.name);
			data = std::move(other.data);
			description = std::move(other.description);
			return *this;
		}
		
		virtual ~NotTrivialType() noexcept 
		{ 
			Logger->trace("Virtual Destructor");
			--ObjectsCounter;
		}

		std::string name;
		std::vector<int32_t> data;
		std::string description;
	};

	class SystemTest_1
	{};

	class SystemTest_2
	{};

	class SystemTest_3
	{};

	class SystemTest_4
	{};

	class SystemTest_5
	{};

	class SystemTest_6
	{};

	class EmptySystemTest
	{
	public:
		static void EntryPoint() {}
	};

	class SleepSystemTest
	{
	public:
		static void Sleep1ms()
		{
			auto duration = std::chrono::milliseconds{ 1 };
			std::this_thread::sleep_for(duration);
		}
	};

	class ReadWritePositionResSystemTest
	{
	public:
		static void EntryPoint(ecs::Resource<Position>) {}
	};

	class ReadOnlyPositionResSystemTest
	{
	public:
		static void EntryPoint(ecs::ConstResource<Position>) {}
	};

	class ReadWritePositionVelocitySpriteComponentsSystemTest
	{
	public:
		static void EntryPoint(ecs::Query<Position, Velocity, Sprite>) {}
	};

	class ReadOnlyPositionVelocitySpriteComponentsSystemTest
	{
	public:
		static void EntryPoint(ecs::ConstQuery<Position, Velocity, Sprite>) {}
	};

	class AddComponentSystemTest
	{
	public:
		static void AddPosition(ecs::WorldCommands worldCommands)
		{
			worldCommands.spawn(Position{});
		}
	};

	class ExpectComponentSystemTest
	{
	public:
		static void ExpectPosition(ecs::ConstQuery<Position> positions, ecs::WorldCommands worldCommands)
		{
			if (positions.isEmpty())
			{
				ExitReason exitReason
				{
					.reason = "Expected non empty query",
					.level = ExitReason::Level::Error
				};

				worldCommands.addResource(exitReason);
			}
			else
			{
				ExitReason exitReason{ "", ExitReason::Info };
				worldCommands.addResource(exitReason);
			}
		}
	};

	class AddResourceSystemTest
	{
	public:
		static void AddPosition(ecs::WorldCommands worldCommands)
		{
			worldCommands.addResource(Position{});
		}
	};

	class ExpectResourceSystemTest
	{
	public:
		static void ExpectPosition(ecs::ConstResource<Position> positionRes, ecs::WorldCommands worldCommands)
		{ 
			if (!positionRes)
			{
				ExitReason exitReason
				{
					.reason = "Expected valid resource of class Position",
					.level = ExitReason::Level::Error
				};

				worldCommands.addResource(exitReason);
			}
			else
			{
				ExitReason exitReason{ "", ExitReason::Info };
				worldCommands.addResource(exitReason);
			}
		}
	};

	struct FailSystemTest
	{
		static void ExpectPositionAlwaysFailing(ecs::ConstResource<Position>)
		{
			EXPECT_FALSE(true);
		}
	};

	class NonMovableClass
	{
	public:
		explicit NonMovableClass(int value) : value{ value } {}

		NonMovableClass() noexcept = default;
		NonMovableClass(const NonMovableClass& other) noexcept = default;
		NonMovableClass(NonMovableClass&& other) noexcept = delete;

		NonMovableClass& operator = (const NonMovableClass& other) = default;
		NonMovableClass& operator = (NonMovableClass&& other) noexcept = delete;
		~NonMovableClass() noexcept = default;

		int value = 0;
	};

	class NonCopyableClass
	{
		NonCopyableClass(const NonCopyableClass& other) noexcept = delete;
		NonCopyableClass& operator = (const NonCopyableClass& other) = delete;

	public:
		explicit NonCopyableClass(int value) : value{ value } {}

		NonCopyableClass() noexcept = default;
		NonCopyableClass(NonCopyableClass&& other) noexcept { *this = std::move(other); };

		NonCopyableClass& operator = (NonCopyableClass&& other) noexcept { value = other.value; other.value = 0; return *this; }
		~NonCopyableClass() noexcept = default;

		int value = 0;
	};

	class TrivialClass
	{
	public:
		explicit TrivialClass(int value) : value{ value } {}

		TrivialClass() noexcept = default;
		TrivialClass(const TrivialClass& other) noexcept = default;
		TrivialClass(TrivialClass&& other) noexcept = default;

		TrivialClass& operator = (const TrivialClass& other) = default;
		TrivialClass& operator = (TrivialClass&& other) noexcept = default;
		~TrivialClass() noexcept = default;

		int value = 0;
	};

	class NonTrivialClass
	{
	public:
		explicit NonTrivialClass(int value) : value{ value } {}

		NonTrivialClass() noexcept = default;
		NonTrivialClass(const NonTrivialClass& other) noexcept = default;
		NonTrivialClass(NonTrivialClass&& other) noexcept = default;

		NonTrivialClass& operator = (const NonTrivialClass& other) = default;
		NonTrivialClass& operator = (NonTrivialClass&& other) noexcept = default;
		~NonTrivialClass() noexcept = default;

		int value = 0;
		std::vector<int> vector;
	};
}