#pragma once
#include <Zmey/Config.h>
#include <Zmey/Memory/MemoryManagement.h>

namespace physx
{
class PxPhysics;
class PxFoundation;
}
namespace Zmey
{
namespace Physics
{
class PhysicsAllocator;
class PhysicsErrorReporter;

class PhysicsEngine
{
public:
	PhysicsEngine();
private:
	physx::PxPhysics* m_Physics;
	physx::PxFoundation* m_Foundation;
	stl::unique_ptr<PhysicsAllocator> m_Allocator;
	stl::unique_ptr<PhysicsErrorReporter> m_ErrorReporter;
};

}
}