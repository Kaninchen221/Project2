#pragma once

#include "P2LibConfig.hpp"
#include "P2Debug.hpp"
#include "P2Function.hpp"
#include "P2Utils.hpp"

#include "ecs/P2Entity.hpp"

#include <algorithm>

namespace P2
{
	class TypeLessVector;

	template<bool IsConst>
	class TypeLessVectorIteratorImpl
	{
	public:

		using Vector = std::conditional_t<IsConst, const TypeLessVector*, TypeLessVector*>;

		TypeLessVectorIteratorImpl(Vector vector, size_t startingIndex = 0) noexcept
			: vector(vector), currentIndex(startingIndex)
		{
			if (!Ensure(vector))
				Terminate();
		}

		TypeLessVectorIteratorImpl() noexcept = delete;
		TypeLessVectorIteratorImpl(const TypeLessVectorIteratorImpl<IsConst>& other) noexcept = default;
		TypeLessVectorIteratorImpl(TypeLessVectorIteratorImpl<IsConst>&& other) noexcept = default;

		TypeLessVectorIteratorImpl<IsConst>& operator = (const TypeLessVectorIteratorImpl<IsConst>& other) noexcept = default;
		TypeLessVectorIteratorImpl<IsConst>& operator = (TypeLessVectorIteratorImpl<IsConst>&& other) noexcept = default;

		~TypeLessVectorIteratorImpl() noexcept = default;

		bool operator == (const TypeLessVectorIteratorImpl<IsConst>& other) const noexcept;

		bool operator != (const TypeLessVectorIteratorImpl<IsConst>& other) const noexcept;

		TypeLessVectorIteratorImpl<IsConst>& operator ++ () noexcept;

		auto* operator * (this auto& self) noexcept;

	private:

		Vector vector{};
		size_t currentIndex = 0;

	};

	using TypeLessVectorIterator = TypeLessVectorIteratorImpl<false>;
	using TypeLessVectorConstIterator = TypeLessVectorIteratorImpl<true>;

	class TypeLessVector
	{
		inline static auto Logger = ConsoleLogger::CreateOrGet("P2::ecs::TypeLessVector");

	public:

		TypeLessVector() noexcept = delete;
		TypeLessVector(const TypeLessVector& other) noexcept = delete;
		TypeLessVector(TypeLessVector&& other) noexcept
			: typeID(other.typeID), destructor(other.destructor), typeSize(other.typeSize), isTriviallyDestructible(other.isTriviallyDestructible)
		{ 
			*this = std::move(other); 
		};

		TypeLessVector& operator = (const TypeLessVector& other) noexcept = delete;
		TypeLessVector& operator = (TypeLessVector&& other) noexcept
		{
			buffer = std::move(other.buffer);
			removedObjects = std::move(other.removedObjects);
			objectsCapacity = other.objectsCapacity;
			objectsCount = other.objectsCount;
			
			other.objectsCapacity = 0;
			other.objectsCount = 0;

			return *this;
		}

		~TypeLessVector() noexcept;

		template<class T>
		static TypeLessVector Create();

		void clear();

		// Return the index of added component
		template<class T>
		size_t add(T&& object);

		bool remove(size_t index);

		template<class T>
		void reserve(size_t count);

		template<class T, class F = void(*)(T&)>
		bool apply(this auto& self, F f);

		template<class T>
		auto get(this auto& self, size_t index);

		// The returned pointer can point to a removed object
		void* getPtr(size_t index) noexcept { return buffer.data() + (index * typeSize); }
		const void* getPtr(size_t index) const noexcept { return buffer.data() + (index * typeSize); }

		bool isValidIndex(size_t index) const noexcept;

		size_t getFirstValidIndex() const noexcept;

		// The returned index could point to a removed object
		size_t getLastIndex() const noexcept { return objectsCount + removedObjects.size() - 1; }

		template<class T>
		bool hasType() const noexcept;

		size_t getObjectsCount() const noexcept { return objectsCount; }

		bool isEmpty() const noexcept { return getObjectsCount() == 0; }

		size_t getObjectsCapacity() const noexcept { return objectsCapacity; }

		auto getTypeID() const noexcept { return typeID; }

		auto getTypeSize() const noexcept { return typeSize; }

