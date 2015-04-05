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
#include "ai\logic_manager.h"
#include "error\log.h"
#include <time.h>

using namespace DirectX;
#include "render/ctes/shader_ctes.h"
#include "render/render_manager.h"
#include "handle\prefabs_manager.h"

#include "components\all_components.h"
#include "components/comp_skeleton.h"
#include "components/comp_skeleton_lookat.h"
#include "components/comp_skeleton_ik.h"
#include "skeletons/ik_handler.h"

#include <PxPhysicsAPI.h>
#include <foundation\PxFoundation.h>

using namespace physx;

#include <AntTweakBar.h>
#include "entity_inspector.h"


static CApp the_app;

CLogicManager	 &logic_manager = CLogicManager::get();
CEntityInspector &entity_inspector = CEntityInspector::get();
CEntityLister	 &entity_lister = CEntityLister::get();
CEntityActioner	 &entity_actioner = CEntityActioner::get();
CDebugOptioner	 &debug_optioner = CDebugOptioner::get();

CEntityManager &entity_manager = CEntityManager::get();
CPhysicsManager &physics_manager = CPhysicsManager::get();

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

const CTexture* cubemap;

float fixedUpdateCounter;

bool debug_mode;

void registerAllComponentMsgs() {
	//SUBSCRIBE(TCompLife, TMsgExplosion, onExplosion);
	SUBSCRIBE(TCompAiFsmBasic, TGroundHit, groundHit);
	SUBSCRIBE(TCompBasicPlayerController, TActorHit, actorHit);
	SUBSCRIBE(TCompBasicPlayerController, TMsgAttackDamage, onAttackDamage);
	SUBSCRIBE(TCompPlayerController, TActorHit, actorHit);
	SUBSCRIBE(TCompPlayerController, TMsgAttackDamage, onAttackDamage);
	SUBSCRIBE(TCompAiFsmBasic, TActorHit, actorHit);
	SUBSCRIBE(TCompAiFsmBasic, TMsgRopeTensed, onRopeTensed);
	SUBSCRIBE(TCompVictoryCond, TVictoryCondition, victory);
}

void createManagers() {
	SET_ERROR_CONTEXT("Creating", "managers");

	getObjManager<CEntity>()->init(1024);
	getObjManager<TCompTransform>()->init(1024);
	getObjManager<TCompLife>()->init(32);
	getObjManager<TCompName>()->init(1024);
	getObjManager<TCompMesh>()->init(1024);
	getObjManager<TCompRender>()->init(1024);
	getObjManager<TCompColliderMesh>()->init(512);
	getObjManager<TCompCamera>()->init(4);
	getObjManager<TCompColliderBox>()->init(512);
	getObjManager<TCompColliderSphere>()->init(512);
	getObjManager<TCompColliderCapsule>()->init(512);
	getObjManager<TCompRigidBody>()->init(512);
	getObjManager<TCompStaticBody>()->init(512);
	getObjManager<TCompAABB>()->init(1024);
	getObjManager<TCompPlayerController>()->init(1);
	getObjManager<TCompPlayerPivotController>()->init(1);
	getObjManager<TCompCameraPivotController>()->init(1);
	getObjManager<TCompThirdPersonCameraController>()->init(1);
	getObjManager<TCompViewerCameraController>()->init(1);
	getObjManager<TCompDistanceJoint>()->init(32);
	getObjManager<TCompJointPrismatic>()->init(32);
	getObjManager<TCompRope>()->init(32);
	getObjManager<TCompNeedle>()->init(1024);
	getObjManager<TCompPlayerPosSensor>()->init(64);
	getObjManager<TCompSensorNeedles>()->init(64);
	//PRUEBA TRIGGER
	getObjManager<TCompTrigger>()->init(1024);
	getObjManager<TCompDistanceText>()->init(32);
	getObjManager<TCompVictoryCond>()->init(1);
	//

	// Interruptores
	getObjManager<TCompSwitchController>()->init(32);

	// Lights (temporary)
	getObjManager<TCompDirectionalLight>()->init(16);
	getObjManager<TCompAmbientLight>()->init(1);
	getObjManager<TCompPointLight>()->init(64);

	getObjManager<TCompAiFsmBasic>()->init(64);
	getObjManager<TCompEnemyController>()->init(64);

	getObjManager<TCompCharacterController>()->init(64);
	getObjManager<TCompUnityCharacterController>()->init(64);
	getObjManager<TCompBasicPlayerController>()->init(1);

	getObjManager<TCompSkeleton>()->init(1024);
	getObjManager<TCompSkeletonLookAt>()->init(1024);
	getObjManager<TCompSkeletonIK>()->init(1024);

	registerAllComponentMsgs();
}

