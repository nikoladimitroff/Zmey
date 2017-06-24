#pragma once
#include <Zmey/Config.h>
#include <Zmey/EntityManager.h>
#include <Zmey/Hash.h>
#include <Zmey/Memory/MemoryManagement.h>

namespace physx
{
class PxActor;
class PxDefaultCpuDispatcher;
class PxFoundation;
class PxMaterial;
class PxPhysics;
class PxScene;
class PxGeometryHolder;
}
namespace Zmey
{
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

	stl::unique_ptr<Geometry> CreateBoxGeometry(float width, float height, float depth) const;
	stl::unique_ptr<Geometry> CreateSphereGeometry(float radius) const;
	stl::unique_ptr<Geometry> CreateCapsuleGeometry(float radius, float height) const;
	void CreatePhysicsActor(EntityId, const PhysicsActorDescription&);
	void CreatePhysicsMaterial(Zmey::Name, const PhysicsMaterialDescription&);
	
	void Simulate(float deltaTime);
	void FetchResults();
private:
	// This struct is neccessary because we'd like to add some properties to the material
	// (such as density) instead of keeping them on the actor like physx does.
	struct CombinedMaterialInfo
	{
		physx::PxMaterial* PhysxMaterial;
		PhysicsMaterialDescription OriginalDescription;
	};

	void SetupBroadphase();
	const CombinedMaterialInfo* FindMaterial(Zmey::Name name) const;

	physx::PxPhysics* m_Physics;
	physx::PxFoundation* m_Foundation;
	physx::PxScene* m_Scene;
	physx::PxDefaultCpuDispatcher* m_DefaultCPUDispatcher;

	stl::unordered_map<EntityId, EntityId::IndexType> m_EntityToActor;
	stl::vector<physx::PxActor*> m_Actors;

	stl::vector<std::pair<Zmey::Name, CombinedMaterialInfo>> m_Materials;

	stl::unique_ptr<PhysicsAllocator> m_Allocator;
	stl::unique_ptr<PhysicsErrorReporter> m_ErrorReporter;
	stl::unique_ptr<PhysicsCpuDispatcher> m_CpuDispatcher;
};

}
}