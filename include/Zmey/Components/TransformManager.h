#pragma once
#include <Zmey/Math/Math.h>
#include <Zmey/EntityManager.h>
#include <Zmey/Components/ComponentRegistryCommon.h>
#include <Zmey/Components/ComponentManager.h>

namespace Zmey
{

namespace Components
{
	
class TransformManager : public IComponentManager
{
	DECLARE_COMPONENT_MANAGER();
public:
	TransformManager();

	struct TransformInstance Lookup(EntityId);

	virtual size_t InitializeFromBlob(const tmp::vector<EntityId>&, const uint8_t*) override;
	virtual void Simulate(float deltaMs) override;
private:
	// TODO: Store all 3 vectors in sequential memory
	stl::vector<Vector3> m_Positions;
	stl::vector<Quaternion> m_Rotations;
	stl::vector<Vector3> m_Scales;
	stl::unordered_map<EntityId, size_t> m_EntityToIndex;
	friend struct TransformInstance;
};

struct TransformInstance
{
	inline TransformInstance(TransformManager& manager, EntityId id)
		: m_Manager(manager)
		, m_Entity(id)
	{
	}
	inline Vector3& Position() const { return m_Manager.m_Positions[m_Manager.m_EntityToIndex[m_Entity]]; }
	inline Vector3& Scale() const { return m_Manager.m_Scales[m_Manager.m_EntityToIndex[m_Entity]]; }
	inline Quaternion& Rotation() const { return m_Manager.m_Rotations[m_Manager.m_EntityToIndex[m_Entity]]; }
private:
	TransformManager& m_Manager;
	EntityId m_Entity;
};

}

}