#pragma once

#include "P2LibConfig.hpp"
#include "P2Utils.hpp"

#include "ECS/P2World.hpp"

#include <vector>
#include <tuple>

namespace P2::ecs
{
	template<class IsConstType, class... Components>
	class QueryIteratorImpl
	{
	public:

		using IsConstT = IsConstType;
		using Archetypes = std::conditional_t<IsConstT{}, std::vector<const Archetype*>, std::vector<Archetype*>>;

		QueryIteratorImpl(std::vector<Archetype*> archetypes, size_t currentArchetypeIndex, size_t currentEntityIndex)
			: archetypes{ archetypes },
			currentArchetypeIndex{ currentArchetypeIndex }, 
			currentEntityIndex{ currentEntityIndex }
		{}

		QueryIteratorImpl(std::vector<const Archetype*> archetypes, size_t currentArchetypeIndex, size_t currentEntityIndex)
			: archetypes{ archetypes },
			currentArchetypeIndex{ currentArchetypeIndex },
			currentEntityIndex{ currentEntityIndex }
		{
		}

		bool operator == (const QueryIteratorImpl& other) const noexcept;
		bool operator != (const QueryIteratorImpl& other) const noexcept { return !operator==(other); }

		QueryIteratorImpl& operator++ () noexcept;

		QueryIteratorImpl& operator+ (uint64_t offset) noexcept;

		auto operator* () const noexcept;

	private:
		
		Archetypes archetypes;
		size_t currentArchetypeIndex = 0;
		size_t currentEntityIndex = 0;

	};

	template<class IsConstType, class... Components>
	bool QueryIteratorImpl<IsConstType, Components...>::operator==(const QueryIteratorImpl& other) const noexcept
	{
		// TODO (mid): Compare archetypes, but comparing two vectors of pointers is causing a massive slowdown
		// The QueryIteratorImpl shouldn't have archetypes but only a pointer to archetypes, but this would require a refactor of the QueryImpl class
		//return archetypes == other.archetypes &&
			return currentArchetypeIndex == other.currentArchetypeIndex &&
			currentEntityIndex == other.currentEntityIndex;
	}

	template<class IsConstType, class... Components>
	QueryIteratorImpl<IsConstType, Components...>& QueryIteratorImpl<IsConstType, Components...>::operator++() noexcept
	{
		do
		{
			if (currentArchetypeIndex >= archetypes.size())
			{
				currentArchetypeIndex = InvalidIndex;
				currentEntityIndex = InvalidIndex;
				return *this;
			}

			auto* archetype = archetypes[currentArchetypeIndex];

			++currentEntityIndex;
			if (currentEntityIndex >= archetype->getEntitiesCount())
			{
				currentArchetypeIndex++;
				currentEntityIndex = 0;

				if (currentArchetypeIndex >= archetypes.size())
				{
					currentArchetypeIndex = InvalidIndex;
					currentEntityIndex = InvalidIndex;
					return *this;
				}

				archetype = archetypes[currentArchetypeIndex];
				if (currentEntityIndex < archetype->getEntitiesCount())
					return *this;

				continue;
			}

			return *this;
		}
		while (true);

		return *this;
	}

	template<class IsConstType, class ...Components>
	inline QueryIteratorImpl<IsConstType, Components...>& QueryIteratorImpl<IsConstType, Components...>::operator+(uint64_t offset) noexcept
	{
		do
		{
			if (currentArchetypeIndex >= archetypes.size())
			{
				currentArchetypeIndex = InvalidIndex;
				currentEntityIndex = InvalidIndex;
				return *this;
			}

			auto* archetype = archetypes[currentArchetypeIndex];

			if (currentEntityIndex + offset >= archetype->getEntitiesCount())
			{
				offset -= currentEntityIndex + archetype->getEntitiesCount();
				++currentArchetypeIndex;
				currentEntityIndex = 0;

				if (currentArchetypeIndex >= archetypes.size())
				{
					currentArchetypeIndex = InvalidIndex;
					currentEntityIndex = InvalidIndex;
					return *this;
				}

				archetype = archetypes[currentArchetypeIndex];
				continue;
			}
			currentEntityIndex += offset;

			return *this;
		} while (true);

		return *this;
	}
	
	template<class IsConstType, class ...Components>
	auto QueryIteratorImpl<IsConstType, Components...>::operator*() const noexcept
	{
		auto archetype = archetypes[currentArchetypeIndex];
		auto& entities = archetype->getEntities();

		auto& entity = entities[currentEntityIndex];

		using ReturnT = std::conditional_t < IsConstT{}, std::tuple<const Components*...>, std::tuple<Components*... >> ;
		return ReturnT{ archetype->getComponentOfType<Components>(entity.getComponentsIndex())... };
	}

