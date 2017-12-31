#pragma once
#include <Zmey/Config.h>
#include <Zmey/EntityManager.h>
#include <Zmey/Hash.h>
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/Physics/PhysicsActor.h>

namespace physx
{
class PxActor;
class PxDefaultCpuDispatcher;
class PxFoundation;
class PxMaterial;
class PxPhysics;
class PxScene;
class PxGeometryHolder;
class PxPvd;
class PxPvdTransport;
}
namespace Zmey
{
class World;

namespace Physics
{
class PhysicsAllocator;
class PhysicsCpuDispatcher;
class PhysicsErrorReporter;

using PhysicsMaterialId = uint8_t;
using Geometry = physx::PxGeometryHolder;

struct PhysicsActorDescription
{
	Zmey::Name Material;
	bool IsStatic;
	bool IsTrigger;
	float Mass;
	Geometry* Geometry;
};

struct PhysicsMaterialDescription
{
	float Friction;
	float Restitution;
	float Density;
};

class PhysicsEngine
{
public:
	PhysicsEngine();
	~PhysicsEngine();

	using GeometryPtr = stl::unique_ptr<Geometry, stl::StdDestructorlessDeleter<Geometry>>;
	GeometryPtr CreateBoxGeometry(float width, float height, float depth) const;
	GeometryPtr CreateSphereGeometry(float radius) const;
	GeometryPtr CreateCapsuleGeometry(float radius, float height) const;
	stl::unique_ptr<PhysicsActor> CreatePhysicsActor(EntityId, const PhysicsActorDescription&);
	void CreatePhysicsMaterial(Zmey::Name, const PhysicsMaterialDescription&);
	
	void Simulate(float deltaTime);
	void FetchResults();
	void SetWorld(Zmey::World& world) { m_World = &world; }
private:
	// This struct is neccessary because we'd like to add some properties to the material
	// (such as density) instead of keeping them on the actor like physx does.
	struct CombinedMaterialInfo
	{
		physx::PxMaterial* PhysxMaterial;
		PhysicsMaterialDescription OriginalDescription;
	};
	template<typename T>
	struct PhysxDeleter
	{
		void operator()(T* physxObj)
		{
			physxObj->release();
		}
	};

	const CombinedMaterialInfo* FindMaterial(Zmey::Name name) const;
	void CreateDebuggerConnection();

	template<typename T>
	using physx_ptr = stl::unique_ptr<T, PhysxDeleter<T>>;
	physx_ptr<physx::PxPhysics> m_Physics;
	physx_ptr<physx::PxFoundation> m_Foundation;
	physx_ptr<physx::PxScene> m_Scene;
	physx_ptr<physx::PxDefaultCpuDispatcher> m_DefaultCPUDispatcher;
	physx_ptr<physx::PxPvd> m_VisualDebugger;
	physx_ptr<physx::PxPvdTransport> m_Transport;

	stl::vector<std::pair<Zmey::Name, CombinedMaterialInfo>> m_Materials;

	stl::unique_ptr<PhysicsAllocator> m_Allocator;
	stl::unique_ptr<PhysicsErrorReporter> m_ErrorReporter;
	stl::unique_ptr<PhysicsCpuDispatcher> m_CpuDispatcher;

	Zmey::World* m_World;

	static const float TimeStep;
	float m_TimeAccumulator;
	bool m_HasIssuedSimulate;
};

}
}