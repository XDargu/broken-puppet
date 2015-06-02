#include "mcv_platform.h"
#include "physics_manager.h"
#include "callback_physics.h"
#include <PxPhysicsAPI.h>
#include <foundation\PxFoundation.h>
#include <string>

using namespace physx;
using namespace DirectX;

static CPhysicsManager physics_manager;

CCallbacks_physx gContactReportCallBack;
CFilterCallback gFilterCallback;
static PxDefaultErrorCallback gDefaultErrorCallback;
static PxDefaultAllocator gDefaultAllocatorCallback;
static PxSimulationFilterShader gDefaultFilterShader = FilterShader;

CPhysicsManager& CPhysicsManager::get() {
	return physics_manager;
}

CPhysicsManager::CPhysicsManager() : gScene(NULL), timeStep( 1.f / 60.f ) // 60 Hz
{
	m_collision = new std::map<physx::PxU32, std::vector<physx::PxU32>>();
	loadCollisions();
}

CPhysicsManager::~CPhysicsManager()
{
	delete m_collision;
	m_collision = nullptr;
}

void CPhysicsManager::loadCollisions() {
	// Parse xml file...
	CCollisionParser p;
	bool success = p.xmlParseFile("collision_table.xml");
	if (!success){
		//ERROR: METER XASSERT
		XASSERT(p.xmlParseFile("collision_table.xml"), "error leyendo tabla de colisiones");
	}
}

void CPhysicsManager::addCollisionFilter(physx::PxU32 s, physx::PxU32 filter){
	m_collision->operator[](s).push_back(filter);
	int prueba = 1;
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

	// Asignación de callback de colisiones
	sceneDesc.simulationEventCallback = &gContactReportCallBack;
	sceneDesc.filterCallback = &gFilterCallback;

	gScene = gPhysicsSDK->createScene(sceneDesc);
	gScene->setFlag(PxSceneFlag::eENABLE_CCD, true);

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
	/*PxRigidStatic* plane = PxCreatePlane(*gPhysicsSDK, PxPlane(PxVec3(0, 1, 0), 0), *mMaterial);
	gScene->addActor(*plane);*/

	//Particulas 
	// create particle system in PhysX SDKs
	//ps->setSimulationFilterData();
	//ps = gPhysicsSDK->createParticleSystem(maxParticles);
	// add particle system to scene, in case creation was successful
	//if (ps)
		//gScene->addActor(*ps);
	//particleCreationData.numParticles = 0;
	//ps->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	//indexPool = PxParticleExt::createIndexPool(maxParticles);

	// Crear el controller manager
	gManager = PxCreateControllerManager(*gScene);
}

void CPhysicsParticleSystem::init(PxU32 numParticles){
	maxParticles = numParticles;

	/*myIndexBuffer = new PxU32[maxParticles];
	myPositionBuffer = new PxVec3[maxParticles];
	myVelocityBuffer = new PxVec3[maxParticles];
	myParticlesForces = new PxVec3[maxParticles];*/

	ps = CPhysicsManager::get().gPhysicsSDK->createParticleSystem(maxParticles);
}

void CPhysicsParticleSystem::addParticles(int numNewParticles, PxU32 particlesToAdd[], PxVec3 positionsToAdd[], PxVec3 velocityToAdd[]){
	//Adding one or more particles to the particle system
	for (int i = 0; i < numNewParticles; i++){
		myIndexBuffer.push_back(particlesToAdd[i]);
		myPositionBuffer.push_back(positionsToAdd[i]);
		myVelocityBuffer.push_back(velocityToAdd[i]);
	}
}

