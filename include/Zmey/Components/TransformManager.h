#pragma once
#include <Zmey/Math/Math.h>
#include <Zmey/EntityManager.h>

namespace Zmey
{

namespace Components
{
	
class TransformManager
{
public:
	void SpawnComponents(const tmp::vector<EntityId>&);
private:
	Vector3* m_Positions;
	float* m_Scales;
	Quaternion* m_Rotations;
	friend struct TransformInstance;
};

struct TransformInstance
{
	const Vector3& Position() const;
	const float Scale() const;
	const Quaternion& Rotation() const;
private:
	TransformManager& m_Manager;
	EntityId m_Entity;
};

}

}