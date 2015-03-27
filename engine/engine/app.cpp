#include "mcv_platform.h"
#include "app.h"
#include "camera.h"
#include "render/render_utils.h"
#include "entity_manager.h"
#include "doom_controller.h"
#include "font/font.h"
#include "render/texture.h"
#include "importer_parser.h"
#include "options_parser.h"
#include "physics_manager.h"
#include "components\all_components.h"
#include "error\log.h"
#include <time.h>

using namespace DirectX;
#include "render/ctes/shader_ctes.h"
#include "render/render_manager.h"

#include <PxPhysicsAPI.h>
#include <foundation\PxFoundation.h>

using namespace physx;

#include <AntTweakBar.h>
#include "entity_inspector.h"


static CApp the_app;

CEntityManager &entity_manager = CEntityManager::get();
CPhysicsManager &physics_manager = CPhysicsManager::get();
deque<CHandle> strings;

#include "ai\ai_basic_patroller.h"
#include "io\iostatus.h"

CApp& CApp::get() {
	return the_app;
}

CApp::CApp()
	: xres(640)
	, yres(480)
{ }

void CApp::loadConfig() {
	// Parse xml file...
	COptionsParser p;
	bool success = p.xmlParseFile("resolution.xml");
	if (success){
		int x_res;
		int y_res;
		bool mode;
		p.getResolution(x_res, y_res, mode);
		xres = x_res;
		yres = y_res;
		fullscreen = mode;
	}
	else{
		xres = 1024;
		yres = 768;
	}


}

// Debug 
CRenderTechnique debugTech;
CMesh        grid;
CMesh        axis;
CMesh		 wiredCube;
CMesh		 intersectsWiredCube;
CMesh		 rope;

CHandle		 life;

CHandle		  activeCamera;
CFont         font;

CShaderCte<TCtesGlobal> ctes_global;

float fixedUpdateCounter;

bool debug_mode;

void registerAllComponentMsgs() {
	//SUBSCRIBE(TCompLife, TMsgExplosion, onExplosion);
	SUBSCRIBE(TCompAiFsmBasic, TGroundHit, groundHit);
	SUBSCRIBE(TCompBasicPlayerController, TActorHit, actorHit);
	SUBSCRIBE(TCompBasicPlayerController, TMsgAttackDamage, onAttackDamage);
	SUBSCRIBE(TCompAiFsmBasic, TActorHit, actorHit);
	SUBSCRIBE(TCompAiFsmBasic, TMsgRopeTensed, onRopeTensed);
	SUBSCRIBE(TCompVictoryCond, TVictoryCondition, victory);
}

void createManagers() {
	CErrorContext ec("Creating", "managers");

	getObjManager<CEntity>()->init(1024);
	getObjManager<TCompTransform>()->init(1024);
	getObjManager<TCompLife>()->init(32);
	getObjManager<TCompName>()->init(1024);
	getObjManager<TCompMesh>()->init(1024);
	getObjManager<TCompRender>()->init(1024);
	getObjManager<TCompColliderMesh>()->init(512);
	getObjManager<TCompCamera>()->init(4);
	getObjManager<TCompCollider>()->init(512);
	getObjManager<TCompColliderSphere>()->init(512);
	getObjManager<TCompRigidBody>()->init(512);
	getObjManager<TCompStaticBody>()->init(512);
	getObjManager<TCompAABB>()->init(1024);
	getObjManager<TCompPlayerController>()->init(1);
	getObjManager<TCompPlayerPivotController>()->init(1);
	getObjManager<TCompCameraPivotController>()->init(1);
	getObjManager<TCompThirdPersonCameraController>()->init(1);
	getObjManager<TCompDistanceJoint>()->init(32);
	getObjManager<TCompJointPrismatic>()->init(32);
	getObjManager<TCompRope>()->init(32);
	getObjManager<TCompNeedle>()->init(1024);
	//PRUEBA TRIGGER
	getObjManager<TCompTrigger>()->init(1024);
	getObjManager<TCompDistanceText>()->init(32);
	getObjManager<TCompVictoryCond>()->init(1);
	//


	// Lights (temporary)
	getObjManager<TCompDirectionalLight>()->init(16);
	getObjManager<TCompAmbientLight>()->init(1);
	getObjManager<TCompPointLight>()->init(64);

	getObjManager<TCompAiFsmBasic>()->init(64);
	getObjManager<TCompEnemyController>()->init(64);

	getObjManager<TCompUnityCharacterController>()->init(64);
	getObjManager<TCompBasicPlayerController>()->init(1);

	registerAllComponentMsgs();
}

