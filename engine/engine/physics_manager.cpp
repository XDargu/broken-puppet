#include "mcv_platform.h"
#include "physics_manager.h"
#include <PxPhysicsAPI.h>
#include <foundation\PxFoundation.h>
#include <string>

using namespace physx;
using namespace DirectX;

static CPhysicsManager physics_manager;

static PxDefaultErrorCallback gDefaultErrorCallback;
static PxDefaultAllocator gDefaultAllocatorCallback;
static PxSimulationFilterShader gDefaultFilterShader = PxDefaultSimulationFilterShader;

CPhysicsManager& CPhysicsManager::get() {
	return physics_manager;
}

CPhysicsManager::CPhysicsManager() : gScene(NULL), timeStep( 1.f / 60.f ) // 60 Hz
{
}

CPhysicsManager::~CPhysicsManager()
{
}

void CPhysicsManager::init() {
	PxFoundation* foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
	
	assert(foundation || fatal("Fatal error creating Physx foundation"));

	gPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale());

	physx::PxVisualDebuggerConnectionFlags flags =
		physx::PxVisualDebuggerConnectionFlag::eDEBUG
		| physx::PxVisualDebuggerConnectionFlag::ePROFILE
		| physx::PxVisualDebuggerConnectionFlag::eMEMORY;

	// Visual debugger
	if (gPhysicsSDK->getPvdConnectionManager())
	{
		PxVisualDebuggerConnection* gConnection = PxVisualDebuggerExt::createConnection(gPhysicsSDK->getPvdConnectionManager(), "127.0.0.1", 5425, 1000, flags);

		gPhysicsSDK->getVisualDebugger()->setVisualizeConstraints(true);
		gPhysicsSDK->getVisualDebugger()->setVisualDebuggerFlag(physx::PxVisualDebuggerFlag::eTRANSMIT_CONTACTS, true);
		gPhysicsSDK->getVisualDebugger()->setVisualDebuggerFlag(physx::PxVisualDebuggerFlag::eTRANSMIT_CONSTRAINTS, true);
		gPhysicsSDK->getVisualDebugger()->setVisualDebuggerFlag(physx::PxVisualDebuggerFlag::eTRANSMIT_SCENEQUERIES, true);
	}	

	PxInitExtensions(*gPhysicsSDK);

	PxSceneDesc sceneDesc(gPhysicsSDK->getTolerancesScale());

	// Gravedad terrestre
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

	if (!sceneDesc.cpuDispatcher){
		PxDefaultCpuDispatcher* mCpuDispatcher = PxDefaultCpuDispatcherCreate(1);
		sceneDesc.cpuDispatcher = mCpuDispatcher;
	}

	// Filtro de colisiones
	if (!sceneDesc.filterShader)
		sceneDesc.filterShader = gDefaultFilterShader;

	gScene = gPhysicsSDK->createScene(sceneDesc);

	// Inicializar Cooking para colliders
	PxTolerancesScale toleranceScale;
	toleranceScale.mass = 1000;
	toleranceScale.speed = sceneDesc.gravity.y;
	bool value = toleranceScale.isValid(); // make sure this value is always true
	gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *foundation, PxCookingParams(toleranceScale));
	if (!gCooking)
		fatal("PxCreateCooking failed!\n");

	PxMaterial* mMaterial = gPhysicsSDK->createMaterial(0.5f, 0.5f, 0.1f);

	// Crear un suelo
	PxRigidStatic* plane = PxCreatePlane(*gPhysicsSDK, PxPlane(PxVec3(0, 1, 0), 0), *mMaterial);
	gScene->addActor(*plane);

	// Crear el controller manager
	gManager = PxCreateControllerManager(*gScene);
}

PxVec3 CPhysicsManager::XMVECTORToPxVec3(XMVECTOR vector) {
	return PxVec3(
		XMVectorGetX(vector),
		XMVectorGetY(vector),
		XMVectorGetZ(vector)
		);
}

XMVECTOR CPhysicsManager::PxVec3ToXMVECTOR(PxVec3 vector) {
	return XMVectorSet(vector.x, vector.y, vector.z, 1);
}

physx::PxQuat CPhysicsManager::XMVECTORToPxQuat(XMVECTOR quat) {
	return PxQuat(
		XMVectorGetX(quat),
		XMVectorGetY(quat),
		XMVectorGetZ(quat),
		XMVectorGetW(quat)
		);
}

XMVECTOR CPhysicsManager::PxQuatToXMVECTOR(physx::PxQuat quat) {
	return XMVectorSet(quat.x, quat.y, quat.z, quat.w);
}

std::string CPhysicsManager::toString(physx::PxVec3 vector) {
	return "(" + std::to_string(vector.x) + ", " + std::to_string(vector.y) + ", " + std::to_string(vector.z) + ")";
}

std::string CPhysicsManager::toString(physx::PxQuat quat) {
	return "(" + std::to_string(quat.x) + ", " + std::to_string(quat.y) + ", " + std::to_string(quat.z) + ", " + std::to_string(quat.w) + ")";
}

bool CPhysicsManager::raycast(PxVec3 origin, PxVec3 unit_dir, PxReal max_distance, PxRaycastBuffer &hit) {
	return gScene->raycast(origin, unit_dir, max_distance, hit);
}

bool CPhysicsManager::raycast(XMVECTOR origin, XMVECTOR unit_dir, PxReal max_distance, PxRaycastBuffer &hit) {
	return gScene->raycast(XMVECTORToPxVec3(origin), XMVECTORToPxVec3(unit_dir), max_distance, hit);
}