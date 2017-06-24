#include <Zmey/Physics/PhysicsEngine.h>

#include <PhysX/PxPhysics.h>
#include <PhysX/PxPhysicsAPI.h>
#include <PhysXShared/foundation/PxAllocatorCallback.h>
#include <PhysX/common/PxTolerancesScale.h>
#include <PhysX/gpu/PxGpu.h>

#include <Zmey/Logging.h>
#include <Zmey/Modules.h>

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
	virtual void submitTask(physx::PxBaseTask& task) override
	{
		Zmey::Modules::TaskSystem->SpawnTask(task.getName(), [&task]()
		{
			task.run();
			task.release();
		});
	}

	virtual uint32_t getWorkerCount() const override
	{
		return Zmey::Modules::TaskSystem->GetWorkerCount();
	}
};


PhysicsEngine::PhysicsEngine()
	: m_Allocator(StaticAlloc<PhysicsAllocator>())
	, m_ErrorReporter(StaticAlloc<PhysicsErrorReporter>())
	, m_CpuDispatcher(StaticAlloc<PhysicsCpuDispatcher>())
{
	m_Foundation = PxCreateFoundation(PX_FOUNDATION_VERSION, *m_Allocator, *m_ErrorReporter);
	ASSERT_FATAL(m_Foundation);

	physx::PxTolerancesScale scale;
	bool recordMemoryAllocations = _DEBUG;
	m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, scale, recordMemoryAllocations, nullptr); // TODO: Add pvd connection for debugging
	ASSERT_FATAL(m_Physics);
	
	physx::PxSceneDesc sceneDesc(m_Physics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);

	sceneDesc.cpuDispatcher = m_CpuDispatcher.get();

	sceneDesc.frictionType = physx::PxFrictionType::eTWO_DIRECTIONAL;
	sceneDesc.frictionType = physx::PxFrictionType::eONE_DIRECTIONAL;
	sceneDesc.flags |= physx::PxSceneFlag::eENABLE_PCM;
	sceneDesc.flags |= physx::PxSceneFlag::eENABLE_STABILIZATION;
	sceneDesc.flags |= physx::PxSceneFlag::eENABLE_ACTIVETRANSFORMS;
	sceneDesc.flags |= physx::PxSceneFlag::eSUPPRESS_EAGER_SCENE_QUERY_REFIT;
	sceneDesc.broadPhaseType = physx::PxBroadPhaseType::eGPU;
	sceneDesc.gpuMaxNumPartitions = 8;

	sceneDesc.broadPhaseType = physx::PxBroadPhaseType::eMBP;

	m_Scene = m_Physics->createScene(sceneDesc);
	ASSERT_FATAL(m_Scene);
	SetupBroadphase();

	{
		physx::PxSceneWriteLock scopedLock(*m_Scene);
		m_Scene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.f);
		m_Scene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
	}
	LOG(Info, Physics, "Physics system initialized!");
}

void PhysicsEngine::SetupBroadphase()
{
	const float range = 1000.0f;
	physx::PxBroadPhaseRegion region;
	region.bounds.maximum = physx::PxVec3(range, range, range);
	region.bounds.minimum = -region.bounds.maximum;
	m_Scene->addBroadPhaseRegion(region);
}

void PhysicsEngine::Simulate(float deltaTime)
{
	TEMP_ALLOCATOR_SCOPE;
	uint32_t scratchMemorySize = 1024 * 1024; // 1mb
	tmp::unique_array<uint8_t> scratchMemory = tmp::make_unique_array<uint8_t>(scratchMemorySize);
	m_Scene->simulate(deltaTime, nullptr, scratchMemory.get(), scratchMemorySize);
}

void PhysicsEngine::FetchResults()
{
	physx::PxU32 error;
	m_Scene->fetchResults(true, &error);
	ASSERT(error == physx::PxErrorCode::eNO_ERROR);
}

}

}