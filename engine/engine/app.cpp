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

#include "item_manager.h"
#include "navmesh\navmesh.h"
#include "nav_mesh_manager.h"

#include "components\all_components.h"
#include "components/comp_skeleton.h"
#include "components/comp_skeleton_lookat.h"
#include "components/comp_skeleton_ik.h"
#include "skeletons/ik_handler.h"
#include "render/render_to_texture.h"
#include "render/deferred_render.h"
#include "audio\sound_manager.h"

#include <PxPhysicsAPI.h>
#include <foundation\PxFoundation.h>

using namespace physx;

#include <AntTweakBar.h>
#include "entity_inspector.h"
#include "render\all_post_process_effects.h"

static CApp the_app;

CLogicManager			 &logic_manager = CLogicManager::get();
CEntityInspector		 &entity_inspector = CEntityInspector::get();
CEntityLister			 &entity_lister = CEntityLister::get();
CEntityActioner			 &entity_actioner = CEntityActioner::get();
CDebugOptioner			 &debug_optioner = CDebugOptioner::get();
CConsole				 &console = CConsole::get();
CPostProcessOptioner	 &post_process_optioner = CPostProcessOptioner::get();

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
CRenderTechnique ropeTech;
CMesh		 wiredCube;
CMesh		 intersectsWiredCube;
CMesh		 rope;

CHandle		 life;

CHandle		  activeCamera;
CFont         font;
CDeferredRender deferred;
CShaderCte<TCtesGlobal> ctes_global;
CRenderToTexture* rt_base;
CSoundManager sm;

const CTexture* cubemap;

float fixedUpdateCounter;
float fps;
bool debug_mode;

TSharpenStep sharpen;
TSSAOStep ssao;
TChromaticAberrationStep chromatic_aberration;
TBlurStep blur;
TGlowStep glow;

//---------------------------------------------------
//CNavmesh nav_prueba;
//---------------------------------------------------

