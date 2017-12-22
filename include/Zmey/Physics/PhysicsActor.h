#pragma once
#include <Zmey/Config.h>
#include <Zmey/Math/Math.h>

namespace physx
{
class PxRigidActor;
}
namespace Zmey
{
class World;

namespace Physics
{

class PhysicsActor
{
public:
	~PhysicsActor();
	void ApplyForce(const Zmey::Vector3& force);
	void ApplyImpulse(const Zmey::Vector3& force);
private:
	PhysicsActor(physx::PxRigidActor& pxActor, bool isStatic);
	friend class PhysicsEngine;

	physx::PxRigidActor& m_Actor;
	bool m_IsStatic;
};

}
}