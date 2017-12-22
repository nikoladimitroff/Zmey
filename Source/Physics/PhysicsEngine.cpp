#include <Zmey/Physics/PhysicsEngine.h>

#include <PhysX/PxPhysics.h>
#include <PhysX/PxPhysicsAPI.h>
#include <PhysXShared/foundation/PxAllocatorCallback.h>
#include <PhysX/common/PxTolerancesScale.h>

#include <Zmey/Logging.h>
#include <Zmey/Modules.h>
#include <Zmey/Components/TransformManager.h>
#include <Zmey/Physics/PhysicsActor.h>
#include <Zmey/World.h>

#include <Windows.h> // Included for GetTempPath; TODO: move to a platform-indepedent header

namespace Zmey
{
namespace Physics
{

class PhysicsAllocator : public physx::PxAllocatorCallback
{
public:
	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line) override
	{
		return Zmey::GAllocator->Malloc(size, 16u);
	}
	virtual void deallocate(void* ptr) override
	{
		Zmey::GAllocator->Free(ptr);
	}
};

class PhysicsErrorReporter : public physx::PxErrorCallback
{
	inline Zmey::LogSeverity PxErrorCodeToSeverity(physx::PxErrorCode::Enum error)
	{
		switch (error)
		{
			case physx::PxErrorCode::eDEBUG_INFO:
				return Zmey::LogSeverity::Debug;
			case physx::PxErrorCode::eDEBUG_WARNING:
			case physx::PxErrorCode::ePERF_WARNING:
				return Zmey::LogSeverity::Warning;
			case physx::PxErrorCode::eINTERNAL_ERROR:
			case physx::PxErrorCode::eINVALID_OPERATION:
			case physx::PxErrorCode::eINVALID_PARAMETER:
			case physx::PxErrorCode::eOUT_OF_MEMORY:
				return Zmey::LogSeverity::Error;
			case physx::PxErrorCode::eABORT:
				return Zmey::LogSeverity::Fatal;
			default:
				NOT_REACHED();
				return Zmey::LogSeverity::Fatal;
		}
	}
	virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override
	{
		Zmey::GLogHandler->WriteLog(PxErrorCodeToSeverity(code), "Physics", message);
	}
};


class PhysicsCpuDispatcher : public physx::PxCpuDispatcher
{
public:
	static void CPUJobEntryPoint(void* data)
	{
		physx::PxBaseTask* task = reinterpret_cast<physx::PxBaseTask*>(data);

		task->run();
		task->release();
	}

	virtual void submitTask(physx::PxBaseTask& task) override
	{
		Job::JobDecl job{ CPUJobEntryPoint, &task };
		Zmey::Modules::JobSystem->RunJobs("PhysX CPU Task", &job, 1, nullptr);
	}

	virtual uint32_t getWorkerCount() const override
	{
		return 3; // TODO: get this from somewhere
	}
};


const float PhysicsEngine::TimeStep = 1 / 60.f;

PhysicsEngine::PhysicsEngine()
	: m_Allocator(StaticAlloc<PhysicsAllocator>())
	, m_ErrorReporter(StaticAlloc<PhysicsErrorReporter>())
	, m_CpuDispatcher(StaticAlloc<PhysicsCpuDispatcher>())
	, m_World(nullptr)
	, m_TimeAccumulator(0.f)
{
	m_Foundation.reset(PxCreateFoundation(PX_FOUNDATION_VERSION, *m_Allocator, *m_ErrorReporter));
	ASSERT_FATAL(m_Foundation);

	physx::PxTolerancesScale scale;
#if defined(_DEBUG)
	bool recordMemoryAllocations = true;
	CreateDebuggerConnection();
#elif defined(NDEBUG)
	bool recordMemoryAllocations = false;
#endif
	m_Physics.reset(PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, scale, recordMemoryAllocations, m_VisualDebugger.get()));
	ASSERT_FATAL(m_Physics);
	
