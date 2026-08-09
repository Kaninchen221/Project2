#pragma once

#include "P2LibConfig.hpp"

#include "ECS/P2World.hpp"

namespace P2::ecs
{
	template<class IsConstType, class Type>
	class P2_API ResourceImpl
	{
	public:

		using IsResourceType = std::true_type;
		using IsConstT = IsConstType;
		using T = std::conditional_t<IsConstT{}, const std::decay_t<Type>, std::decay_t<Type>>;

		ResourceImpl(World& world)
			: resource{ world.getResource<T>() }
		{
		}

		ResourceImpl(const World& world)
			: resource{ world.getResource<T>() }
		{
		}

		ResourceImpl() noexcept = delete;
		ResourceImpl(const ResourceImpl& other) noexcept = default;
		ResourceImpl(ResourceImpl&& other) noexcept = default;
		~ResourceImpl() noexcept = default;

		ResourceImpl& operator = (const ResourceImpl& other) noexcept = default;
		ResourceImpl& operator = (ResourceImpl&& other) noexcept = default;

		operator bool() const noexcept { return resource; }

		T* operator -> () noexcept { return get(); }
		T const* operator -> () const noexcept { return get(); }

		T* get() noexcept { return resource; }
		T const* get() const noexcept { return resource; }
		
		T& operator * () noexcept { return *get(); }
		const T& operator * () const noexcept { return *get(); }

	private:

		T* resource{};

	};

	template<class Type>
	using Resource = ResourceImpl<std::false_type, Type>;

	template<class Type>
	using ConstResource = ResourceImpl<std::true_type, Type>;

}