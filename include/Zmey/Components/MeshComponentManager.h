#pragma once
#include <Zmey/EntityManager.h>
#include <Zmey/Components/ComponentRegistryCommon.h>
#include <Zmey/Components/ComponentManager.h>
#include <Zmey/Graphics/GraphicsObjects.h>

namespace Zmey
{

namespace Components
{

class MeshComponentManager : public ComponentManager
{
	DECLARE_COMPONENT_MANAGER(MeshComponentManager);
public:
	virtual void InitializeFromBlob(const tmp::vector<EntityId>&, Zmey::MemoryInputStream&) override;
	virtual void Simulate(float deltaTime) override;
private:
	stl::vector<Graphics::MeshHandle> m_Meshes;
	stl::unordered_map<EntityId, EntityId::IndexType> m_EntityToIndex;
};

}

}