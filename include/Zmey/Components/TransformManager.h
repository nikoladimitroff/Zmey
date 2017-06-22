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

	ZMEY_API struct TransformInstance Lookup(EntityId);

	ZMEY_API void AddNewEntity(EntityId id, Vector3 pos, Vector3 scale, Quaternion rot);
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
	inline TransformInstance(TransformManager& manager, EntityId::IndexType index)
		: m_Manager(manager)
		, m_EntityIndex(index)
	{
	}
	inline Vector3& Position() const { return m_Manager.m_Positions[m_EntityIndex]; }
	inline Vector3& Scale() const { return m_Manager.m_Scales[m_EntityIndex]; }
	inline Quaternion& Rotation() const { return m_Manager.m_Rotations[m_EntityIndex]; }
private:
	TransformManager& m_Manager;
	EntityId::IndexType m_EntityIndex;
};

}

}