void registerAllComponentMsgs() {
	//SUBSCRIBE(TCompLife, TMsgExplosion, onExplosion);
	SUBSCRIBE(TCompAiFsmBasic, TGroundHit, groundHit);
	SUBSCRIBE(TCompBasicPlayerController, TActorHit, actorHit);

	//IA events
	SUBSCRIBE(TCompAiBT, TActorHit, actorHit);
	//SUBSCRIBE(TCompBtGrandma, TMsgEnemyTied, onEnemyTied);

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
	getObjManager<TCompJointHinge>()->init(32);
	getObjManager<TCompJointD6>()->init(512);
	getObjManager<TCompRope>()->init(32);
	getObjManager<TCompNeedle>()->init(1024);
	getObjManager<TCompPlayerPosSensor>()->init(64);
	getObjManager<TCompSensorNeedles>()->init(64);
	getObjManager<TCompSensorTied>()->init(64);
	getObjManager<TCompSensorDistPlayer>()->init(64);
	//PRUEBA TRIGGER
	getObjManager<TCompTrigger>()->init(1024);
	getObjManager<TCompDistanceText>()->init(32);
	getObjManager<TCompVictoryCond>()->init(1);
	//

	// Interruptores
	getObjManager<TCompSwitchController>()->init(32);
	getObjManager<TCompSwitchPullController>()->init(32);
	getObjManager<TCompSwitchPushController>()->init(32);

	// Plataformas
	getObjManager<TCompPlatformPath>()->init(32);	

	// Lights (temporary)
	getObjManager<TCompDirectionalLight>()->init(16);
	getObjManager<TCompAmbientLight>()->init(1);
	getObjManager<TCompPointLight>()->init(64);

	getObjManager<TCompAiFsmBasic>()->init(64);
	getObjManager<TCompEnemyController>()->init(64);
	getObjManager<TCompBtGrandma>()->init(64);

	getObjManager<TCompCharacterController>()->init(64);
	getObjManager<TCompUnityCharacterController>()->init(64);
	getObjManager<TCompBasicPlayerController>()->init(1);

	getObjManager<TCompSkeleton>()->init(1024);
	getObjManager<TCompSkeletonLookAt>()->init(1024);
	getObjManager<TCompSkeletonIK>()->init(1024);

	getObjManager<TCompRagdoll>()->init(64);
	getObjManager<TCompShadows>()->init(8);


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
	//getObjManager<TCompUnityCharacterController>()->initHandlers();
	getObjManager<TCompPlayerController>()->initHandlers();
	getObjManager<TCompPlayerPivotController>()->initHandlers();
	getObjManager<TCompCameraPivotController>()->initHandlers();
	getObjManager<TCompThirdPersonCameraController>()->initHandlers();
	getObjManager<TCompDistanceJoint>()->initHandlers();
	getObjManager<TCompJointPrismatic>()->initHandlers();
	getObjManager<TCompJointD6>()->initHandlers();
	getObjManager<TCompEnemyController>()->initHandlers();

	getObjManager<TCompPlayerPosSensor>()->initHandlers();
	getObjManager<TCompSensorNeedles>()->initHandlers();
	getObjManager<TCompSensorTied>()->initHandlers();

	// PLATFORMS
	getObjManager<TCompPlatformPath>()->initHandlers();

	// SWITCHS
	getObjManager<TCompSwitchPullController>()->initHandlers();
	getObjManager<TCompSwitchPushController>()->initHandlers();

	//PRUEBA TRIGGER

	getObjManager<TCompTrigger>()->initHandlers();
	getObjManager<TCompDistanceText>()->initHandlers();
	getObjManager<TCompVictoryCond>()->initHandlers();

	getObjManager<TCompBasicPlayerController>()->initHandlers();
	getObjManager<TCompAiFsmBasic>()->initHandlers();
	getObjManager<TCompBtGrandma>()->initHandlers();

	getObjManager<TCompSkeleton>()->initHandlers();
	getObjManager<TCompShadows>()->initHandlers();

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
	bool is_ok = true;
	// Boot LUA
	logic_manager.bootLUA();

	XASSERT(font.create(), "Error creating the font");


   //loadScene("data/scenes/my_file-backup.xml");
	loadScene("data/scenes/my_file.xml");
	

	sm.addMusicTrack(0, "plug in baby.mp3");
	sm.addMusicTrack(1, "More than a feeling - Boston.mp3");

	sm.playTrack(0);

	// Create debug meshes	
	is_ok = createUnitWiredCube(wiredCube, XMFLOAT4(1.f, 1.f, 1.f, 1.f));
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
	console.init();
	post_process_optioner.sharpen = &sharpen;
	post_process_optioner.chromatic_aberration = &chromatic_aberration;
	post_process_optioner.blur = &blur;

	post_process_optioner.init();
	

	entity_lister.update();	
#endif
	
	fps = 0;

	// Timer test
	logic_manager.setTimer("TestTimer", 10);

	cubemap = texture_manager.getByName("sunsetcube1024");
	
	cubemap->activate(3);

	texture_manager.getByName("storm")->activate(4);

	is_ok &= sharpen.create("sharpen", xres, yres, 1);	
	is_ok &= ssao.create("ssao", xres, yres, 1);
	is_ok &= chromatic_aberration.create("chromatic_aberration", xres, yres, 1);
	is_ok &= blur.create("blur", xres, yres, 1);
	is_ok &= glow.create("glow", xres, yres, 1);

	assert(is_ok);

	//PRUEBAS NAV MESHES -----------------
	bool valid = CNav_mesh_manager::get().build_nav_mesh();
	//------------------------------------
	/*CEntity* r = entity_manager.getByName("dvn_arqui_suelo_esqui2_in_01_10.0");
	CHandle t = r->get<TCompTransform>();
	TCompTransform* tt = t;

	CRigidAnimation anim(t);

	anim.addRelativeKeyframe(XMVectorSet(0, -15, 0, 0), XMQuaternionIdentity(), 3);
	anim.addRelativeKeyframe(XMVectorSet(15, 0, 0, 0), XMQuaternionIdentity(), 5);
	anim.addRelativeKeyframe(XMVectorSet(0, 0, 0, 0), XMQuaternionIdentity(), 5);
	anim.addRelativeKeyframe(XMVectorSet(0, 4, 0, 0), XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), deg2rad(270)), 10);
	
	logic_manager.addRigidAnimation(anim);	*/

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

	fps = 1.0f / delta_secs;

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

	if (io.becomesReleased(CIOStatus::F8_KEY)) {
		render_techniques_manager.reload("deferred_gbuffer");
		//render_techniques_manager.reload("deferred_point_lights");
		//render_techniques_manager.reload("deferred_dir_lights");
		//render_techniques_manager.reload("deferred_resolve");
		//render_techniques_manager.reload("chromatic_aberration");
		render_techniques_manager.reload("deferred_dir_lights");
		render_techniques_manager.reload("skin_basic");
		texture_manager.reload("Foco_albedo");
		texture_manager.reload("Foco_normal");
		
	}

	//-----------------------------------------------------------------------------------------
	CNav_mesh_manager::get().checkUpdates();
	//-----------------------------------------------------------------------------------------

	//----------------------- PRUEBAS NAVMESH/DETOUR ------------------------------------------
	/*XMVECTOR ini = XMVectorSet(0, 0, 0, 0);
	XMVECTOR fin = XMVectorSet(-8.05f, 0.10f, -27.60f, 0.f);
	CEntity* player = entity_manager.getByName("Player");
	TCompTransform* player_t = player->get<TCompTransform>();
	fin = player_t->position;
	std::vector<XMVECTOR> path;
	CNav_mesh_manager::get().findPath(ini, fin, path);*/
	//-----------------------------------------------------------------------------------------

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
	//Insertamos aguja en vector agujas del item manager
	/*Citem_manager::get().addNeedle(new_e_needle);

	//Insertamos aguja en vector agujas del item manager					
	Citem_manager::get().addNeedle(new_e_needle2);

	//borrado de la aguja también del item manager
	CEntity* e = (CEntity*)firstNeedle;
	TCompNeedle* needle = e->get<TCompNeedle>();
	Citem_manager::get().removeNeedle(needle);*/


	// Update ---------------------
	ctes_global.get()->world_time += elapsed;
	
	getObjManager<TCompTransform>()->update(elapsed);
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
	getObjManager<TCompBtGrandma>()->update(elapsed);
	getObjManager<TCompUnityCharacterController>()->update(elapsed);
	getObjManager<TCompCharacterController>()->update(elapsed);

	// SWITCH
	getObjManager<TCompSwitchController>()->update(elapsed);
	getObjManager<TCompSwitchPullController>()->update(elapsed);
	getObjManager<TCompSwitchPushController>()->update(elapsed);

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
	getObjManager<TCompPlatformPath>()->fixedUpdate(elapsed);
	getObjManager<TCompCharacterController>()->fixedUpdate(elapsed);	
	getObjManager<TCompRigidBody>()->fixedUpdate(elapsed); // Update rigidBodies of the scene
	getObjManager<TCompStaticBody>()->fixedUpdate(elapsed);
	getObjManager<TCompRagdoll>()->fixedUpdate(elapsed);	
}

