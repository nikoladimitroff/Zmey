#include <Zmey/Physics/PhysicsActor.h>

#include <PhysX/PxPhysics.h>
#include <PhysX/PxPhysicsAPI.h>
#include <Zmey/Modules.h>

namespace Zmey
{
namespace Physics
{

PhysicsActor::~PhysicsActor()
{
	m_Actor.release();
}
inline physx::PxVec3 ZmeyVectorToPxVector(const Zmey::Vector3& vec)
{
	return physx::PxVec3(vec.x, vec.y, vec.z);
}

void PhysicsActor::ApplyForce(const Zmey::Vector3& force)
{
	ASSERT(!m_IsStatic);
	static_cast<physx::PxRigidBody&>(m_Actor).addForce(ZmeyVectorToPxVector(force), physx::PxForceMode::eFORCE);
}
void PhysicsActor::ApplyImpulse(const Zmey::Vector3& impulse)
{
	ASSERT(!m_IsStatic);
	static_cast<physx::PxRigidBody&>(m_Actor).addForce(ZmeyVectorToPxVector(impulse), physx::PxForceMode::eIMPULSE);
}
PhysicsActor::PhysicsActor(physx::PxRigidActor& pxActor, bool isStatic)
	: m_Actor(pxActor)
	, m_IsStatic(isStatic)
{
}

}

}