void initManagers() {
	CErrorContext ec("Initializing", "managers");

	getObjManager<TCompCamera>()->initHandlers();
	getObjManager<TCompCollider>()->initHandlers();
	getObjManager<TCompColliderSphere>()->initHandlers();
	getObjManager<TCompRigidBody>()->initHandlers();
	getObjManager<TCompStaticBody>()->initHandlers();
	getObjManager<TCompAABB>()->initHandlers();
	getObjManager<TCompUnityCharacterController>()->initHandlers();
	getObjManager<TCompPlayerController>()->initHandlers();
	getObjManager<TCompPlayerPivotController>()->initHandlers();
	getObjManager<TCompCameraPivotController>()->initHandlers();
	getObjManager<TCompThirdPersonCameraController>()->initHandlers();
	getObjManager<TCompDistanceJoint>()->initHandlers();
	getObjManager<TCompJointPrismatic>()->initHandlers();
	getObjManager<TCompEnemyController>()->initHandlers();

	//PRUEBA TRIGGER
	getObjManager<TCompTrigger>()->initHandlers();
	getObjManager<TCompDistanceText>()->initHandlers();
	getObjManager<TCompVictoryCond>()->initHandlers();

	getObjManager<TCompBasicPlayerController>()->initHandlers();
	getObjManager<TCompAiFsmBasic>()->initHandlers();
}

bool CApp::create() {
	CErrorContext ec("Creating", "app");

	// Log file configuration
	FILE* log = fopen("log.txt", "w");
	FILELog::ReportingLevel() = logERROR;
	FILELog::ReportingLevel() = logDEBUG;
	Output2FILE::Stream() = log;

	if (!::render.createDevice())
		return false;

	// Start random seed
	srand(time(NULL));

	// public delta time inicialization
	delta_time = 0.f;
	total_time = delta_time;
	fixedUpdateCounter = 0.0f;

	renderAABB = false;
	renderAxis = false;
	renderGrid = false;
	renderNames = false;
	debug_mode = false;

	createManagers();

	physics_manager.init();

	CImporterParser p;
	XASSERT(p.xmlParseFile("data/scenes/my_file.xml"), "Error loading the scene");
	//XASSERT(p.xmlParseFile("my_file.xml"), "Error loading the scene");
	//bool is_ok = p.xmlParseFile("data/scenes/scene_enemies.xml");

	initManagers();	

	// Create Debug Technique
	XASSERT(debugTech.load("basic"), "Error loading basic technique");
	
	CEntity* e = entity_manager.getByName("PlayerCamera");
	XASSERT(CHandle(e).isValid(), "Camera not valid");

	activeCamera = e->get<TCompCamera>();

	XASSERT(font.create(), "Error creating the font");
	font.camera = (TCompCamera*)activeCamera;

	// Ctes ---------------------------
	bool is_ok = renderUtilsCreate();

	//ctes_global.world_time = XMVectorSet(0, 0, 0, 0);
	ctes_global.get()->world_time = 0.f; // XMVectorSet(0, 0, 0, 0);
	is_ok &= ctes_global.create();
	XASSERT(is_ok, "Error creating global constants");

	// Create debug meshes
	is_ok &= createGrid(grid, 10);
	is_ok &= createAxis(axis);
	is_ok &= createUnitWiredCube(wiredCube, XMFLOAT4(1.f, 1.f, 1.f, 1.f));
	is_ok &= createUnitWiredCube(intersectsWiredCube, XMFLOAT4(1.f, 0.f, 0.f, 1.f));

	XASSERT(is_ok, "Error creating debug meshes");

#ifdef _DEBUG
	// Init AntTweakBar
	TwInit(TW_DIRECT3D11, ::render.device);
	TwWindowSize(xres, yres);

	entity_inspector.init();
	entity_inspector.inspectEntity(nullptr);

	entity_lister.init();
	entity_actioner.init();
	debug_optioner.init();
#endif

	activateInspectorMode(false);

	return true;
}

// -------------------------------------
void CApp::doFrame() {

	static LARGE_INTEGER before;
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	LARGE_INTEGER delta_ticks;
	delta_ticks.QuadPart = now.QuadPart - before.QuadPart;

	//delta_ticks.QuadPart *= 1000000;
	//delta_ticks.QuadPart /= freq.QuadPart;
	//double delta_secs = delta_ticks.QuadPart * 1e-6;
	float delta_secs = delta_ticks.QuadPart * (1.0f / freq.LowPart);
	delta_time = delta_secs;
	total_time += delta_secs;

	float fps = 1.0f / delta_secs;

	before = now;

	// To avoid the fist huge delta time
	if (delta_secs < 0.5) {		
		
		// Fixed update
		fixedUpdateCounter += delta_secs;

		while (fixedUpdateCounter > physics_manager.timeStep) {
			fixedUpdateCounter -= physics_manager.timeStep;
			fixedUpdate(physics_manager.timeStep);
		}

		update(delta_secs);
	}

	entity_manager.destroyRemovedHandles();
	render();
}

