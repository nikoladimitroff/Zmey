#pragma once

#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/EntityManager.h>

namespace Zmey
{
namespace Components
{

class IComponentManager
{
public:
	virtual ~IComponentManager() {}

	virtual size_t InitializeFromBlob(const tmp::vector<EntityId>& entities, const uint8_t* blob) = 0;
	virtual void Simulate(float deltaMs) = 0;
};

}
}