	template<class... Components>
	using QueryIterator = QueryIteratorImpl<std::false_type, Components...>;

	template<class... Components>
	using ConstQueryIterator = QueryIteratorImpl<std::true_type, Components...>;

	template<class IsConstType, class... Components>
	class QueryImpl
	{
		inline static auto Logger = ConsoleLogger::CreateOrGet("P2::ecs::Query");

		QueryImpl() noexcept = default;

	public:

		using IsQueryType = std::true_type;
		using IsConstT = IsConstType;
		using ComponentsT = std::tuple<Components...>;

		using Archetypes = std::conditional_t<IsConstT{}, std::vector<const Archetype*>, std::vector<Archetype*>>;

		static_assert(std::tuple_size_v<ComponentsT> != 0);

		template<class WorldT>
		QueryImpl(WorldT& world)
		{
			static_assert(
				std::is_same_v<WorldT, World> ||
				std::is_same_v<WorldT, const World>,
				"You must pass the World class as a param");

			if constexpr (IsConstType{})
			{
				const World& constWorld = world;
				archetypes = constWorld.getArchetypesWith<Components...>();
			}
			else
			{
				archetypes = world.template getArchetypesWith<Components...>();
			}
		}

		QueryImpl(const QueryImpl& other) noexcept = default;
		QueryImpl(QueryImpl&& other) noexcept = default;
		~QueryImpl() noexcept = default;

		QueryImpl& operator = (const QueryImpl& other) noexcept = default;
		QueryImpl& operator = (QueryImpl&& other) noexcept = default;

		size_t getComponentCount() const noexcept;

		size_t getComponentPerTypeCount() const noexcept;

		constexpr size_t getTypeCount() const noexcept { return std::tuple_size_v<ComponentsT>; }

		bool isEmpty() const noexcept { return getComponentCount() == 0; }

		// Useful when you need to fill a buffer with components data
		// Return an array of TypeLessVectors
		template<class ComponentType>
		auto getComponentsPack(this auto& self);

		QueryIteratorImpl<IsConstT, Components...> begin() noexcept { return beginImpl(); }
		QueryIteratorImpl<IsConstT, Components...> begin() const noexcept { return beginImpl(); }

		QueryIteratorImpl<IsConstT, Components...> end() noexcept { return endImpl(); }
		QueryIteratorImpl<IsConstT, Components...> end() const noexcept { return endImpl(); }

		QueryIteratorImpl<IsConstT, Components...> operator [](size_t index) const 
		{
			return begin() + index;
		}

	private:

		auto beginImpl() const noexcept { return archetypes.empty() ? end() : QueryIteratorImpl<IsConstT, Components...>{ archetypes, 0, 0 }; }
		auto endImpl() const noexcept { return QueryIteratorImpl<IsConstT, Components...>{ archetypes, InvalidIndex, InvalidIndex }; }

		Archetypes archetypes;

	};

	template<class IsConstType, class... Components>
	size_t QueryImpl<IsConstType, Components...>::getComponentCount() const noexcept
	{
		size_t count = 0;
		for (const auto& archetype : archetypes)
		{
			if (!archetype)
			{
				Logger->critical("Query has an invalid archetype!");
				continue;
			}

			((count += archetype->getComponentsOfType<Components>()->getObjectsCount()), ...);
		}
		return count;
	}

	template<class IsConstType, class... Components>
	inline size_t QueryImpl<IsConstType, Components...>::getComponentPerTypeCount() const noexcept
	{
		// We shouldn't be able to create a Query<> without any types
		// So getTypeCount can't return 0
		return getComponentCount() / getTypeCount();
	}

	template<class IsConstType, class... Components>
	template<class ComponentType>
	auto QueryImpl<IsConstType, Components...>::getComponentsPack(this auto& self)
	{
		using ResultT = std::conditional_t<IsConstType{}, std::vector<const TypeLessVector*>, std::vector<TypeLessVector* >> ;

		using ComponentT = std::remove_cvref_t<ComponentType>;

		ResultT result;
		for (auto archetype : self.archetypes)
		{
			result.push_back(archetype->getComponentsOfType<ComponentT>());
		}

		return result;
	}

	template<class... Components>
	using Query = QueryImpl<std::false_type, Components...>;

	template<class... Components>
	using ConstQuery = QueryImpl<std::true_type, Components...>;
}