bool CPhysicsParticleSystem::createParticles(PxU32 numParticles){
	//IMP: It should be noted that access to particles such as creating, updating, releasing, and
	//reading particle - related properties can only be done when the PhysX scene is not
	//being simulated

	//declare particle descriptor for creating new particles
	//PxParticleCreationData particleCreationData;

	//PxParticleExt::IndexPool* indexPool = PxParticleExt::createIndexPool(maxParticles);

	// create an index pool for a particle system with maximum particle count of maxParticles

	// use the indexPool for allocating numNewAppParticles indices that can be used
	// for particle creation throughout the particle system lifetime. If numAllocated
	// is smaller than numNewAppParticles, the maxParticles limit was exceeded
	//PxU32 numAllocated = indexPool->allocateIndices(maxParticles, PxStrideIterator<PxU32>(myIndexBuffer));

	maxParticles = numParticles;
	ps = CPhysicsManager::get().gPhysicsSDK->createParticleSystem(maxParticles);

	bool success = false;

	if (!myIndexBuffer.empty()){
		particleCreationData.numParticles = maxParticles;
		particleCreationData.indexBuffer =
			PxStrideIterator<const PxU32>(&myIndexBuffer[0]);
		particleCreationData.positionBuffer =
			PxStrideIterator<const PxVec3>(&myPositionBuffer[0]);
		particleCreationData.velocityBuffer =
			PxStrideIterator<const PxVec3>(&myVelocityBuffer[0]);



		// create particles in *PxParticleSystem* ps
		success = ps->createParticles(particleCreationData);

		if (ps)
			CPhysicsManager::get().gScene->addActor(*ps);
	}

	// create an index pool for a particle system with maximum particle count of maxParticles
	//PxParticleExt::IndexPool* indexPool = PxParticleExt::createIndexPool(maxParticles);

	return success;
}

void CPhysicsParticleSystem::updateParticles(){
	// declare strided iterator for providing array of indices corresponding to
	// particles that should be removed
	//PxU32 myIndexBuffer[] = { 0, 1, 2 };
	//PxVec3 appParticleForces[] = { PxVec3(0, 0, 0), PxVec3(0, 0, 0),
		//PxVec3(0, 0, 0) };
	// specify strided iterator to provide update forces
	PxStrideIterator<const PxVec3> forceBuffer(&myParticlesForces[0]);

	// specify strided iterator to provide indices of particles that need to be updated
	PxStrideIterator<const PxU32> indexBuffer(&myIndexBuffer[0]);

	// specify force update on PxParticleSystem ps choosing the "force" unit
	ps->addForces(maxParticles, indexBuffer, forceBuffer, PxForceMode::eFORCE);
}

void CPhysicsParticleSystem::releaseParticles(PxU32 indicesToRelease){
	//PxU32 myIndexBuffer[] = { 0, 1, 2 };
	//PxParticleExt::IndexPool* indexPool = PxParticleExt::createIndexPool(maxParticles);
	//indexPool->freeIndices(numAppParticleIndices, PxStrideIterator<PxU32>(indicesToRelease));
	PxStrideIterator<const PxU32> indexBuffer(&myIndexBuffer[0]);

	// release particles in *PxParticleSystem* ps
	ps->releaseParticles(indicesToRelease, indexBuffer);
}

void CPhysicsParticleSystem::releaseAllParticles(){
	//indexPool->release();
	ps->releaseParticles();
}

