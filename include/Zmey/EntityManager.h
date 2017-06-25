#pragma once
#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{

struct EntityId
{
	EntityId()
		: EntityId(0xFFFFFFFFFFFFFFFF)
	{}

	EntityId(const EntityId& other) = default;
	EntityId& operator=(const EntityId& rhs)
	{
		std::memcpy(this, &rhs, sizeof(EntityId));
		return *this;
	}
	using IndexType = uint32_t;

	friend bool operator==(const EntityId& lhs, const EntityId& rhs)
	{
		return std::memcmp(&lhs, &rhs, sizeof(EntityId)) == 0;
	}

	// Conversion to / from double for scripting purposes
	explicit EntityId(double bits)
		: Index(0u)
		, Generation(0u)
		, UnusedBits(0u)
	{
		std::memcpy(const_cast<EntityId*>(this), &bits, sizeof(double));
	}
	operator double()
	{
		return *(double*)this;
	}

	explicit EntityId(uint64_t bits)
		: Index(0u)
		, Generation(0u)
		, UnusedBits(0u)
	{
		std::memcpy(const_cast<EntityId*>(this), &bits, sizeof(uint64_t));
	}

	static EntityId NullEntity()
	{
		EntityId nullEntity{ 0xFFFFFFFFFFFFFFFF };
		return nullEntity;
	}
private:
	EntityId(uint32_t index, uint16_t generation)
		: Index(index)
		, Generation(generation)
		, UnusedBits(0)
	{}
	const uint32_t Index;
	const uint16_t Generation;
	const uint16_t UnusedBits;
	friend class EntityManager;
};

class EntityManager
{
public:
	ZMEY_API EntityId SpawnOne();
	ZMEY_API tmp::vector<EntityId> SpawnRange(EntityId::IndexType count);
	ZMEY_API void Destroy(EntityId);
	ZMEY_API bool IsAlive(EntityId);
private:
	stl::vector<uint16_t> m_Generation;
	stl::queue<uint_fast32_t> m_FreeIndices;
};

}

namespace std
{
template<>
struct hash<Zmey::EntityId>
{
inline std::size_t operator()(const Zmey::EntityId& id) const
{
	return *reinterpret_cast<const size_t*>(&id);
}
};

}