		TypeLessVectorIterator begin() noexcept;
		TypeLessVectorIterator end() noexcept;

		TypeLessVectorConstIterator cbegin() const noexcept;
		TypeLessVectorConstIterator cend() const noexcept;

		const void* data() const noexcept { return buffer.data(); }

	private:

		using Buffer = std::vector<std::byte>;

		TypeLessVector(const ID& typeID, Function<void, void*> destructor, size_t typeSize, bool isTriviallyDestructible)
			: typeID(typeID), destructor(destructor), typeSize(typeSize), isTriviallyDestructible(isTriviallyDestructible)
		{
		}

		template<class T>
		void reallocateElements(size_t desiredObjectsCapacity);

		Buffer buffer;

		std::vector<size_t> removedObjects;

		size_t objectsCapacity = 0;
		size_t objectsCount = 0;

		// Type info
		const ID typeID;
		const size_t typeSize = 0;
		const bool isTriviallyDestructible = false;
		const Function<void, void*> destructor;
	};
}

namespace P2
{
	template<bool IsConst>
	bool TypeLessVectorIteratorImpl<IsConst>::operator==(const TypeLessVectorIteratorImpl<IsConst>& other) const noexcept
	{
		return vector == other.vector && currentIndex == other.currentIndex;
	}

	template<bool IsConst>
	bool TypeLessVectorIteratorImpl<IsConst>::operator!=(const TypeLessVectorIteratorImpl<IsConst>& other) const noexcept
	{
		return !operator==(other);
	}

	template<bool IsConst>
	TypeLessVectorIteratorImpl<IsConst>& TypeLessVectorIteratorImpl<IsConst>::operator++() noexcept
	{
		do
		{
			currentIndex++;
		} while (!vector->isValidIndex(currentIndex) && currentIndex < vector->getLastIndex() + 1);

		return *this;
	}

	template<bool IsConst>
	auto* TypeLessVectorIteratorImpl<IsConst>::operator*(this auto& self) noexcept
	{
		using ReturnT = std::conditional_t<IsConst, const void*, void*>;

		return ReturnT{ self.vector->getPtr(self.currentIndex) };
	}

	template<class T>
	TypeLessVector TypeLessVector::Create()
	{
		using ObjectT = CollapsePossibleBatcherT<T>;

		// Lambda that invokes destructor
		auto destructor = [](void* objectVoidPtr) 
		{
			ObjectT* objectPtr = reinterpret_cast<ObjectT*>(objectVoidPtr);
			std::destroy_at(objectPtr);
		};

		return TypeLessVector
		(
			ResolvePossibleComponentBatcher<T>(), destructor, sizeof(ObjectT), std::is_trivially_constructible_v<ObjectT>
		);
	}

	template<class T>
	size_t TypeLessVector::add(T&& object)
	{
		using Object = std::decay_t<T>;

		constexpr bool IsCopyConstructible = std::is_copy_constructible_v<Object>;
		constexpr bool IsMoveConstructible = std::is_move_constructible_v<Object>;

		static_assert(IsCopyConstructible || IsMoveConstructible, "TypeLessVector::add: Object must be copy or move constructible.");

		if (typeID != ResolvePossibleComponentBatcher<Object>())
		{
			Logger->error("TypeLessVector::add: Type mismatch. Expected typeID: {}, but got typeID: {}", typeID, GetTypeID<Object>());
			Ensure(false);
			return InvalidIndex;
		}

		size_t byteIndex = InvalidIndex;
		size_t objectIndex = InvalidIndex;

		/// Index for a new component at a released index
		if (!removedObjects.empty())
		{
			objectIndex = removedObjects.back();
			removedObjects.pop_back();
			byteIndex = objectIndex * typeSize;
		}
		else /// Index for a new component at the end
		{
			/// Call reallocation only when we really need it
			if (objectsCount + 1 > objectsCapacity)
			{
				reallocateElements<Object>((objectsCount + 1) * 2 /* growth factor */);
			}

			byteIndex = typeSize * objectsCount;
			objectIndex = byteIndex / typeSize;
		}

		++objectsCount;

		Object* location = reinterpret_cast<Object*>(&buffer[byteIndex]);

		if constexpr (IsMoveConstructible)
		{
			std::construct_at<Object, T&&>(location, std::forward<T>(object));
		}
		else if (IsCopyConstructible)
		{
			std::construct_at<Object, const T&>(location, std::forward<T>(object));
		}

		return objectIndex;
	}

