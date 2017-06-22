#pragma once
#include <Zmey/Config.h>
#include <Zmey/Memory/MemoryManagement.h>

namespace physx
{
class PxPhysics;
class PxFoundation;
class PxScene;
class PxDefaultCpuDispatcher;
}
namespace Zmey
{
namespace Physics
{
class PhysicsAllocator;
class PhysicsErrorReporter;
class PhysicsCpuDispatcher;

class PhysicsEngine
{
public:
	PhysicsEngine();

	void Tick(float deltaTime);
private:
	void SetupBroadphase();

	physx::PxPhysics* m_Physics;
	physx::PxFoundation* m_Foundation;
	physx::PxScene* m_Scene;
	physx::PxDefaultCpuDispatcher* m_DefaultCPUDispatcher;
	stl::unique_ptr<PhysicsAllocator> m_Allocator;
	stl::unique_ptr<PhysicsErrorReporter> m_ErrorReporter;
	stl::unique_ptr<PhysicsCpuDispatcher> m_CpuDispatcher;
};

}
}