	physx::PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);

	sceneDesc.cpuDispatcher = m_CpuDispatcher.get();
	sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;

	sceneDesc.frictionType = physx::PxFrictionType::eTWO_DIRECTIONAL;
	sceneDesc.frictionType = physx::PxFrictionType::eONE_DIRECTIONAL;
	sceneDesc.flags |= physx::PxSceneFlag::eENABLE_PCM;
	sceneDesc.flags |= physx::PxSceneFlag::eENABLE_STABILIZATION;
	sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVETRANSFORMS;
	sceneDesc.flags |= physx::PxSceneFlag::eSUPPRESS_EAGER_SCENE_QUERY_REFIT;

	sceneDesc.broadPhaseType = physx::PxBroadPhaseType::eSAP; // TODO: Switch to MBP

	m_Scene.reset(m_Physics->createScene(sceneDesc));
	ASSERT_FATAL(m_Scene);

	{
		physx::PxSceneWriteLock scopedLock(*m_Scene);
		m_Scene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.f);
		m_Scene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
	}
	LOG(Info, Physics, "Physics system initialized!");
}

void PhysicsEngine::CreateDebuggerConnection()
{
#if 0
	auto scope = TempAllocator::GetTlsAllocator().ScopeNow();
	auto neededSize = ::GetTempPathA(0, nullptr);
	tmp::string path;
	path.resize(neededSize);
	::GetTempPathA(neededSize, &path[0]);
	path.pop_back(); // GetTempPath adds a null-terminator so get rid of it
	path.append("Zmey.pxd2");
	physx::PxPvdTransport* transport = physx::PxDefaultPvdFileTransportCreate(path.c_str());
#endif

	//The normal way to connect to pvd.  PVD needs to be running at the time this function is called.
	//We don't worry about the return value because we are already registered as a listener for connections
	//and thus our onPvdConnected call will take care of setting up our basic connection state.
	char ip[] = "127.0.0.1";
	m_Transport.reset(physx::PxDefaultPvdSocketTransportCreate(ip, 5425, 1000));
	if (!m_Transport)
		return;

	//Use these flags for a clean profile trace with minimal overhead
	physx::PxPvdInstrumentationFlags flags = physx::PxPvdInstrumentationFlag::eALL;
#ifdef NDEBUG
	flags = physx::PxPvdInstrumentationFlag::ePROFILE;
#endif

	m_VisualDebugger.reset(physx::PxCreatePvd(*m_Foundation));
	m_VisualDebugger->connect(*m_Transport, flags);
}

void PhysicsEngine::Simulate(float deltaTime)
{
	m_TimeAccumulator += deltaTime;
	if (m_TimeAccumulator < PhysicsEngine::TimeStep)
	{
		return;
	}

	m_TimeAccumulator -= PhysicsEngine::TimeStep;
	TEMP_ALLOCATOR_SCOPE;
	uint32_t scratchMemorySize = 1024 * 1024; // 1mb
	tmp::unique_array<uint8_t> scratchMemory = tmp::make_unique_array<uint8_t>(scratchMemorySize);
	m_Scene->simulate(PhysicsEngine::TimeStep, nullptr, scratchMemory.get(), scratchMemorySize);
	m_HasIssuedSimulate = true;
}

inline void SetZmeyTransformFromPhysx(Zmey::Components::TransformInstance& transform, const physx::PxTransform& pxTransform)
{
	transform.Position().x = pxTransform.p.x;
	transform.Position().y = pxTransform.p.y;
	transform.Position().z = pxTransform.p.z;
	transform.Rotation().x = pxTransform.q.x;
	transform.Rotation().y = pxTransform.q.y;
	transform.Rotation().z = pxTransform.q.z;
	transform.Rotation().w = pxTransform.q.w;
}

inline void SetPhysxTransformFromZmey(physx::PxTransform& pxTransform, const Zmey::Components::TransformInstance& transform)
{
	pxTransform.p.x = transform.Position().x;
	pxTransform.p.y = transform.Position().y;
	pxTransform.p.z = transform.Position().z;
	pxTransform.q.x = transform.Rotation().x;
	pxTransform.q.y = transform.Rotation().y;
	pxTransform.q.z = transform.Rotation().z;
	pxTransform.q.w = transform.Rotation().w;
}

