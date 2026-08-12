#pragma once

#include "P2LibConfig.hpp"
#include "P2TypeLessVector.hpp"
#include "P2Entity.hpp"
#include "P2Utils.hpp"

#include <algorithm>
#include <ranges>

namespace P2::ecs
{
	class P2_API Archetype
	{
		inline static auto Logger = ConsoleLogger::CreateOrGet("P2::ecs::Archetype");

	public:

		using Entities = std::vector<Entity>;

		Archetype(Archetype&& other) noexcept = default;
		Archetype& operator = (Archetype&& other) noexcept = default;

		~Archetype() noexcept = default;

		template<class... Components>
		static Archetype Create();

		template<class... Components>
		size_t add(const Entity& entity, Components&&... components);

		template<class... Components>
		void addBatch(ID firstEntityID, size_t count, Components&&... components);

		template<class T>
		void reserve(size_t componentsCount);

		bool remove(const Entity& entity);

		bool hasEntity(const Entity& entity) const;
		
		template<class Component>
		auto* getComponentOfType(this auto& self, size_t index) noexcept;

		template<class Component>
		auto* getComponentsOfType(this auto& self) noexcept;

		template<class... Components>
		constexpr bool hasTypes() const noexcept;

		template<class... Components>
		constexpr bool typesEqual() const noexcept;

		size_t getEntitiesCount() const noexcept { return entities.size(); }

		size_t getComponentCount() const noexcept;

		auto& getEntities() const noexcept { return entities; }

	private:

		Archetype() noexcept = default;
		Archetype(const Archetype& other) noexcept = default;

		Archetype& operator = (const Archetype& other) noexcept = default;

		std::vector<TypeLessVector> componentsPack;

		std::vector<ID> types;

		Entities entities;

		template<class Component>
		size_t addSingleComponent(Component&& component)
		{
			auto components = getComponentsOfType<Component>();
			return components->add(std::forward<Component>(component));
		}
	};

	template<class... Components>
	Archetype Archetype::Create()
	{
		Archetype archetype;
		(archetype.componentsPack.push_back(TypeLessVector::Create<Components>()), ...);

		(archetype.types.push_back(ResolvePossibleComponentBatcher<Components>()), ...);

		return archetype;
	}

	template<class... Components>
	size_t Archetype::add(const Entity& entity, Components&&... components)
	{
		if (sizeof...(Components) != componentsPack.size())
			return InvalidIndex;

		size_t index = InvalidIndex;
		((index = addSingleComponent(std::forward<Components>(components))), ...);

		if (index != InvalidIndex)
			entities.emplace_back(entity.getID(), index);

		return index;
	}

	template<class... Components>
	void Archetype::addBatch([[maybe_unused]] ID firstEntityID, size_t count, Components&&... components)
	{
		auto reserveComponents = [&Logger = Logger, &self = *this, &count = count]<class ComponentT>(ComponentT&&)
		{
			auto* vector = self.getComponentsOfType<ComponentT>();
			if (!vector)
			{
				Logger->critical("Couldn't find a vector with components with ComponentT ID: {}", ResolvePossibleComponentBatcher<ComponentT>());
				return;
			}

			/// There is no much sense to check it every time but let's stay with it
			const auto freeCapacity = vector->getObjectsCapacity() - vector->getObjectsCount();
			if (freeCapacity < count)
			{
				vector->reserve<CollapsePossibleBatcher<ComponentT>>(count + vector->getObjectsCount());
			}
		};

		(reserveComponents(components), ...);

		auto addComponents = 
			[&Logger = Logger, &self = *this, &firstEntityID = firstEntityID, count = count]
			<class ComponentT>(ComponentT&& component)
			{
				Entities newEntities;
				newEntities.reserve(count);

				auto* vector = self.getComponentsOfType<ComponentT>();
				if (!vector)
				{
					Logger->critical("Couldn't find a vector with components with ComponentT ID: {}", ResolvePossibleComponentBatcher<ComponentT>());
					return Entities{};
				}

				for (size_t i = 0; i < count; ++i)
				{
					if constexpr (IsBatcher<ComponentT>())
					{
						const auto componentIndex = vector->add(std::invoke(component, i));
						newEntities.emplace_back(Entity{ firstEntityID + i, componentIndex });
					}
					else
					{
						const auto componentIndex = vector->add(std::forward<ComponentT>(component));
						newEntities.emplace_back(Entity{ firstEntityID + i, componentIndex });
					}
				}

				return newEntities;
			};

		/// TODO (very low): Most probably we are making here multiple assignments to the same variable
		/// But I'm not sure at 100% about this, but most probably yes
		Entities addedEntities;
		((addedEntities = addComponents(components)), ...);
		entities.append_range(addedEntities);
	}

	template<class T>
	void Archetype::reserve(size_t componentsCount)
	{
		for (auto& components : componentsPack)
		{
			components.reserve<T>(componentsCount);
		}
	}

	template<class Component>
	auto* Archetype::getComponentOfType(this auto& self, size_t index) noexcept
	{
		using ReturnT = std::conditional_t<IsSelfConst<decltype(self)>(), const Component, Component>;

		auto components = self.template getComponentsOfType<Component>();
		if (!components)
			return static_cast<ReturnT*>(nullptr);

		if (index >= components->getObjectsCount())
			return static_cast<ReturnT*>(nullptr);

		return components->get<ReturnT>(index);
	}

	template<class Component>
	auto* Archetype::getComponentsOfType(this auto& self) noexcept
	{
		using ReturnT = std::conditional_t<IsSelfConst<decltype(self)>(), const TypeLessVector, TypeLessVector>;

		for (auto& components : self.componentsPack)
		{
			if (components.hasType<Component>())
				return &components;
		}

		return static_cast<ReturnT*>(nullptr);
	}

	template<class... Components>
	constexpr bool Archetype::hasTypes() const noexcept
	{
		const std::vector<ID> wantedTypes
		{
			ResolvePossibleComponentBatcher<Components>()...
		};

		for (auto wantedType : wantedTypes)
		{
			if (!std::ranges::contains(types, wantedType))
				return false;
		}

		return true;
	}

	template<class... Components>
	constexpr bool Archetype::typesEqual() const noexcept
	{
		std::vector<ID> wantedTypes
		{
			ResolvePossibleComponentBatcher<Components>()...
		};

		for (const auto& type : types)
		{
			auto it = std::ranges::find(wantedTypes, type);
			if (it == wantedTypes.end())
				return false;

			wantedTypes.erase(it);
		}

		return wantedTypes.empty();
	}
}