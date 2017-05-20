#pragma once

#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/EntityManager.h>

namespace Zmey
{
class MemoryInputStream;
namespace Components
{

class IComponentManager
{
public:
	virtual ~IComponentManager() {}

	virtual void InitializeFromBlob(const tmp::vector<EntityId>& entities, MemoryInputStream& blob) = 0;
	virtual void Simulate(float deltaMs) = 0;
};

}
}