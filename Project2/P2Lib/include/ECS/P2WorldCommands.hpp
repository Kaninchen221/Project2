#pragma once

#include "P2LibConfig.hpp"
#include "ECS/P2World.hpp"
#include "ECS/P2Entity.hpp"

namespace P2::ecs
{
	class P2_API WorldCommands
	{
		inline static auto Logger = ConsoleLogger::CreateOrGet("P2::ecs::WorldCommands");

	public:

		using IsWorldCommandsType = std::true_type;

		WorldCommands(World& world) : world{ world } {}
		WorldCommands(const WorldCommands& other) noexcept = default;
		WorldCommands(WorldCommands&& other) noexcept = default;
		~WorldCommands() noexcept
		{
			world.addCommands(std::move(commands));
		}

		WorldCommands& operator = (const WorldCommands& other) noexcept = default;
		WorldCommands& operator = (WorldCommands&& other) noexcept = default;

		/// Entities & Components
		template<class... Components>
		void spawn(Components&&... components)
		{
			auto asTuple = std::tuple(std::forward<Components>(components)...);

			auto command = 
			[components = std::move(asTuple)]
			(World& world) mutable
			{
				std::apply([&](auto&&... args) 
				{
					world.spawn(std::forward<decltype(args)>(args)...);
				}, components);
			};

			commands.push_back(command);
		}

		void remove(const Entity& entity)
		{
			auto command = [entity](World& world) { world.remove(entity); };
			commands.push_back(command);
		}

		/// Resources
		// Resources are unique by type
		// User can't remove resources

		// TODO (low): Add param to define if the resource is open to public or it's private 
		// - Public: Any system can edit it (default)
		// - Private: Only the owner can edit it but everybody else can read it
		template<class ResourceT>
		void addResource(ResourceT&& resource)
		{
			using Type = std::remove_cvref_t<ResourceT>;

			auto command = [resource = std::forward<ResourceT>(resource)](World& world) mutable
			{
				world.addResource(std::forward<ResourceT>(resource));
			};

			commands.push_back(std::move(World::Command{ std::move(command) }));
		}

	protected:

		World& world;
		World::Commands commands;

	};
}