void CApp::update(float elapsed) {

	CIOStatus& io = CIOStatus::get();
	// Update input
	io.update(elapsed);

	//Acceso al componente player controller para mirar el número de tramas de hilo disponible
	CEntity* e = CEntityManager::get().getByName("Player");
#ifdef _DEBUG
	if (io.becomesReleased(CIOStatus::INSPECTOR_MODE)) {
		if (io.getMousePointer())
			activateInspectorMode(true);
		else
			activateInspectorMode(false);
	}

	if (io.becomesReleased(CIOStatus::DEBUG_MODE)) {
		if (!debug_mode)
			activateDebugMode(true);
		else
			activateDebugMode(false);
	}
#endif
	//Calculate the current number of strings
	unsigned int num_strings = numStrings();

	if (io.becomesReleased(CIOStatus::CANCEL_STRING)) {

		if (io.getTimePressed(CIOStatus::CANCEL_STRING) < .5f  && num_strings > 0) {
			CHandle c_rope = strings.back();
			strings.pop_back();
			entity_manager.remove(c_rope.getOwner());
		}		
	}

	if (io.isPressed(CIOStatus::CANCEL_STRING)) {
		if (io.getTimePressed(CIOStatus::CANCEL_STRING) >= .5f && num_strings > 0) {
			strings.clear();
			for (int i = 0; i < entity_manager.getEntities().size(); ++i)
			{
				TCompDistanceJoint* djoint = ((CEntity*)entity_manager.getEntities()[i])->get<TCompDistanceJoint>();

				if (djoint) {
					entity_manager.remove(CHandle(djoint).getOwner());
				}
			}
		}
	}

	if (io.becomesPressed(CIOStatus::TENSE_STRING)) {
		for (int i = 0; i < entity_manager.getEntities().size(); ++i)
		{
			TCompDistanceJoint* djoint = ((CEntity*)entity_manager.getEntities()[i])->get<TCompDistanceJoint>();

			if (djoint) {
				djoint->joint->setMaxDistance(0.1f);
				PxRigidActor* a1 = nullptr;
				PxRigidActor* a2 = nullptr;

				djoint->joint->getActors(a1, a2);
				// Wake up the actors, if dynamic
				if (a1->isRigidDynamic()) {
					((physx::PxRigidDynamic*)a1)->wakeUp();
					((CEntity*)entity_manager.getByName(a1->getName()))->sendMsg(TMsgRopeTensed(djoint->joint->getDistance()));
				}
				if (a2->isRigidDynamic()) {
					((physx::PxRigidDynamic*)a2)->wakeUp();
					((CEntity*)entity_manager.getByName(a2->getName()))->sendMsg(TMsgRopeTensed(djoint->joint->getDistance()));
				}
			}
		}
	}


	if (io.becomesPressed(CIOStatus::THROW_STRING)) {	

			// Get the camera position
			CEntity* e = CEntityManager::get().getByName("PlayerCamera");
			TCompTransform* t = e->get<TCompTransform>();

			// Raycast detecting the collider the mouse is pointing at
			PxRaycastBuffer hit;
			physics_manager.raycast(t->position, t->getFront(), 1000, hit);

			static int entitycount = 1;
			static PxRigidActor* firstActor = nullptr;
			static PxVec3 firstPosition = PxVec3(0, 0, 0);
			static PxVec3 firstOffset = PxVec3(0, 0, 0);
			static CHandle firstNeedle;
			if (hit.hasBlock) {
				PxRaycastHit blockHit = hit.block;
				//dbg("Click en un actor en: %f, %f, %f\n", blockHit.actor->getGlobalPose().p.x, blockHit.actor->getGlobalPose().p.y, blockHit.actor->getGlobalPose().p.z);
				//dbg("Punto de click: %f, %f, %f\n", blockHit.position.x, blockHit.position.y, blockHit.position.z);

				if (firstActor == nullptr) {
					firstActor = blockHit.actor;
					firstPosition = blockHit.position;
					firstOffset = firstActor->getGlobalPose().q.rotateInv(blockHit.position - firstActor->getGlobalPose().p);

					CEntity* new_e = entity_manager.createEmptyEntity();
					CEntity* rigidbody_e = entity_manager.getByName(firstActor->getName());

					TCompName* new_e_name = CHandle::create<TCompName>();
					std::strcpy(new_e_name->name, ("Needle" + to_string(entitycount)).c_str());
					new_e->add(new_e_name);

					TCompTransform* new_e_trans = CHandle::create<TCompTransform>();
					new_e->add(new_e_trans);
					new_e_trans->scale = XMVectorSet(2, 2, 2, 1);

					/*TCompMesh* new_e_mesh = CHandle::create<TCompMesh>();
					std::strcpy(new_e_mesh->path, "aguja");
					new_e_mesh->mesh = mesh_manager.getByName("aguja");
					new_e->add(new_e_mesh);*/

					TCompNeedle* new_e_needle = CHandle::create<TCompNeedle>();
					new_e->add(new_e_needle);
					XMVECTOR rotation;
					if (firstPosition == physics_manager.XMVECTORToPxVec3(t->position)) {
						XMMATRIX view = XMMatrixLookAtRH(t->position, t->position - (physics_manager.PxVec3ToXMVECTOR(firstPosition + physics_manager.XMVECTORToPxVec3(t->getFront() * 0.01f)) - t->position), XMVectorSet(0, 1, 0, 0));
						rotation = XMQuaternionInverse(XMQuaternionRotationMatrix(view));
					}
					else {
						XMMATRIX view = XMMatrixLookAtRH(t->position, t->position - (physics_manager.PxVec3ToXMVECTOR(firstPosition) - t->position), XMVectorSet(0, 1, 0, 0));
						rotation = XMQuaternionInverse(XMQuaternionRotationMatrix(view));
					}
					bool a = firstActor->isRigidDynamic();

					XMMATRIX view_normal = XMMatrixLookAtRH(physics_manager.PxVec3ToXMVECTOR(firstPosition - blockHit.normal), physics_manager.PxVec3ToXMVECTOR(firstPosition), XMVectorSet(0, 1, 0, 0));
					XMVECTOR normal_rotation = XMQuaternionInverse(XMQuaternionRotationMatrix(view_normal));
					XMVECTOR finalQuat = XMQuaternionSlerp(rotation, normal_rotation, 0.35f);

					new_e_needle->create(
						firstActor->isRigidDynamic() ? physics_manager.PxVec3ToXMVECTOR(firstOffset) : physics_manager.PxVec3ToXMVECTOR(firstPosition)
						, XMQuaternionMultiply(finalQuat, XMQuaternionInverse(physics_manager.PxQuatToXMVECTOR(firstActor->getGlobalPose().q)))
						, rigidbody_e->get<TCompRigidBody>()
						);

					firstNeedle = new_e;

				}
				else if (blockHit.actor != firstActor) {
					if (num_strings >= max_num_string){
						CHandle c_rope = strings.front();
						strings.pop_front();
						entity_manager.remove(c_rope.getOwner());
					}

					CEntity* new_e = entity_manager.createEmptyEntity();

					TCompName* new_e_name = CHandle::create<TCompName>();
					std::strcpy(new_e_name->name, ("Joint" + to_string(entitycount)).c_str());
					new_e->add(new_e_name);

					TCompDistanceJoint* new_e_j = CHandle::create<TCompDistanceJoint>();
					PxVec3 pos = firstActor->getGlobalPose().q.rotate(firstOffset) + firstActor->getGlobalPose().p;
					new_e_j->create(firstActor, blockHit.actor, 1, firstPosition, blockHit.position);

					// Obtener el offset con coordenadas de mundo = (Offset_mundo - posición) * inversa(rotación)			  
					PxVec3 offset_1 = firstOffset;//firstActor->getGlobalPose().q.rotateInv(firstPosition - firstActor->getGlobalPose().p);
					PxVec3 offset_2 = blockHit.actor->getGlobalPose().q.rotateInv(blockHit.position - blockHit.actor->getGlobalPose().p);

					new_e_j->joint->setLocalPose(PxJointActorIndex::eACTOR0, PxTransform(offset_1));
					new_e_j->joint->setLocalPose(PxJointActorIndex::eACTOR1, PxTransform(offset_2));

					new_e->add(new_e_j);

					TCompRope* new_e_r = CHandle::create<TCompRope>();
					new_e->add(new_e_r);
					new_e_r->create();

					// Needle
					CEntity* new_e2 = entity_manager.createEmptyEntity();
					CEntity* rigidbody_e = entity_manager.getByName(blockHit.actor->getName());

					TCompName* new_e_name2 = CHandle::create<TCompName>();
					std::strcpy(new_e_name2->name, ("Needle" + to_string(entitycount)).c_str());
					new_e2->add(new_e_name2);

					TCompTransform* new_e_trans2 = CHandle::create<TCompTransform>();
					new_e2->add(new_e_trans2);
					new_e_trans2->scale = XMVectorSet(2, 2, 2, 1);

					/*TCompMesh* new_e_mesh2 = CHandle::create<TCompMesh>();
					std::strcpy(new_e_mesh2->path, "aguja");
					new_e_mesh2->mesh = mesh_manager.getByName("aguja");
					new_e2->add(new_e_mesh2);*/

					TCompNeedle* new_e_needle2 = CHandle::create<TCompNeedle>();
					new_e2->add(new_e_needle2);
					XMVECTOR rotation;
					if (blockHit.position == physics_manager.XMVECTORToPxVec3(t->position)) {
						XMMATRIX view = XMMatrixLookAtRH(t->position, t->position - (physics_manager.PxVec3ToXMVECTOR(blockHit.position + physics_manager.XMVECTORToPxVec3(t->getFront() * 0.01f)) - t->position), XMVectorSet(0, 1, 0, 0));
						rotation = XMQuaternionInverse(XMQuaternionRotationMatrix(view));
					}
					else {
						XMMATRIX view = XMMatrixLookAtRH(t->position, t->position - (physics_manager.PxVec3ToXMVECTOR(blockHit.position) - t->position), XMVectorSet(0, 1, 0, 0));
						rotation = XMQuaternionInverse(XMQuaternionRotationMatrix(view));
					}
					bool a = blockHit.actor->isRigidDynamic();

					XMMATRIX view_normal = XMMatrixLookAtRH(physics_manager.PxVec3ToXMVECTOR(blockHit.position - blockHit.normal), physics_manager.PxVec3ToXMVECTOR(blockHit.position), XMVectorSet(0, 1, 0, 0));
					XMVECTOR normal_rotation = XMQuaternionInverse(XMQuaternionRotationMatrix(view_normal));
					XMVECTOR finalQuat = XMQuaternionSlerp(rotation, normal_rotation, 0.35f);

					new_e_needle2->create(
						blockHit.actor->isRigidDynamic() ? physics_manager.PxVec3ToXMVECTOR(offset_2) : physics_manager.PxVec3ToXMVECTOR(blockHit.position)
						, XMQuaternionMultiply(finalQuat, XMQuaternionInverse(physics_manager.PxQuatToXMVECTOR(blockHit.actor->getGlobalPose().q)))
						, rigidbody_e->get<TCompRigidBody>()
						);


					strings.push_back(CHandle(new_e_r));
					firstActor = nullptr;
					firstNeedle = CHandle();
					entitycount++;
					
				}
				// Same actor, action cancelled
				else {
					firstActor = nullptr;
					firstPosition = PxVec3(0, 0, 0);
					entity_manager.remove(firstNeedle);
					firstNeedle = CHandle();
				}
			}
	}


	if (io.becomesPressed(CIOStatus::EXTRA)) {
		// Get the camera position
		CEntity* e = CEntityManager::get().getByName("PlayerCamera");
		TCompTransform* t = e->get<TCompTransform>();

		// Raycast detecting the collider the mouse is pointing at
		PxRaycastBuffer hit;
		physics_manager.raycast(t->position, t->getFront(), 1000, hit);

		static int entitycount = 1;
		static PxRigidActor* firstActor = nullptr;
		static PxVec3 firstPosition = PxVec3(0, 0, 0);
		if (hit.hasBlock) {
			PxRaycastHit blockHit = hit.block;

			CEntity* new_e = entity_manager.createEmptyEntity();

			TCompName* new_e_name = CHandle::create<TCompName>();
			strcpy(new_e_name->name, ("TestCube" + std::to_string(entitycount)).c_str());
			new_e->add(new_e_name);

			TCompTransform* new_e_t = CHandle::create<TCompTransform>();
			new_e_t->position = physics_manager.PxVec3ToXMVECTOR(blockHit.position + blockHit.normal * 1);
			new_e->add(new_e_t);

			TCompMesh* new_e_m = CHandle::create<TCompMesh>();
			new_e_m->mesh = mesh_manager.getByName("primitive_box");
			strcpy(new_e_m->path, "primitive_box");
			new_e->add(new_e_m);

			TCompCollider* new_e_c = CHandle::create<TCompCollider>();
			new_e_c->setShape(0.5f, 0.5f, 0.5f, 0.5f, 0.2f, 0.6f);
			new_e->add(new_e_c);

			TCompRigidBody* new_e_r = CHandle::create<TCompRigidBody>();
			new_e->add(new_e_r);
			new_e_r->create(10, false, true);


			entitycount++;
		}
	}
	

	// Update ---------------------
	ctes_global.get()->world_time += elapsed;
	
	// Ñapa para luz ambiental
	for (int i = 0; i < entity_manager.getEntities().size(); ++i) {
		CEntity* e_ambLight = entity_manager.getEntities()[i];
		TCompAmbientLight* ambLight = e_ambLight->get<TCompAmbientLight>();
		if (ambLight && ambLight->active) {
			ctes_global.get()->AmbientLight = ambLight->color;
		}
	}

	// Ñapa para luces direccionales
	// Recorrer las luces y añadirlas al array
	ctes_global.get()->LightCount = 0;
	for (int i = 0; i < entity_manager.getEntities().size(); ++i) {
		CEntity* e_dirLD = entity_manager.getEntities()[i];
		TCompDirectionalLight* dirLD = e_dirLD->get<TCompDirectionalLight>();
		TCompTransform* transLD = e_dirLD->get<TCompTransform>();
		if (dirLD && transLD && dirLD->active) {
			ctes_global.get()->LightDirections[ctes_global.get()->LightCount] = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), transLD->rotation);
			// La intensidad se pasa en el alfa del color
			ctes_global.get()->LightColors[ctes_global.get()->LightCount] = XMVectorSetW(dirLD->color, dirLD->intensity * 0.1f);
			ctes_global.get()->LightCount++;
		}
	}

	// Ñapa para luces puntuales
	// Recorrer las luces y añadirlas al array
	ctes_global.get()->OmniLightCount = 0;
	for (int i = 0; i < entity_manager.getEntities().size(); ++i) {
		CEntity* e_pointL = entity_manager.getEntities()[i];
		TCompPointLight* pointL = e_pointL->get<TCompPointLight>();
		TCompTransform* trans_pointL = e_pointL->get<TCompTransform>();
		if (pointL && trans_pointL && pointL->active) {
			ctes_global.get()->OmniLightColors[ctes_global.get()->OmniLightCount] = XMVectorSetW(pointL->color, pointL->intensity * 0.1f);;
			ctes_global.get()->OmniLightPositions[ctes_global.get()->OmniLightCount] = trans_pointL->position;
			ctes_global.get()->OmniLightRadius[ctes_global.get()->OmniLightCount] = XMVectorSet(pointL->radius, 0, 0, 0);
			ctes_global.get()->OmniLightCount++;
		}
	}
	float f = ctes_global.get()->world_time;
	dbg("time: %f\n", f);

	ctes_global.uploadToGPU();
	ctes_global.activateInVS(2);
	ctes_global.activateInPS(2);

	CEntity* cam = entity_manager.getByName("PlayerCamera");
	TCompTransform* cam_t = cam->get<TCompTransform>();
	activateCamera(cam_t->position, 1);

	getObjManager<TCompPlayerController>()->update(elapsed); // Update player transform
	getObjManager<TCompPlayerPivotController>()->update(elapsed);
	getObjManager<TCompCameraPivotController>()->update(elapsed);
	getObjManager<TCompThirdPersonCameraController>()->update(elapsed); // Then update camera transform, wich is relative to the player
	getObjManager<TCompCamera>()->update(elapsed);  // Then, update camera view and projection matrix
	getObjManager<TCompAABB>()->update(elapsed); // Update objects AABBs
	getObjManager<TCompAiFsmBasic>()->update(elapsed);
	getObjManager<TCompUnityCharacterController>()->update(elapsed);

	//PRUEBA TRIGGER
	getObjManager<TCompTrigger>()->update(elapsed);
	getObjManager<TCompDistanceText>()->update(elapsed);
	getObjManager<TCompBasicPlayerController>()->update(elapsed);

