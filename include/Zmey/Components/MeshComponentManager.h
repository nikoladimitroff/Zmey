#pragma once
#include <Zmey/EntityManager.h>
#include <Zmey/Components/ComponentRegistryCommon.h>
#include <Zmey/Components/ComponentManager.h>
#include <Zmey/Graphics/GraphicsObjects.h>
#include <Zmey/Math/Math.h>

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
	virtual void RemoveEntity(EntityId id) override;

	//TODO(alex): remove this after visibility objects are created
	stl::vector<std::tuple<EntityId, Graphics::MeshHandle, Vector3>> GetMeshes();
private:
	stl::vector<Graphics::MeshHandle> m_Meshes;
	stl::vector<Vector3> m_MeshColors;
	stl::unordered_map<EntityId, EntityId::IndexType> m_EntityToIndex;
};

}

}