void initManagers() {
	CErrorContext ec("Initializing", "managers");

	getObjManager<TCompCamera>()->initHandlers();
	getObjManager<TCompColliderBox>()->initHandlers();
	getObjManager<TCompColliderSphere>()->initHandlers();
	getObjManager<TCompColliderCapsule>()->initHandlers();
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

	getObjManager<TCompPlayerPosSensor>()->initHandlers();
	getObjManager<TCompSensorNeedles>()->initHandlers();

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
	srand((unsigned int)time(NULL));
	
	createManagers();

	physics_manager.init();

	XASSERT(font.create(), "Error creating the font");

	loadScene("data/scenes/my_file.xml");

	// Create debug meshes
	bool is_ok = createGrid(grid, 10);
	is_ok &= createAxis(axis);
	is_ok &= createUnitWiredCube(wiredCube, XMFLOAT4(1.f, 1.f, 1.f, 1.f));
	is_ok &= createUnitWiredCube(intersectsWiredCube, XMFLOAT4(1.f, 0.f, 0.f, 1.f));

	XASSERT(is_ok, "Error creating debug meshes");

#ifdef _DEBUG
	// Init AntTweakBar
	TwInit(TW_DIRECT3D11, ::render.device);
	TwWindowSize(xres, yres);

	entity_inspector.init();
	entity_inspector.inspectEntity(CHandle());

	entity_lister.init();
	entity_actioner.init();
	debug_optioner.init();

	entity_lister.update();
#endif
	
	// Timer test
	logic_manager.setTimer("TestTimer", 10);

	cubemap = texture_manager.getByName("sunsetcube1024");

	cubemap->activate(3);

	CEntity* r = entity_manager.getByName("Hose001_2.0");
	CHandle t = r->get<TCompTransform>();
	TCompTransform* tt = t;


	logic_manager.addKeyFrame(t, tt->position + XMVectorSet(0, 6, 0, 0), tt->rotation, 10);
	logic_manager.addKeyFrame(t, tt->position + XMVectorSet(-20, 0, 0, 0), tt->rotation, 5);
	logic_manager.addKeyFrame(t, tt->position + XMVectorSet(-20, 0, 0, 0), tt->rotation, 5);
	logic_manager.addKeyFrame(t, tt->position + XMVectorSet(-20, 6, 0, 0), XMQuaternionMultiply(tt->rotation, XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), deg2rad(90))), 10);
	

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

	if (io.becomesReleased(CIOStatus::EXTRA)) {
		loadScene("data/scenes/milestone2.xml");
	}

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

	ctes_global.uploadToGPU();
	ctes_global.activateInVS(2);
	ctes_global.activateInPS(2);

	CEntity* cam = entity_manager.getByName("PlayerCamera");
	TCompTransform* cam_t = cam->get<TCompTransform>();
	activateCamera(cam_t->position, 1);

	getObjManager<TCompSkeleton>()->update(elapsed);
	getObjManager<TCompSkeletonLookAt>()->update(elapsed);
	getObjManager<TCompSkeletonIK>()->update(elapsed);
	getObjManager<TCompPlayerController>()->update(elapsed); // Update player transform
	getObjManager<TCompPlayerPivotController>()->update(elapsed);
	getObjManager<TCompCameraPivotController>()->update(elapsed);
	getObjManager<TCompThirdPersonCameraController>()->update(elapsed); // Then update camera transform, wich is relative to the player
	getObjManager<TCompViewerCameraController>()->update(elapsed);
	getObjManager<TCompCamera>()->update(elapsed);  // Then, update camera view and projection matrix
	getObjManager<TCompAABB>()->update(elapsed); // Update objects AABBs
	getObjManager<TCompAiFsmBasic>()->update(elapsed);
	getObjManager<TCompUnityCharacterController>()->update(elapsed);
	getObjManager<TCompCharacterController>()->update(elapsed);
	
	// Interruptor
	getObjManager<TCompSwitchController>()->update(elapsed);

	//PRUEBA TRIGGER
	getObjManager<TCompTrigger>()->update(elapsed);
	getObjManager<TCompDistanceText>()->update(elapsed);
	getObjManager<TCompBasicPlayerController>()->update(elapsed);

	logic_manager.update(elapsed);

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
	getObjManager<TCompCharacterController>()->fixedUpdate(elapsed);
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

	render_manager.renderAll((TCompCamera*)activeCamera, ((TCompTransform*)((CEntity*)activeCamera.getOwner())->get<TCompTransform>()));
	renderEntities();
	renderDebugEntities();

#ifdef _DEBUG
	TwDraw();
