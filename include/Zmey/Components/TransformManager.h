#pragma once
#include <Zmey/Math/Math.h>
#include <Zmey/EntityManager.h>
#include <Zmey/Components/ComponentRegistryCommon.h>
#include <Zmey/Components/ComponentManager.h>

namespace Zmey
{

namespace Components
{
	
class TransformManager : public ComponentManager
{
	DECLARE_COMPONENT_MANAGER(TransformManager);
public:

	struct TransformInstance& Lookup(EntityId);

	void AddNewEntity(EntityId id, Vector3 pos, Vector3 scale, Quaternion rot);
	virtual void InitializeFromBlob(const tmp::vector<EntityId>&, Zmey::MemoryInputStream&) override;
	virtual void Simulate(float deltaTime) override;
private:
	// TODO: Store all 3 vectors in sequential memory
	stl::vector<Vector3> m_Positions;
	stl::vector<Quaternion> m_Rotations;
	stl::vector<Vector3> m_Scales;
	stl::unordered_map<EntityId, EntityId::IndexType> m_EntityToIndex;
	friend struct TransformInstance;
};

struct TransformInstance
{
	inline TransformInstance(TransformManager& manager, EntityId id)
		: m_Manager(manager)
		, m_Entity(id)
	{
	}
	inline Vector3& Position() const { return m_Manager.m_Positions[m_Manager.m_EntityToIndex.at(m_Entity)]; }
	inline Vector3& Scale() const { return m_Manager.m_Scales[m_Manager.m_EntityToIndex.at(m_Entity)]; }
	inline Quaternion& Rotation() const { return m_Manager.m_Rotations[m_Manager.m_EntityToIndex.at(m_Entity)]; }
private:
	TransformManager& m_Manager;
	EntityId m_Entity;
};

}

}