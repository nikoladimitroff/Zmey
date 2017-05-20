#pragma once
#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{

struct EntityId
{
	EntityId(const EntityId&) = default;
	EntityId& operator=(const EntityId&) = default;
	using IndexType = uint32_t;

	friend bool operator==(const EntityId& lhs, const EntityId& rhs)
	{
		return std::memcmp(&lhs, &rhs, sizeof(EntityId)) == 0;
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
	EntityId SpawnOne();
	tmp::vector<EntityId> SpawnRange(EntityId::IndexType count);
	void Destroy(EntityId);
	bool IsAlive(EntityId);
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