#endif

	std::string life_text = "Life: " + std::to_string((int)((TCompLife*)((CEntity*)entity_manager.getByName("Player"))->get<TCompLife>())->life);
	font.print(15, 15, life_text.c_str());

	std::string strings_text = "Ropes: " + std::to_string(numStrings()) + "/4";
	font.print(15, 35, strings_text.c_str());

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
		TCompName* name = ((CEntity*)entity_manager.getEntities()[i])->get<TCompName>();


		// Draw the joints
		if (c_rope) {
			/*PxRigidActor* a1 = nullptr;
			PxRigidActor* a2 = nullptr;

			djoint->joint->getActors(a1, a2);

			XMVECTOR initialPos = XMVectorZero();
			XMVECTOR finalPos = XMVectorZero();

			XMVECTOR rot1 = XMQuaternionIdentity();
			XMVECTOR rot2 = XMQuaternionIdentity();

			if (a1) {
				XMVECTOR offset_pos1 = physics_manager.PxVec3ToXMVECTOR(djoint->joint->getLocalPose(PxJointActorIndex::eACTOR0).p);

				XMVECTOR pos1 = physics_manager.PxVec3ToXMVECTOR(a1->getGlobalPose().p);
				rot1 = physics_manager.PxQuatToXMVECTOR(a1->getGlobalPose().q);

				XMVECTOR offset_rotado_1 = XMVector3Rotate(offset_pos1, rot1);

				//   RECREATE ROPE   
				// Obtener el punto en coordenadas de mundo = Offset * rotación + posición
				initialPos = pos1 + offset_rotado_1;
			}
			else {
				initialPos = physics_manager.PxVec3ToXMVECTOR(djoint->joint->getLocalPose(PxJointActorIndex::eACTOR0).p);
			}
			if (a2) {
				XMVECTOR offset_pos2 = physics_manager.PxVec3ToXMVECTOR(djoint->joint->getLocalPose(PxJointActorIndex::eACTOR1).p);

				XMVECTOR pos2 = physics_manager.PxVec3ToXMVECTOR(a2->getGlobalPose().p);
				rot2 = physics_manager.PxQuatToXMVECTOR(a2->getGlobalPose().q);

				XMVECTOR offset_rotado_2 = XMVector3Rotate(offset_pos2, rot2);

				//   RECREATE ROPE   
				// Obtener el punto en coordenadas de mundo = Offset * rotación + posición
				finalPos = pos2 + offset_rotado_2;
			}
			else {
				finalPos = physics_manager.PxVec3ToXMVECTOR(djoint->joint->getLocalPose(PxJointActorIndex::eACTOR1).p);
			}*/

			XMVECTOR initialPos = c_rope->pos_1;
			XMVECTOR finalPos = c_rope->pos_2;

			XMVECTOR rot1 = XMQuaternionIdentity();
			XMVECTOR rot2 = XMQuaternionIdentity();

			float dist = V3DISTANCE(initialPos, finalPos);
			float maxDist = dist;

			if (djoint) {
				maxDist = pow(djoint->joint->getMaxDistance(), 2);
			}

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

	getObjManager<TCompSkeleton>()->renderDebug3D();
	getObjManager<TCompTrigger>()->renderDebug3D();

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
			setWorldMatrix(XMMatrixAffineTransformation(XMVectorSet(0.05f, 0.05f, 0.05f, 0), zero, zero, aabb->min));
			wiredCube.activateAndRender();
			setWorldMatrix(XMMatrixAffineTransformation(XMVectorSet(0.05f, 0.05f, 0.05f, 0), zero, zero, aabb->max));
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
	//renderNames = active;

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
	//renderNames = active;

	debug_mode = active;
}

void CApp::destroy() {
	TwTerminate();
	mesh_manager.destroyAll();
	texture_manager.destroyAll();
	render_techniques_manager.destroyAll();
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

void CApp::loadScene(std::string scene_name) {
	CImporterParser p;
	entity_manager.clear();
	mesh_manager.destroyAll();
	texture_manager.destroyAll();
	render_techniques_manager.destroyAll();
	material_manager.destroyAll();
	render_manager.destroyAllKeys();
	ctes_global.destroy();
	renderUtilsDestroy();
	entity_lister.resetEventCount();

	XASSERT(p.xmlParseFile(scene_name), "Error loading the scene: %s", scene_name.c_str());

	// public delta time inicialization
	delta_time = 0.f;
	total_time = delta_time;
	fixedUpdateCounter = 0.0f;

	renderAABB = false;
	renderAxis = false;
	renderGrid = false;
	renderNames = false;
	debug_mode = false;

	//physics_manager.init();

	initManagers();

	// Create Debug Technique
	XASSERT(debugTech.load("basic"), "Error loading basic technique");

	CEntity* e = entity_manager.getByName("PlayerCamera");
	XASSERT(CHandle(e).isValid(), "Camera not valid");

	activeCamera = e->get<TCompCamera>();

	font.camera = activeCamera;

	// Ctes ---------------------------
	bool is_ok = renderUtilsCreate();

	//ctes_global.world_time = XMVectorSet(0, 0, 0, 0);
	ctes_global.get()->world_time = 0.f; // XMVectorSet(0, 0, 0, 0);
	is_ok &= ctes_global.create();
	XASSERT(is_ok, "Error creating global constants");

	XASSERT(is_ok, "Error creating debug meshes");

#ifdef _DEBUG	
	entity_inspector.inspectEntity(CHandle());
#endif

	activateInspectorMode(false);
}

void CApp::loadPrefab(std::string prefab_name) {
	CEntity* e = prefabs_manager.getInstanceByName(prefab_name.c_str());
}
