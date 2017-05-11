#pragma once
#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{

struct EntityId
{
	EntityId(const EntityId&) = default;
	EntityId& operator=(const EntityId&) = default;
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

// TODO To be replaced by something loaded from the resource loader
using EntityDescription = void*;
class EntityManager
{
public:
	EntityId Spawn(const EntityDescription&);
	void Destroy(EntityId);
	bool IsAlive(EntityId);
private:
	stl::vector<uint16_t> m_Generation;
};

}