	template<class T>
	void TypeLessVector::reserve(size_t desiredObjectsCapacity)
	{
		reallocateElements<T>(desiredObjectsCapacity);
	}

	template<class T, class F>
	inline bool TypeLessVector::apply(this auto& self, F f)
	{
		using Type = std::remove_reference_t<T>;

		if (GetTypeID<Type>() != self.getTypeID())
		{
			return false;
		}

		constexpr bool IsSelfConstV = std::is_const_v<std::remove_reference_t<decltype(self)>>;
		auto it = [&self = self]() { if constexpr (IsSelfConstV) { return self.cbegin(); } else { return self.begin(); } }();
		auto end = [&self = self]() { if constexpr (IsSelfConstV) { return self.cend(); } else { return self.end(); } }();

		for (it; it != end; ++it)
		{
			auto& element = 
				[](auto rawPtr) -> auto&
				{ 
					if constexpr (IsSelfConstV) { return *reinterpret_cast<const Type*>(rawPtr); } else { return *reinterpret_cast<Type*>(rawPtr); } 
				}(*it);

			std::invoke(f, element);
		}

		return true;
	}

	template<class T>
	auto TypeLessVector::get(this auto& self, size_t index)
	{
		using ReturnT = std::conditional_t<IsSelfConst<decltype(self)>(), const T*, T*>;
		using Object = std::decay_t<T>;

		auto& typeSize = self.typeSize;
		auto& removedObjects = self.removedObjects;
		auto& buffer = self.buffer;
		auto& objectsCount = self.objectsCount;
		auto& typeID = self.typeID;

		constexpr size_t objectSize = sizeof(Object);
		if (objectSize != typeSize)
			return static_cast<ReturnT>(nullptr);

		if (index >= objectsCount + removedObjects.size())
			return static_cast<ReturnT>(nullptr);

		if (ResolvePossibleComponentBatcher<Object>() != typeID)
			return static_cast<ReturnT>(nullptr);

		if (std::ranges::contains(removedObjects, index))
			return static_cast<ReturnT>(nullptr);

		const size_t offset = index * typeSize;

#	if P2_DEBUG
		if (offset + typeSize > buffer.size())
		{
			Logger->error("TypeLessVector::get: Invalid offset. Index: {}, Offset: {}", index, offset);
			Ensure(false); // Invalid offset
			return static_cast<ReturnT>(nullptr);
		}
#	endif // P2_DEBUG

		return reinterpret_cast<ReturnT>(buffer.data() + offset);
	}
	
	template<class T>
	inline bool TypeLessVector::hasType() const noexcept
	{
		return ResolvePossibleComponentBatcher<T>() == typeID;
	}

	template<class T>
	void TypeLessVector::reallocateElements(size_t desiredObjectsCapacity)
	{
		using Object = std::decay_t<T>;

		if (sizeof(Object) != typeSize)
			return;

		const size_t desiredSize = desiredObjectsCapacity * typeSize;

		if (desiredSize > buffer.capacity())
		{
			const auto newSize = static_cast<size_t>(desiredSize);
			Buffer newBuffer{ newSize, std::byte{} };

			// Destroy old objects and move the newBuffer to the buffer
			for (size_t objectIndex = 0; objectIndex < objectsCount + removedObjects.size(); ++objectIndex)
			{
				auto* element = get<Object>(objectIndex);
				// Element index can point at removed element
				if (!element)
					continue;

				const size_t byteIndex = objectIndex * typeSize;

				auto newObject = reinterpret_cast<Object*>(&newBuffer[byteIndex]);
				auto oldObject = reinterpret_cast<Object*>(&buffer[byteIndex]);
				std::construct_at(newObject, std::move(*oldObject));

				if (!std::is_trivially_destructible_v<T>)
					destructor.invoke(oldObject);
			}

			// Move the new vector to the old vector
			buffer = std::move(newBuffer);
			objectsCapacity = desiredObjectsCapacity;
		}
	}
}