void CPhysicsParticleSystem::setParticlesFilterCollision(){
	PxFilterData filterData;
	filterData.word0 = FilterGroup::ePARTICLES;
	ps->setSimulationFilterData(filterData);
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

void CPhysicsManager::raycastAll(physx::PxVec3 origin, physx::PxVec3 unit_dir, physx::PxReal max_distance, physx::PxRaycastBuffer &hit) {
	const PxU32 bufferSize = 256;        // [in] size of 'hitBuffer'
	PxRaycastHit hitBuffer[bufferSize];  // [out] User provided buffer for results
	PxRaycastBuffer buf(hitBuffer, bufferSize); // [out] Blocking and touching hits will be stored here

	// Raycast against all static & dynamic objects (no filtering)
	// The main result from this call are all hits along the ray, stored in 'hitBuffer'
	gScene->raycast(origin, unit_dir, max_distance, buf);	
	hit = buf;
}

void CPhysicsManager::raycastAll(XMVECTOR origin, XMVECTOR unit_dir, physx::PxReal max_distance, physx::PxRaycastBuffer &hit) {
	const PxU32 bufferSize = 256;        // [in] size of 'hitBuffer'
	PxRaycastHit hitBuffer[bufferSize];  // [out] User provided buffer for results
	PxRaycastBuffer buf(hitBuffer, bufferSize); // [out] Blocking and touching hits will be stored here

	// Raycast against all static & dynamic objects (no filtering)
	// The main result from this call are all hits along the ray, stored in 'hitBuffer'
	gScene->raycast(XMVECTORToPxVec3(origin), XMVECTORToPxVec3(unit_dir).getNormalized(), max_distance, buf);
	hit = buf;
}

bool CPhysicsManager::sweepTest(PxGeometry sweepShape, PxTransform initialPose, PxVec3 unit_dir, PxReal max_distance, PxSweepBuffer &hit) {
	return gScene->sweep(sweepShape, initialPose, unit_dir, max_distance, hit);
}

bool CPhysicsManager::sweepTest(PxGeometry sweepShape, TTransform initialPose, XMVECTOR unit_dir, PxReal max_distance, PxSweepBuffer &hit) {
	return gScene->sweep(sweepShape, transformToPxTransform(initialPose), XMVECTORToPxVec3(unit_dir), max_distance, hit);
}

void CPhysicsManager::sweepTestAll(PxGeometry sweepShape, PxTransform initialPose, PxVec3 unit_dir, PxReal max_distance, PxSweepBuffer &hit) {
	const PxU32 bufferSize = 256;        // [in] size of 'hitBuffer'
	PxSweepHit hitBuffer[bufferSize];  // [out] User provided buffer for results
	PxSweepBuffer s_buf(hitBuffer, bufferSize); // [out] Blocking and touching hits will be stored here

	gScene->sweep(sweepShape, initialPose, unit_dir, max_distance, s_buf);
	hit = s_buf;
}

void CPhysicsManager::sweepTestAll(PxGeometry sweepShape, TTransform initialPose, XMVECTOR unit_dir, PxReal max_distance, PxSweepBuffer &hit) {
	const PxU32 bufferSize = 256;        // [in] size of 'hitBuffer'
	PxSweepHit hitBuffer[bufferSize];  // [out] User provided buffer for results
	PxSweepBuffer s_buf(hitBuffer, bufferSize); // [out] Blocking and touching hits will be stored here

	gScene->sweep(sweepShape, transformToPxTransform(initialPose), XMVECTORToPxVec3(unit_dir), max_distance, s_buf);
	hit = s_buf;
}


PxTransform CPhysicsManager::transformToPxTransform(TTransform the_transform) {
	return PxTransform(
		XMVECTORToPxVec3(the_transform.position),
		XMVECTORToPxQuat(the_transform.rotation)
		);
}
TTransform CPhysicsManager::pxTransformToTransform(PxTransform the_transform) {
	return TTransform(
		PxVec3ToXMVECTOR(the_transform.p),
		PxQuatToXMVECTOR(the_transform.q),
		XMVectorSet(1, 1, 1, 1)
	);
}

//Asignación de mascaras para filtrar las colisiones por Actores
void setupFiltering(PxRigidActor* actor, PxU32 filterGroup, PxU32 filterMask)
{
	PxFilterData filterData;
	filterData.word0 = filterGroup;
	filterData.word1 = filterMask;
	const PxU32 numShapes = actor->getNbShapes();
	PxShape** shapes = (PxShape**)malloc(sizeof(PxShape*)*numShapes);
	actor->getShapes(shapes, numShapes);
	for (PxU32 i = 0; i < numShapes; i++)
	{
		PxShape* shape = shapes[i];
		shape->setSimulationFilterData(filterData);
		shape->setQueryFilterData(filterData);
	}
	free(shapes);
}

//Asignación de mascaras para filtrar las colisiones por Shapes
void setupFiltering(PxShape* shape, PxU32 filterGroup, PxU32 filterMask)
{
	PxFilterData filterData;
	filterData.word0 = filterGroup;
	filterData.word1 = filterMask;
	shape->setSimulationFilterData(filterData);
	shape->setQueryFilterData(filterData);
	//free(shape);
}

PxU32 convertStrInCollisionFilter(std::string name){
	PxU32 result;
	if (name == "Enemy"){
		result = FilterGroup::eENEMY;
	}else if (name == "Player"){
		result = FilterGroup::ePLAYER;
	}else if (name == "Actor"){
		result = FilterGroup::eACTOR;
	}else if (name == "Level"){
		result = FilterGroup::eLEVEL;
	}else if (name == "Enemy_Ragdoll"){
		result = FilterGroup::eENEMY_RG;
	}else if (name == "Player_Ragdoll"){
		result = FilterGroup::ePLAYER_RG;
	}else if (name == "Actor_non_collision"){
		result = FilterGroup::eACTOR_NON_COLLISION;
	}else{
		result = FilterGroup::eUNDEFINED;
	}
	return result;
}