#ifdef _DEBUG
	entity_inspector.update();
	entity_lister.update();
	entity_actioner.update();
#endif
}

// Physics update
void CApp::fixedUpdate(float elapsed) {
	physics_manager.gScene->simulate(elapsed);
	physics_manager.gScene->fetchResults(true);

	getObjManager<TCompPlayerController>()->fixedUpdate(elapsed); // Update kinematic player
	getObjManager<TCompEnemyController>()->fixedUpdate(elapsed);
	getObjManager<TCompRope>()->fixedUpdate(elapsed);
	getObjManager<TCompNeedle>()->fixedUpdate(elapsed);
	getObjManager<TCompUnityCharacterController>()->fixedUpdate(elapsed);
	getObjManager<TCompBasicPlayerController>()->fixedUpdate(elapsed);
	getObjManager<TCompRigidBody>()->fixedUpdate(elapsed); // Update rigidBodies of the scene
}

void CApp::render() {

	// Render ---------------------
	float ClearColor[4] = { 0.1f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
	::render.ctx->ClearRenderTargetView(::render.render_target_view, ClearColor);
	::render.ctx->ClearDepthStencilView(::render.depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);

	activateWorldMatrix(0);
	activateTint(0);

	activateTextureSamplers();

	render_techniques_manager.getByName("basic")->activate();
	activateWorldMatrix(0);

	render_manager.renderAll(((TCompCamera*)activeCamera)->view_projection);
	renderDebugEntities();
	renderEntities();

#ifdef _DEBUG
	TwDraw();
#endif

	/*std::string life_text = "Life: " + std::to_string((int)((TCompLife*)((CEntity*)entity_manager.getByName("Player"))->get<TCompLife>())->life);
	font.print(15, 15, life_text.c_str());

	std::string strings_text = "Ropes: " + std::to_string(numStrings()) + "/4";
	font.print(15, 35, strings_text.c_str());*/

	::render.swap_chain->Present(0, 0);

}

void CApp::renderEntities() {

	debugTech.activate();
	const CTexture *t = texture_manager.getByName("wood_d");
	t->activate(0);	

	//ctes_global.activateInVS(2);

	activateWorldMatrix(0);
	activateTint(0);

	// Render entities
	for (int i = 0; i < entity_manager.getEntities().size(); ++i)
	{
		TCompTransform* t = ((CEntity*)entity_manager.getEntities()[i])->get<TCompTransform>();
		TCompMesh* mesh = ((CEntity*)entity_manager.getEntities()[i])->get<TCompMesh>();

		TCompDistanceJoint* djoint = ((CEntity*)entity_manager.getEntities()[i])->get<TCompDistanceJoint>();
		TCompRope* c_rope = ((CEntity*)entity_manager.getEntities()[i])->get<TCompRope>();

		// Draw the joints
		if (c_rope) {
			PxRigidActor* a1 = nullptr;
			PxRigidActor* a2 = nullptr;

			djoint->joint->getActors(a1, a2);
			if (a1 && a2) {

				XMVECTOR offset_pos1 = physics_manager.PxVec3ToXMVECTOR(djoint->joint->getLocalPose(PxJointActorIndex::eACTOR0).p);
				XMVECTOR offset_pos2 = physics_manager.PxVec3ToXMVECTOR(djoint->joint->getLocalPose(PxJointActorIndex::eACTOR1).p);

				XMVECTOR pos1 = physics_manager.PxVec3ToXMVECTOR(a1->getGlobalPose().p);
				XMVECTOR pos2 = physics_manager.PxVec3ToXMVECTOR(a2->getGlobalPose().p);

				XMVECTOR rot1 = physics_manager.PxQuatToXMVECTOR(a1->getGlobalPose().q);
				XMVECTOR rot2 = physics_manager.PxQuatToXMVECTOR(a2->getGlobalPose().q);

				XMVECTOR offset_rotado_1 = XMVector3Rotate(offset_pos1, rot1);
				XMVECTOR offset_rotado_2 = XMVector3Rotate(offset_pos2, rot2);

				//   RECREATE ROPE   
				// Obtener el punto en coordenadas de mundo = Offset * rotación + posición
				XMVECTOR initialPos = pos1 + offset_rotado_1;
				XMVECTOR finalPos = pos2 + offset_rotado_2;

				float dist = djoint->joint->getDistance();
				float maxDist = pow(djoint->joint->getMaxDistance(), 2);

				float tension = 1 - (min(dist, maxDist) / (maxDist * 1.2f));

				rope.destroy();
				createFullString(rope, initialPos, finalPos, tension, c_rope->width);

				float color_tension = min(dist / maxDist * 0.25f, 1);
				setTint(XMVectorSet(color_tension * 3, (1 - color_tension) * 3, 0, 1));
				setWorldMatrix(XMMatrixIdentity());
				rope.activateAndRender();

				setWorldMatrix(XMMatrixAffineTransformation(XMVectorSet(0.1f, 0.1f, 0.1f, 0.1f), XMVectorZero(), rot1, initialPos));
				wiredCube.activateAndRender();

				setWorldMatrix(XMMatrixAffineTransformation(XMVectorSet(0.1f, 0.1f, 0.1f, 0.1f), XMVectorZero(), rot2, finalPos));
				wiredCube.activateAndRender();
			}
		}

		// If the component has no transform it can't be rendered
		if (!t)
			continue;

		if (mesh)
			setTint(mesh->color);

		setWorldMatrix(t->getWorld());

		if (mesh && mesh->active)
			mesh->mesh->activateAndRender();

		// Draw texts
		TCompDistanceText* c_text = ((CEntity*)entity_manager.getEntities()[i])->get<TCompDistanceText>();
		if (c_text && t) {
			float old_size = font.size;
			font.size = c_text->size;
			unsigned int old_col = font.color;
			font.color = c_text->color;
			font.print3D(t->position, c_text->text);
			font.size = old_size;
			font.color = old_col;
		}
	}
}

void CApp::renderDebugEntities() {

	debugTech.activate();
	setWorldMatrix(XMMatrixIdentity());
	if (renderGrid)
		grid.activateAndRender();
	if (renderAxis)
		axis.activateAndRender();

	// Render entities
	for (int i = 0; i < entity_manager.getEntities().size(); ++i)
	{
		CEntity* e = (CEntity*)entity_manager.getEntities()[i];
		TCompTransform* t = e->get<TCompTransform>();
		TCompName* name = e->get<TCompName>();
		TCompAABB* aabb = e->get<TCompAABB>();

		// If the component has no transform it can't be rendered
		if (!t)
			continue;

		setWorldMatrix(t->getWorld());
		if (renderAxis)
			axis.activateAndRender();

		// If the entity has name, print it
		if (name && renderNames)
			font.print3D(t->position, name->name);

		// If the entity has an AABB, draw it
		if (aabb && renderAABB) {
			bool intersects = false;
			for (int j = 0; j < entity_manager.getEntities().size(); ++j) {
				CEntity* e2 = (CEntity*)entity_manager.getEntities()[j];
				TCompAABB* aabb2 = e2->get<TCompAABB>();
				if (aabb2 && i != j && aabb->intersects(aabb2)) {
					intersects = true;
					break;
				}
			}

			// Draw AABB
			XMVECTOR zero = XMVectorSet(0.f, 0.f, 0.f, 1.f);
			setWorldMatrix(XMMatrixAffineTransformation(aabb->getSize(), zero, zero, aabb->getCenter()));
			if (intersects)
				intersectsWiredCube.activateAndRender();
			else
				wiredCube.activateAndRender();

			// Draw max and min
			setWorldMatrix(XMMatrixAffineTransformation(XMVectorSet(0.05, 0.05, 0.05, 0), zero, zero, aabb->min));
			wiredCube.activateAndRender();
			setWorldMatrix(XMMatrixAffineTransformation(XMVectorSet(0.05, 0.05, 0.05, 0), zero, zero, aabb->max));
			wiredCube.activateAndRender();
		}
	}
}

void CApp::activateInspectorMode(bool active) {
	CIOStatus& io = CIOStatus::get();
	// Update input
	io.setMousePointer(!active);

	// Activa el modo debug
	renderAxis = active;
	renderAABB = active;
	renderGrid = active;
	renderNames = active;

	// Desactivar los componentes
	getObjManager<TCompPlayerController>()->setActiveComponents(!active);
	getObjManager<TCompPlayerPivotController>()->setActiveComponents(!active);
	getObjManager<TCompCameraPivotController>()->setActiveComponents(!active);
	getObjManager<TCompThirdPersonCameraController>()->setActiveComponents(!active);
}

void CApp::activateDebugMode(bool active) {
	CIOStatus& io = CIOStatus::get();

	// Activa el modo debug
	renderAxis = active;
	renderAABB = active;
	renderGrid = active;
	renderNames = active;

	debug_mode = active;
}

void CApp::destroy() {
	TwTerminate();
	mesh_manager.destroyAll();
	texture_manager.destroyAll();
	axis.destroy();
	grid.destroy();
	intersectsWiredCube.destroy();
	wiredCube.destroy();
	renderUtilsDestroy();
	debugTech.destroy();
	font.destroy();
	::render.destroyDevice();
}

unsigned int CApp::numStrings(){
	int num_strings = 0;
	for (int i = 0; i < entity_manager.getEntities().size(); ++i){
		TCompRope* c_rope = ((CEntity*)entity_manager.getEntities()[i])->get<TCompRope>();
		if (c_rope){			
			num_strings++;
		}
	}
	return num_strings;
}

void CApp::activateVictory(){
	getObjManager<TCompThirdPersonCameraController>()->setActiveComponents(false);
}
