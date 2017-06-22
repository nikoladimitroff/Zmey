#include <Zmey/Physics/PhysicsEngine.h>

#include <PhysX/PxPhysics.h>
#include <PhysX/PxPhysicsAPI.h>
#include <PhysXShared/foundation/PxAllocatorCallback.h>

#include <Zmey/Memory/MemoryManagement.h>

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

PhysicsEngine::PhysicsEngine()
	: m_Allocator(StaticAlloc<PhysicsAllocator>())
	, m_ErrorReporter(StaticAlloc<PhysicsErrorReporter>())
{
	m_Foundation = PxCreateFoundation(PX_FOUNDATION_VERSION, *m_Allocator, *m_ErrorReporter);
	ASSERT_FATAL(m_Foundation);
//
//#if PX_SUPPORT_GPU_PHYSX
//	if (mCreateCudaCtxManager)
//	{
//		PxCudaContextManagerDesc cudaContextManagerDesc;
//
//#if defined(RENDERER_ENABLE_CUDA_INTEROP)
//		if (!mApplication.getCommandLine().hasSwitch("nointerop"))
//		{
//			switch (getRenderer()->getDriverType())
//			{
//			case Renderer::DRIVER_DIRECT3D11:
//				cudaContextManagerDesc.interopMode = PxCudaInteropMode::D3D11_INTEROP;
//				break;
//			case Renderer::DRIVER_OPENGL:
//				cudaContextManagerDesc.interopMode = PxCudaInteropMode::OGL_INTEROP;
//				break;
//			}
//			cudaContextManagerDesc.graphicsDevice = getRenderer()->getDevice();
//		}
//#endif 
//		mCudaContextManager = PxCreateCudaContextManager(*mFoundation, cudaContextManagerDesc);
//		if (mCudaContextManager)
//		{
//			if (!mCudaContextManager->contextIsValid())
//			{
//				mCudaContextManager->release();
//				mCudaContextManager = NULL;
//			}
//		}
//	}
//#endif
//
//	createPvdConnection();
//
//	PxTolerancesScale scale;
//	customizeTolerances(scale);
//
//	mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, scale, recordMemoryAllocations, mPvd);
//	if (!mPhysics)
//		fatalError("PxCreatePhysics failed!");
//
//	if (!PxInitExtensions(*mPhysics, mPvd))
//		fatalError("PxInitExtensions failed!");
//
//	PxCookingParams params(scale);
//	params.meshWeldTolerance = 0.001f;
//	params.meshPreprocessParams = PxMeshPreprocessingFlags(PxMeshPreprocessingFlag::eWELD_VERTICES);
//	params.buildGPUData = true; //Enable GRB data being produced in cooking.
//	mCooking = PxCreateCooking(PX_PHYSICS_VERSION, *mFoundation, params);
//	if (!mCooking)
//		fatalError("PxCreateCooking failed!");
//
//	mPhysics->registerDeletionListener(*this, PxDeletionEventFlag::eUSER_RELEASE);
//
//	// setup default material...
//	mMaterial = mPhysics->createMaterial(0.5f, 0.5f, 0.1f);
//	if (!mMaterial)
//		fatalError("createMaterial failed!");
//
//#if defined(RENDERER_TABLET)
//	// load touchscreen control material
//	{
//		SampleFramework::SampleAsset* ps_asset = getAsset(controlMaterialPath, SampleFramework::SampleAsset::ASSET_MATERIAL);
//		mManagedAssets.push_back(ps_asset);
//		PX_ASSERT(ps_asset->getType() == SampleFramework::SampleAsset::ASSET_MATERIAL);
//		SampleFramework::SampleMaterialAsset* mat_ps_asset = static_cast<SampleFramework::SampleMaterialAsset*>(ps_asset);
//		if (mat_ps_asset->getNumVertexShaders() > 0)
//		{
//			RenderMaterial* mat = SAMPLE_NEW(RenderMaterial)(*getRenderer(), mat_ps_asset->getMaterial(0), mat_ps_asset->getMaterialInstance(0), MATERIAL_CONTROLS);
//			mRenderMaterials.push_back(mat);
//		}
//	}
//	// load touchscreen button material
//	{
//		SampleFramework::SampleAsset* ps_asset = getAsset(buttonMaterialPath, SampleFramework::SampleAsset::ASSET_MATERIAL);
//		mManagedAssets.push_back(ps_asset);
//		PX_ASSERT(ps_asset->getType() == SampleFramework::SampleAsset::ASSET_MATERIAL);
//		SampleFramework::SampleMaterialAsset* mat_ps_asset = static_cast<SampleFramework::SampleMaterialAsset*>(ps_asset);
//		if (mat_ps_asset->getNumVertexShaders() > 0)
//		{
//			RenderMaterial* mat = SAMPLE_NEW(RenderMaterial)(*getRenderer(), mat_ps_asset->getMaterial(0), mat_ps_asset->getMaterialInstance(0), MATERIAL_BUTTONS);
//			mRenderMaterials.push_back(mat);
//		}
//	}
//	Renderer* renderer = getRenderer();
//	RenderMaterial* controlMaterial = getMaterial(MATERIAL_CONTROLS);
//	renderer->initControls(controlMaterial->mRenderMaterial,
//		controlMaterial->mRenderMaterialInstance);
//	RenderMaterial* buttonMaterial = getMaterial(MATERIAL_BUTTONS);
//	// add buttons for common use
//	PxReal yInc = -0.12f;
//	PxVec2 leftBottom(0.58f, 0.90f);
//	PxVec2 rightTop(0.99f, 0.82f);
//	renderer->addButton(leftBottom, rightTop, NULL,
//		buttonMaterial->mRenderMaterial, buttonMaterial->mRenderMaterialInstance);
//	leftBottom.y += yInc; rightTop.y += yInc;
//	renderer->addButton(leftBottom, rightTop, NULL,
//		buttonMaterial->mRenderMaterial, buttonMaterial->mRenderMaterialInstance);
//	leftBottom.y += yInc; rightTop.y += yInc;
//	renderer->addButton(leftBottom, rightTop, NULL,
//		buttonMaterial->mRenderMaterial, buttonMaterial->mRenderMaterialInstance);
//	leftBottom.y += yInc; rightTop.y += yInc;
//	renderer->addButton(leftBottom, rightTop, NULL,
//		buttonMaterial->mRenderMaterial, buttonMaterial->mRenderMaterialInstance);
//	leftBottom.y += yInc; rightTop.y += yInc;
//
//	// add buttons for individual sample
//	leftBottom.y += yInc; rightTop.y += yInc;
//	renderer->addButton(leftBottom, rightTop, NULL,
//		buttonMaterial->mRenderMaterial, buttonMaterial->mRenderMaterialInstance);
//	leftBottom.y += yInc; rightTop.y += yInc;
//	renderer->addButton(leftBottom, rightTop, NULL,
//		buttonMaterial->mRenderMaterial, buttonMaterial->mRenderMaterialInstance);
//	leftBottom.y += yInc; rightTop.y += yInc;
//	renderer->addButton(leftBottom, rightTop, NULL,
//		buttonMaterial->mRenderMaterial, buttonMaterial->mRenderMaterialInstance);
//	leftBottom.y += yInc; rightTop.y += yInc;
//	renderer->addButton(leftBottom, rightTop, NULL,
//		buttonMaterial->mRenderMaterial, buttonMaterial->mRenderMaterialInstance);
//	leftBottom.y += yInc; rightTop.y += yInc;
//	renderer->addButton(leftBottom, rightTop, NULL,
//		buttonMaterial->mRenderMaterial, buttonMaterial->mRenderMaterialInstance);
//
//	// quick access button
//	leftBottom.y += yInc; rightTop.y += yInc;
//	renderer->addButton(leftBottom, rightTop, NULL,
//		buttonMaterial->mRenderMaterial, buttonMaterial->mRenderMaterialInstance);
//
//	// next, previous buttons
//	leftBottom.x = -0.4f;
//	leftBottom.y = 0.9f;
//	rightTop.x = -0.1f;
//	rightTop.y = 0.82f;
//	renderer->addButton(leftBottom, rightTop, NULL,
//		buttonMaterial->mRenderMaterial, buttonMaterial->mRenderMaterialInstance);
//	leftBottom.x = 0.1f;
//	leftBottom.y = 0.9f;
//	rightTop.x = 0.4f;
//	rightTop.y = 0.82f;
//	renderer->addButton(leftBottom, rightTop, NULL,
//		buttonMaterial->mRenderMaterial, buttonMaterial->mRenderMaterialInstance);
//
//#endif		
//
//	PX_ASSERT(NULL == mScene);
//
//	PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
//	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
//	getDefaultSceneDesc(sceneDesc);
//
//
//
//	if (!sceneDesc.cpuDispatcher)
//	{
//		mCpuDispatcher = PxDefaultCpuDispatcherCreate(mNbThreads);
//		if (!mCpuDispatcher)
//			fatalError("PxDefaultCpuDispatcherCreate failed!");
//		sceneDesc.cpuDispatcher = mCpuDispatcher;
//	}
//
//	if (!sceneDesc.filterShader)
//		sceneDesc.filterShader = getSampleFilterShader();
//
//#if PX_SUPPORT_GPU_PHYSX
//	if (!sceneDesc.gpuDispatcher && mCudaContextManager)
//		sceneDesc.gpuDispatcher = mCudaContextManager->getGpuDispatcher();
//#endif
//
//	//sceneDesc.frictionType = PxFrictionType::eTWO_DIRECTIONAL;
//	//sceneDesc.frictionType = PxFrictionType::eONE_DIRECTIONAL;
//	sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
//	sceneDesc.flags |= PxSceneFlag::eENABLE_PCM;
//	//sceneDesc.flags |= PxSceneFlag::eENABLE_AVERAGE_POINT;
//	sceneDesc.flags |= PxSceneFlag::eENABLE_STABILIZATION;
//	//sceneDesc.flags |= PxSceneFlag::eADAPTIVE_FORCE;
//	sceneDesc.flags |= PxSceneFlag::eENABLE_ACTIVETRANSFORMS;
//	sceneDesc.flags |= PxSceneFlag::eSUPPRESS_EAGER_SCENE_QUERY_REFIT;
//	//sceneDesc.flags |= PxSceneFlag::eDISABLE_CONTACT_CACHE;
//	sceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;
//	sceneDesc.gpuMaxNumPartitions = 8;
//
//
//#ifdef USE_MBP
//	sceneDesc.broadPhaseType = PxBroadPhaseType::eMBP;
//#endif
//
//	customizeSceneDesc(sceneDesc);
//
//	mScene = mPhysics->createScene(sceneDesc);
//	if (!mScene)
//		fatalError("createScene failed!");
//
//	PxSceneWriteLock scopedLock(*mScene);
//
//	PxSceneFlags flag = mScene->getFlags();
//
//	PX_UNUSED(flag);
//	mScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, mInitialDebugRender ? mDebugRenderScale : 0.0f);
//	mScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
//
//	PxPvdSceneClient* pvdClient = mScene->getScenePvdClient();
//	if (pvdClient)
//	{
//		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
//		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
//		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
//	}
//
//#ifdef USE_MBP
//	setupMBP(*mScene);
//#endif
//
//	mApplication.refreshVisualizationMenuState(PxVisualizationParameter::eCOLLISION_SHAPES);
//	mApplication.applyDefaultVisualizationSettings();
//	mApplication.setMouseCursorHiding(false);
//	mApplication.setMouseCursorRecentering(false);
//	mCameraController.setMouseLookOnMouseButton(false);
//	mCameraController.setMouseSensitivity(1.0f);
//
//	if (mCreateGroundPlane)
//		createGrid();
//
//	LOG_INFO("PhysXSample", "Init sample ok!");
}

}

}