void PhysicsEngine::FetchResults()
{
	if (!m_HasIssuedSimulate)
	{
		return;
	}

	physx::PxU32 error(physx::PxErrorCode::eNO_ERROR);
	m_Scene->fetchResults(true, &error);
	ASSERT(error == physx::PxErrorCode::eNO_ERROR);
	m_HasIssuedSimulate = false;

	physx::PxU32 activeTransformsCount = -1;
	auto activeTransforms = m_Scene->getActiveTransforms(activeTransformsCount);
	auto& transformManager = m_World->GetManager<Zmey::Components::TransformManager>();
	for (physx::PxU32 i = 0; i < activeTransformsCount; ++i)
	{
		auto entityId = static_cast<Zmey::EntityId>(reinterpret_cast<uint64_t>(activeTransforms[i].userData));
		auto transform = transformManager.Lookup(entityId);
		// TODO This currently copies transforms as an array of structs; change to SoA.
		SetZmeyTransformFromPhysx(transform, activeTransforms[i].actor2World);
	}
}

PhysicsEngine::GeometryPtr PhysicsEngine::CreateBoxGeometry(float width, float height, float depth) const
{
	physx::PxBoxGeometry box(width / 2, height / 2, depth / 2);
	GeometryPtr result(new physx::PxGeometryHolder(std::move(box)));
	return result;
}
PhysicsEngine::GeometryPtr PhysicsEngine::CreateSphereGeometry(float radius) const
{
	physx::PxSphereGeometry sphere(radius);
	GeometryPtr result(new physx::PxGeometryHolder(std::move(sphere)));
	return result;
}
PhysicsEngine::GeometryPtr PhysicsEngine::CreateCapsuleGeometry(float radius, float height) const
{
	physx::PxCapsuleGeometry capsule(radius, height / 2);
	GeometryPtr result(new physx::PxGeometryHolder(std::move(capsule)));
	return result;
}

stl::unique_ptr<PhysicsActor> PhysicsEngine::CreatePhysicsActor(EntityId entityId, const PhysicsActorDescription& actorDescription)
{
	const CombinedMaterialInfo* material = FindMaterial(actorDescription.Material);
	ASSERT(material);
	physx::PxRigidActor* actor = nullptr;
	auto actorTransform = m_World->GetManager<Zmey::Components::TransformManager>().Lookup(entityId);
	physx::PxTransform currentTransform;
	SetPhysxTransformFromZmey(currentTransform, actorTransform);
	if (actorDescription.IsStatic)
	{
		actor = m_Physics->createRigidStatic(currentTransform);
	}
	else
	{
		actor = m_Physics->createRigidDynamic(currentTransform);
	}
	physx::PxShapeFlags flags = physx::PxShapeFlag::eSIMULATION_SHAPE | physx::PxShapeFlag::eVISUALIZATION |
		static_cast<physx::PxShapeFlag::Enum>(actorDescription.IsTrigger * physx::PxShapeFlag::eTRIGGER_SHAPE) |
		static_cast<physx::PxShapeFlag::Enum>(!actorDescription.IsTrigger * physx::PxShapeFlag::eSCENE_QUERY_SHAPE);
	physx::PxShape* shape = m_Physics->createShape(actorDescription.Geometry->any(), *material->PhysxMaterial, true, flags);
	actor->attachShape(*shape);
	actor->userData = reinterpret_cast<void*>(static_cast<uint64_t>(entityId));
	shape->release();
	m_Scene->addActor(*actor);

	stl::unique_ptr<Zmey::Physics::PhysicsActor> physicsActor(new Zmey::Physics::PhysicsActor(*actor, actorDescription.IsStatic));
	return physicsActor;
}
void PhysicsEngine::CreatePhysicsMaterial(Zmey::Name name, const PhysicsMaterialDescription& description)
{
	auto material = m_Physics->createMaterial(description.Friction, description.Friction, description.Restitution);
	m_Materials.push_back(std::make_pair(name, CombinedMaterialInfo{ material, description }));
}

const PhysicsEngine::CombinedMaterialInfo* PhysicsEngine::FindMaterial(Zmey::Name name) const
{
	auto it = std::find_if(m_Materials.begin(), m_Materials.end(), [name](const std::pair<Zmey::Name, CombinedMaterialInfo>& materialNamePair)
	{
		return materialNamePair.first == name;
	});
	if (it != m_Materials.end())
	{
		return &it->second;
	}
	return nullptr;
}

}

}