void CApp::render() {

	// Render ---------------------
	float ClearColor[4] = { 0.1f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
	::render.ctx->ClearRenderTargetView(::render.render_target_view, ClearColor);
	::render.ctx->ClearDepthStencilView(::render.depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);

	ctes_global.uploadToGPU();
	ctes_global.activateInPS(2);
	activateTextureSamplers();
	CCamera camera = *(TCompCamera*)activeCamera;

	/*CHandle h_light = entity_manager.getByName("the_light");
	CEntity* e_light = h_light;
	if (e_light) {
		TCompCamera* cam_light = e_light->get<TCompCamera>();
		activateLight(*cam_light, 4);
	}*/

	activateZConfig(ZConfig::ZCFG_DEFAULT);

	// Generate all shadows maps
	CTraceScoped scope("gen_shadows");
	getObjManager<TCompShadows>()->onAll(&TCompShadows::generate);


	deferred.render(&camera, *rt_base);

	sharpen.apply(rt_base);
	chromatic_aberration.apply(sharpen.getOutput());
	blur.apply(chromatic_aberration.getOutput());
	//glow.apply(blur.getOutput());

	::render.activateBackbuffer();
	static int sz = 150;
	
	//drawTexture2D(0, 0, xres, yres, texture_manager.getByName("rt_normals"));
	//drawTexture2D(0, 0, sz, sz, texture_manager.getByName("rt_albedo"));
	
	activateZConfig(ZConfig::ZCFG_DISABLE_ALL);
	//ssao.getOutput()->activate(1);
	


	//texture_manager.getByName("rt_depth")->activate(2);

	//drawTexture2D(0, 0, xres, yres, rt_base, "sharpen");
	drawTexture2D(0, 0, xres, yres, texture_manager.getByName("rt_lights"));
	//drawTexture2D(0, 0, xres, yres, texture_manager.getByName("rt_depth")); 

	//drawTexture2D(0, 0, sz * camera.getAspectRatio(), sz, bs2.getOutput());

	/*CHandle h_light = entity_manager.getByName("the_light");
	CEntity* e_light = h_light;
	TCompShadows* shadow = e_light->get<TCompShadows>();
	
	
	drawTexture2D(0, 0, sz * camera.getAspectRatio(), sz, texture_manager.getByName("rt_depth"));
	drawTexture2D(0, sz, sz * camera.getAspectRatio(), sz, texture_manager.getByName("rt_lights"));
	drawTexture2D(0, 2*sz, sz * camera.getAspectRatio(), sz, shadow->rt.getZTexture());	
	drawTexture2D(0, 3 * sz, sz * camera.getAspectRatio(), sz, texture_manager.getByName("rt_normals"));
	drawTexture2D(0, 4 * sz, sz * camera.getAspectRatio(), sz, texture_manager.getByName("rt_albedo"));*/
	render_techniques_manager.getByName("basic")->activate();
	activateWorldMatrix(0);
	activateCamera(camera, 1);

	setWorldMatrix(XMMatrixIdentity());
	render_techniques_manager.getByName("basic")->activate();
	activateWorldMatrix(0);

	activateZConfig(ZConfig::ZCFG_DEFAULT);

	//render_manager.renderAll((TCompCamera*)activeCamera, ((TCompTransform*)((CEntity*)activeCamera.getOwner())->get<TCompTransform>()));
	renderEntities();
	render_manager.renderAll(&camera, false);
	renderDebugEntities();

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
	const CTexture *t = texture_manager.getByName("grass");
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

			ropeTech.activate();
			float color_tension = min(dist / maxDist * 0.25f, 1);
			setTint(XMVectorSet(color_tension * 3, (1 - color_tension) * 3, 0, 1));
			setWorldMatrix(XMMatrixIdentity());
			rope.activateAndRender();

			setWorldMatrix(XMMatrixAffineTransformation(XMVectorSet(0.1f, 0.1f, 0.1f, 0.1f), XMVectorZero(), rot1, initialPos));
			wiredCube.activateAndRender();

			setWorldMatrix(XMMatrixAffineTransformation(XMVectorSet(0.1f, 0.1f, 0.1f, 0.1f), XMVectorZero(), rot2, finalPos));
			wiredCube.activateAndRender();
			debugTech.activate();
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

	std::string s_fps = "FPS: " + std::to_string(fps);
	font.print(300, 30, s_fps.c_str());

	getObjManager<TCompSkeleton>()->renderDebug3D();
	getObjManager<TCompTrigger>()->renderDebug3D();

	//--------- NavMesh render Prueba --------------
	if (CIOStatus::get().isPressed(CIOStatus::EXIT)){
		CNav_mesh_manager::get().keep_updating_navmesh = false;
		exit(-1);
	}
	//if (renderNavMesh)
	CNav_mesh_manager::get().render_nav_mesh();
	//----------------------------------------------

	CNav_mesh_manager::get().pathRender();

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
	renderNavMesh = active;
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
	CNav_mesh_manager::get().keep_updating_navmesh = false;
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
	CNav_mesh_manager::get().clearNavMesh();
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
	//logic_manager.clearKeyframes();
	logic_manager.clearAnimations();

	rt_base = new CRenderToTexture;
	rt_base->create("deferred_output", xres, yres, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::USE_BACK_ZBUFFER);

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
	XASSERT(ropeTech.load("textured"), "Error loading basic technique");

	CEntity* e = entity_manager.getByName("PlayerCamera");
	XASSERT(CHandle(e).isValid(), "Camera not valid");

	activeCamera = e->get<TCompCamera>();

	font.camera = activeCamera;

	// Ctes ---------------------------
	bool is_ok = renderUtilsCreate();
	is_ok &= deferred.create(xres, yres);

	//ctes_global.world_time = XMVectorSet(0, 0, 0, 0);
	ctes_global.get()->world_time = 0.f; // XMVectorSet(0, 0, 0, 0);
	is_ok &= ctes_global.create();
	XASSERT(is_ok, "Error creating global constants");

	XASSERT(is_ok, "Error creating debug meshes");

#ifdef _DEBUG	
	entity_inspector.inspectEntity(CHandle());
#endif

	activateInspectorMode(false);
	std::string name = split_string(split_string(scene_name, "/").back(), ".").front();
	logic_manager.onSceneLoad(name);

	//Borrado de mapa de colisiones una vez cargado en sus respectivos colliders
	CPhysicsManager::get().m_collision->clear();
}

void CApp::loadPrefab(std::string prefab_name) {
	CEntity* e = prefabs_manager.getInstanceByName(prefab_name.c_str());
}
