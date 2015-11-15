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
#include "skeletons\skeleton_manager.h"

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
#include "particles\importer_particle_groups.h"
#include "importer_resource_loader.h"

#include <PxPhysicsAPI.h>
#include <foundation\PxFoundation.h>

using namespace physx;

#include <AntTweakBar.h>
#include "entity_inspector.h"
#include "render\all_post_process_effects.h"
#include "render/render_instances.h"

#include "audio\sound.h"
#include "audio\sound_manager.h"

#include "theoraplayer/TheoraPlayer.h"
#include "theoraplayer/TheoraDataSource.h"
#include "theoraplayer/TheoraVideoManager.h"

static CApp the_app;

CLogicManager			 &logic_manager = CLogicManager::get();
CEntityInspector		 &entity_inspector = CEntityInspector::get();
CEntityLister			 &entity_lister = CEntityLister::get();
CEntityActioner			 &entity_actioner = CEntityActioner::get();
CDebugOptioner			 &debug_optioner = CDebugOptioner::get();
CConsole				 &console = CConsole::get();
CPostProcessOptioner	 &post_process_optioner = CPostProcessOptioner::get();
CSoundManager            &sm = CSoundManager::get();

CEntityManager &entity_manager = CEntityManager::get();
CPhysicsManager &physics_manager = CPhysicsManager::get();
std::mutex	preLoadMutex;

// Video
TheoraVideoManager *mgr;
TheoraVideoClip *clip = nullptr;

unsigned long endframe;
ID3D11Texture2D* tex;
CTexture* videoTexture;
ID3D11ShaderResourceView* m_shaderResourceView;

const unsigned int max_num_needles = 512;

#include "ai\ai_basic_patroller.h"
#include "io\iostatus.h"

#ifndef FINAL_RELEASE
	#include "windows.h"
	#include "psapi.h"
#endif

CApp& CApp::get() {
	return the_app;
}

CApp::CApp()
	: xres(640)
	, yres(480)
	, time_modifier(1)
	, slow_motion_counter(0)
{
	bar = nullptr;
}

void CApp::loadConfig() {
	// Parse xml file...
	COptionsParser p;
	bool success = p.xmlParseFile("settings.xml");
	if (success){
		int x_res;
		int y_res;
		bool mode;
		p.getResolution(x_res, y_res, mode);
		xres = x_res;
		yres = y_res;
		fullscreen = mode;
		first_scene = p.first_scene;
	}
	else{
		xres = 800;
		yres = 600;
	}
}

// Debug 
const CRenderTechnique* debugTech;
const CRenderTechnique* ropeTech;
CMesh		 wiredCube;
CMesh		 intersectsWiredCube;
CMesh		 rope;

CHandle		 life;
CHandle		 h_player;

CFont         font;
CDeferredRender deferred;
CRenderToTexture* rt_base;

const CTexture* cubemap;

float fixedUpdateCounter;
float fps;
bool debug_mode;

TSharpenStep sharpen;
TSSAOStep ssao;
TChromaticAberrationStep chromatic_aberration;
TBlurStep blur;
TBlurCameraStep blur_camera;
TGlowStep glow;
TSilouetteStep silouette;
TUnderwaterEffect underwater;
TSSRRStep ssrr;
TFogStep fog;
TLensFlareStep lens_flare;

//---------------------------------------------------
//CNavmesh nav_prueba;
//---------------------------------------------------

void registerAllComponentMsgs() {
	//SUBSCRIBE(TCompLife, TMsgExplosion, onExplosion);
	SUBSCRIBE(TCompRigidBody, TMsgExplosion, onExplosion);
	SUBSCRIBE(TCompAiFsmBasic, TGroundHit, groundHit);
	SUBSCRIBE(TCompBasicPlayerController, TActorHit, actorHit);

	// Boss Events
	SUBSCRIBE(TCompExplosion, TMsgOnDetonate, onDetonate);
	SUBSCRIBE(TCompHitch, TMsgRopeTensed, onRopeTensed);

	//IA events
	SUBSCRIBE(TCompBtGrandma, TActorHit, actorHit);
	SUBSCRIBE(TCompBtGrandma, TWarWarning, warWarning);
	SUBSCRIBE(TCompBtGrandma, TPlayerFound, notifyPlayerFound);
	//SUBSCRIBE(TCompBtGrandma, TPlayerTouch, notifyPlayerTouch);
	SUBSCRIBE(TCompBtGrandma, TMsgRopeTensed, onRopeTensed);
	SUBSCRIBE(TCompSubstituteBoss, TMsgRopeTensed, onRopeTensed);

	SUBSCRIBE(TCompBtSoldier, TActorHit, actorHit);
	SUBSCRIBE(TCompBtSoldier, TWarWarning, warWarning);
	SUBSCRIBE(TCompBtSoldier, TPlayerFound, notifyPlayerFound);
	//SUBSCRIBE(TCompBtSoldier, TPlayerTouch, notifyPlayerTouch);
	SUBSCRIBE(TCompBtSoldier, TMsgRopeTensed, onRopeTensed);
	SUBSCRIBE(TCompBtSoldier, TMsgNeedleHit, onNeedleHit);


	SUBSCRIBE(TCompBasicPlayerController, TMsgAttackDamage, onAttackDamage);
	SUBSCRIBE(TCompPlayerController, TActorHit, actorHit);
	SUBSCRIBE(TCompPlayerController, TMsgAttackDamage, onAttackDamage);
	SUBSCRIBE(TCompAiFsmBasic, TActorHit, actorHit);
	SUBSCRIBE(TCompAiFsmBasic, TMsgRopeTensed, onRopeTensed);
	SUBSCRIBE(TCompVictoryCond, TVictoryCondition, victory);
}

void createManagers() {
	SET_ERROR_CONTEXT("Creating", "managers");

	getObjManager<CEntity>()->init(4096);
	getObjManager<TCompTransform>()->init(4096);
	getObjManager<TCompLife>()->init(32);
	getObjManager<TCompName>()->init(4096);
	getObjManager<TCompMesh>()->init(4096);
	getObjManager<TCompRender>()->init(4096);
	getObjManager<TCompRecastAABB>()->init(32);
	getObjManager<TCompColliderMesh>()->init(4096);
	getObjManager<TCompColliderConvex>()->init(512);
	getObjManager<TCompCamera>()->init(32);
	getObjManager<TCompColliderBox>()->init(1024);
	getObjManager<TCompColliderSphere>()->init(512);
	getObjManager<TCompColliderCapsule>()->init(512);
	getObjManager<TCompColliderMultiple>()->init(512);
	getObjManager<TCompRigidBody>()->init(2048);
	getObjManager<TCompStaticBody>()->init(4096);
	getObjManager<TCompAABB>()->init(4096);
	getObjManager<TCompGNLogic>()->init(32);
	getObjManager<TCompGNItem>()->init(64);
	getObjManager<TCompZoneAABB>()->init(32);
	getObjManager<TCompHfxZone>()->init(32);
	getObjManager<TCompGoldenNeedle>()->init(32);
	getObjManager<TCompPlayerController>()->init(1);
	getObjManager<TCompPlayerPivotController>()->init(1);
	getObjManager<TCompCameraPivotController>()->init(1);
	getObjManager<TCompThirdPersonCameraController>()->init(1);
	getObjManager<TCompViewerCameraController>()->init(1);
	getObjManager<TCompDistanceJoint>()->init(256);
	getObjManager<TCompJointPrismatic>()->init(256);
	getObjManager<TCompJointHinge>()->init(256);
	getObjManager<TCompJointD6>()->init(512);
	getObjManager<TCompJointFixed>()->init(512);
	getObjManager<TCompRope>()->init(128);
	getObjManager<TCompNeedle>()->init(max_num_needles);
	getObjManager<TCompPlayerPosSensor>()->init(64);
	getObjManager<TCompSensorNeedles>()->init(64);
	getObjManager<TCompSensorTied>()->init(64);
	getObjManager<TCompSensorDistPlayer>()->init(64);
	getObjManager<TCompTrigger>()->init(256);	
	getObjManager<TCompDistanceText>()->init(32);
	getObjManager<TCompVictoryCond>()->init(1);
	
	// Otros
	getObjManager<TCompExplosion>()->init(64);
	getObjManager<TCompBossPrefab>()->init(512);
	getObjManager<TCompPlayerRope>()->init(2);

	// Interruptores
	getObjManager<TCompSwitchController>()->init(32);
	getObjManager<TCompSwitchPullController>()->init(32);
	getObjManager<TCompSwitchPushController>()->init(32);

	// Plataformas
	getObjManager<TCompPlatformPath>()->init(32);

	// Lights (temporary)
	getObjManager<TCompDirectionalLight>()->init(16);
	getObjManager<TCompAmbientLight>()->init(1);
	getObjManager<TCompPointLight>()->init(512);

	// AI
	getObjManager<TCompAiFsmBasic>()->init(64);
	getObjManager<TCompEnemyController>()->init(64);
	getObjManager<TCompBtGrandma>()->init(64);
	getObjManager<TCompBtSoldier>()->init(64);
	getObjManager<TCompAiBoss>()->init(1);
	getObjManager<TCompSubstituteBoss>()->init(1);
	
	getObjManager<TCompHitch>()->init(4);

	getObjManager<TCompCharacterController>()->init(64);
	getObjManager<TCompUnityCharacterController>()->init(64);
	getObjManager<TCompBasicPlayerController>()->init(1);

	getObjManager<TCompSkeleton>()->init(256);
	getObjManager<TCompSkeletonLookAt>()->init(256);
	getObjManager<TCompSkeletonIK>()->init(256);

	getObjManager<TCompRagdoll>()->init(64);
	getObjManager<TCompShadows>()->init(8);

	getObjManager<TCompParticleGroup>()->init(512);
	getObjManager<TCompParticleEditor>()->init(1);
	getObjManager<TCompAnimEditor>()->init(1);

	//Audio
	getObjManager<TCompAudioListener>()->init(1024);
	getObjManager<TCompAudioSource>()->init(1024);

	getObjManager<TCompOcclusionPlane>()->init(128);

	// GUI
	getObjManager<TCompButton>()->init(32);

	// Animation
	getObjManager<TCompVibration>()->init(256);
	getObjManager<TCompLocalRotation>()->init(256);

	registerAllComponentMsgs();
}

void initManagers() {
	CErrorContext ec("Initializing", "managers");

	getObjManager<TCompTransform>()->initHandlers();
	getObjManager<TCompCamera>()->initHandlers();
	getObjManager<TCompRecastAABB>()->initHandlers();
	getObjManager<TCompColliderBox>()->initHandlers();
	getObjManager<TCompColliderSphere>()->initHandlers();
	getObjManager<TCompColliderCapsule>()->initHandlers();
	getObjManager<TCompColliderMesh>()->initHandlers();
	getObjManager<TCompColliderConvex>()->initHandlers();
	getObjManager<TCompColliderMultiple>()->initHandlers();
	getObjManager<TCompRigidBody>()->initHandlers();
	getObjManager<TCompStaticBody>()->initHandlers();
	getObjManager<TCompAABB>()->initHandlers();
	getObjManager<TCompGNLogic>()->initHandlers();
	getObjManager<TCompGNItem>()->initHandlers();
	getObjManager<TCompZoneAABB>()->initHandlers();
	getObjManager<TCompHfxZone>()->initHandlers();
	//getObjManager<TCompGoldenNeedle>()->initHandlers();
	//getObjManager<TCompUnityCharacterController>()->initHandlers();
	getObjManager<TCompPlayerController>()->initHandlers();
	getObjManager<TCompPlayerPivotController>()->initHandlers();
	getObjManager<TCompCameraPivotController>()->initHandlers();
	getObjManager<TCompThirdPersonCameraController>()->initHandlers();
	getObjManager<TCompDistanceJoint>()->initHandlers();
	getObjManager<TCompJointPrismatic>()->initHandlers();
	getObjManager<TCompJointD6>()->initHandlers();
	getObjManager<TCompJointFixed>()->initHandlers();
	getObjManager<TCompEnemyController>()->initHandlers();

	getObjManager<TCompPlayerPosSensor>()->initHandlers();
	getObjManager<TCompSensorNeedles>()->initHandlers();
	getObjManager<TCompSensorTied>()->initHandlers();
	getObjManager<TCompViewerCameraController>()->initHandlers();

	// BOSS
	getObjManager<TCompExplosion>()->initHandlers();
	getObjManager<TCompBossPrefab>()->initHandlers();
	getObjManager<TCompHitch>()->initHandlers();
	getObjManager<TCompSubstituteBoss>()->initHandlers();	

	// PLATFORMS
	getObjManager<TCompPlatformPath>()->initHandlers();

	// SWITCHS
	getObjManager<TCompSwitchPullController>()->initHandlers();
	getObjManager<TCompSwitchPushController>()->initHandlers();

	getObjManager<TCompTrigger>()->initHandlers();
	getObjManager<TCompDistanceText>()->initHandlers();
	getObjManager<TCompVictoryCond>()->initHandlers();

	getObjManager<TCompBasicPlayerController>()->initHandlers();
	getObjManager<TCompAiFsmBasic>()->initHandlers();
	getObjManager<TCompBtGrandma>()->initHandlers();
	getObjManager<TCompBtSoldier>()->initHandlers();
	getObjManager<TCompAiBoss>()->initHandlers();
	

	getObjManager<TCompSkeleton>()->initHandlers();
	getObjManager<TCompShadows>()->initHandlers();

	getObjManager<TCompParticleGroup>()->initHandlers();
	getObjManager<TCompParticleEditor>()->initHandlers();
	getObjManager<TCompAnimEditor>()->initHandlers();
	getObjManager<TCompPlayerRope>()->initHandlers();

	//AUDIO
	getObjManager<TCompAudioListener>()->initHandlers();
	getObjManager<TCompAudioSource>()->initHandlers();

	//GUI
	getObjManager<TCompButton>()->initHandlers();

	// Animation
	//getObjManager<TCompVibration>()->initHandlers();

}

void CApp::preLoad(){
	preLoadMutex.lock();
	/*CImporterResourceLoader loader;
	loader.xmlParseFile("data/scenes/scene_final_boss.xml");
	loader.xmlParseFile("data/scenes/scene_4.xml");
	loader.xmlParseFile("data/scenes/scene_3.xml");
	loader.xmlParseFile("data/scenes/scene_2.xml");
	loader.xmlParseFile("data/scenes/scene_1.xml");	*/
	preLoadMutex.unlock();
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
	srand((unsigned int)time(0));

	createManagers();	

#ifdef _DEBUG
	// Init AntTweakBar
	TwInit(TW_DIRECT3D11, ::render.device);
	TwWindowSize(xres, yres);
#endif
#ifndef FINAL_RELEASE
	rope_thrown = false;
	physx_clamp = false;
	draw_lights = true;
#endif

	draw_gui = true;

	bool is_ok = true;

	rt_base = nullptr;

	// Boot LUA
	logic_manager.bootLUA();

	XASSERT(font.create(), "Error creating the font");
		

	// Initialize render
	rt_base = new CRenderToTexture;
	rt_base->create("deferred_output", xres, yres, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_UNKNOWN, CRenderToTexture::USE_BACK_ZBUFFER);

	is_ok &= renderUtilsCreate();

	is_ok &= deferred.create(xres, yres);

	is_ok &= sharpen.create("sharpen", xres, yres, 1);
	is_ok &= ssao.create("ssao", xres, yres, 2);
	is_ok &= chromatic_aberration.create("chromatic_aberration", xres, yres, 1);
	is_ok &= blur.create("blur", xres, yres, 1);
	is_ok &= blur_camera.create("blur_camera", xres, yres, 1);
	is_ok &= glow.create("glow", xres, yres, 4);
	is_ok &= underwater.create("underwater", xres, yres, 1);
	is_ok &= ssrr.create("ssrr", xres, yres, 1);
	is_ok &= silouette.create("silouette", xres, yres, 1);
	is_ok &= lens_flare.create("lens_flare", xres, yres, 1);
	is_ok &= fog.create("fog", xres, yres, 1);

	// Create Debug Technique
	debugTech = render_techniques_manager.getByName("basic");
	ropeTech = render_techniques_manager.getByName("rope");

	// global ctes
	is_ok &= ctes_global.create();
	
	physics_manager.init();
	
	// Preload scenes
	bar = new std::thread(&CApp::preLoad, this);
	
	menu_scene = "data/scenes/scene_menu.xml";

	game_state = TGameState::MAIN_MENU;
	loadScene(first_scene);

	// Create debug meshes	
	is_ok = createUnitWiredCube(wiredCube, XMFLOAT4(1.f, 1.f, 1.f, 1.f));
	is_ok &= createUnitWiredCube(intersectsWiredCube, XMFLOAT4(1.f, 0.f, 0.f, 1.f));

	XASSERT(is_ok, "Error creating debug meshes");

#ifdef _DEBUG

	entity_inspector.init();
	entity_inspector.inspectEntity(CHandle());

	entity_lister.init();
	entity_actioner.init();
	debug_optioner.init();
	console.init();
	post_process_optioner.sharpen = &sharpen;
	post_process_optioner.chromatic_aberration = &chromatic_aberration;
	post_process_optioner.blur = &blur;
	post_process_optioner.ssrr = &ssrr;

	post_process_optioner.init();


	entity_lister.update();
#endif

	fps = 0;

	// Timer test
	logic_manager.setTimer("TestTimer", 2);

	assert(is_ok);

	//PRUEBAS NAV MESHES -----------------
	//bool valid = CNav_mesh_manager::get().build_nav_mesh();
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

	//Runs the ai thread
	CNav_mesh_manager::get().nav_mesh_init();

	TIME_ACCUM = 0;
	time_since_last_update = 0;

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
	
	
	float pxStep = physics_manager.timeStep;
	before = now;

	const int FRAME_LIMIT = 60;
	const double TIME_PER_FRAME = 1.0f / FRAME_LIMIT;
	const float MIN_PHYSX_DELTA = TIME_PER_FRAME * 0.0f;

	fps = 0.0f;
	// To avoid the fist huge delta time
	if (delta_secs < 0.5) {
		time_since_last_update += delta_secs;
		TIME_ACCUM += delta_secs;

		CIOStatus& io = CIOStatus::get();
		

		if (TIME_ACCUM >= 0) {

			fps = (float)(1.0f / time_since_last_update);
#ifndef FINAL_RELEASE
			TFrame f;
			f.fps = fps;
			frames_d.push_front(f);
			if (frames_d.size() > xres) {
				frames_d.pop_back();
			}
#endif

			delta_time = time_since_last_update;
			total_time += delta_time;

			// Update input
			io.update(delta_time);
			//io.gameUpdate(delta_time);
#ifndef FINAL_RELEASE

			// Pause
			if (CIOStatus::get().becomesReleased(CIOStatus::E)){
				pause = !pause;
			}
#endif
			// Slow motion
			if (slow_motion_counter > 0) {
				slow_motion_counter -= delta_time;
				if (slow_motion_counter <= 0) {
					CSoundManager::get().desactivateSlowMo();
					time_modifier = 1;
					slow_motion_counter = 0;
				}
			}

			delta_time *= time_modifier;
			pxStep *= time_modifier;

			// Fixed update
			if (total_time > 0.5) {
				fixedUpdateCounter += delta_time;

				//while (fixedUpdateCounter > pxStep) {
				//if (fixedUpdateCounter > pxStep) {
				//fixedUpdateCounter -= pxStep;
				//fixedUpdateCounter = 0;
				if (delta_time < MIN_PHYSX_DELTA) {
#ifndef FINAL_RELEASE
					physx_clamp = true;
#endif
					fixedUpdate(MIN_PHYSX_DELTA);
				}
				else {
					fixedUpdate(delta_time);
				}

				//}


				/*while (fixedUpdateCounter > pxStep) {
					fixedUpdateCounter -= pxStep;
					fixedUpdate(pxStep);
					}*/
			}

			update(delta_time);
			entity_manager.destroyRemovedHandles();

			time_since_last_update = 0;
			//TIME_ACCUM = 0;
			TIME_ACCUM -= TIME_PER_FRAME;

			//io.cleanKeys();
			render();

		}
	}
}

void CApp::update(float elapsed) {

	if (game_state == TGameState::INITIAL_VIDEO) {
		if (!video_sound_played) {
			CSoundManager::get().playEvent("INITIAL_VIDEO", "video_sound");
			logic_manager.playSubtitles("INITIAL_VIDEO");
			video_sound_played = true;
		}
		CSoundManager::get().update(elapsed);
		logic_manager.update(elapsed);

		if (CIOStatus::get().becomesReleased(CIOStatus::EXIT)){
			game_state = TGameState::GAMEPLAY;
			CSoundManager::get().stopNamedInstance("video_sound", FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_IMMEDIATE);
			CSoundManager::get().update(elapsed);
			clip->stop();
			mgr->destroyVideoClip(clip);
			videoTexture->destroy();
			delete videoTexture;
			loadScene("data/scenes/scene_1.xml");
		}
		return;
	}

	if (game_state == TGameState::FINAL_VIDEO && current_scene_name == "empty_scene") {
		if (!video_sound_played) {
			CSoundManager::get().playEvent("FINAL_VIDEO", "video_sound");
			logic_manager.playSubtitles("FINAL_VIDEO");
			video_sound_played = true;
		}
		CSoundManager::get().update(elapsed);
		logic_manager.update(elapsed);

		if (CIOStatus::get().becomesReleased(CIOStatus::EXIT)){
			game_state = TGameState::MAIN_MENU;
			CSoundManager::get().stopNamedInstance("video_sound", FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_IMMEDIATE);
			CSoundManager::get().update(elapsed);
			clip->stop();
			mgr->destroyVideoClip(clip);
			videoTexture->destroy();
			delete videoTexture;
			loadScene(menu_scene);
		}
		return;
	}

	if (pause)
		return;

	CIOStatus& io = CIOStatus::get();
	// Update input

	if (CIOStatus::get().becomesReleased(CIOStatus::EXIT)){
		if (game_state == TGameState::MAIN_MENU) {
			logic_manager.exitGame(); 
		}
		else {
			game_state = TGameState::MAIN_MENU;
			logic_manager.loadScene(menu_scene);
		}
	}

#ifndef FINAL_RELEASE
	if (io.isPressed(CIOStatus::EXTRA)) {
	}

	draw_lights = !io.isPressed(CIOStatus::ALT);

	// Slow motion
	if (io.becomesReleased(CIOStatus::Q)) {
		if (time_modifier == 1) {
			time_modifier = 0.05f;
			CSoundManager::get().activateSlowMo();
		}
		else{
			time_modifier = 1;
			CSoundManager::get().desactivateSlowMo();
		}
	}
//#ifdef _DEBUG
	if (io.becomesReleased(CIOStatus::NUM0)) { debug_map = 0; }
	if (io.becomesReleased(CIOStatus::NUM1)) { debug_map = 1; }
	if (io.becomesReleased(CIOStatus::NUM2)) { debug_map = 2; }
	if (io.becomesReleased(CIOStatus::NUM3)) { debug_map = 3; }
	if (io.becomesReleased(CIOStatus::NUM4)) { debug_map = 4; }
	if (io.becomesReleased(CIOStatus::NUM5)) { debug_map = 5; }
	if (io.becomesReleased(CIOStatus::NUM6)) { debug_map = 6; }
//#endif
	if (io.isPressed(CIOStatus::CTRL)) {
		if (io.becomesReleased(CIOStatus::NUM1)) { loadScene("data/scenes/scene_1.xml"); }
		if (io.becomesReleased(CIOStatus::NUM2)) { loadScene("data/scenes/scene_2.xml"); }
		if (io.becomesReleased(CIOStatus::NUM3)) { loadScene("data/scenes/scene_3.xml"); }
		if (io.becomesReleased(CIOStatus::NUM4)) { loadScene("data/scenes/scene_4.xml"); }
		if (io.becomesReleased(CIOStatus::NUM5)) { loadScene("data/scenes/scene_final_boss.xml"); }
		if (io.becomesReleased(CIOStatus::NUM6)) {}
		if (io.becomesReleased(CIOStatus::NUM7)) { /*loadScene("data/scenes/scene_3_noenemy.xml");*/ }
		if (io.becomesReleased(CIOStatus::NUM8)) { loadScene("data/scenes/my_file.xml"); }
	}

	/*if (io.becomesReleased(CIOStatus::F8_KEY)) {
		renderWireframe = !renderWireframe;
	}*/

	if (io.becomesReleased(CIOStatus::F4_KEY)) {
		
	}

	if (io.becomesReleased(CIOStatus::F8_KEY)) {
		render_techniques_manager.reload("blur");
		render_techniques_manager.reload("glow");
		render_techniques_manager.reload("glow_lights");
		render_techniques_manager.reload("blur_camera");		
		render_techniques_manager.reload("ssrr");
		render_techniques_manager.reload("silouette");
		render_techniques_manager.reload("silouette_type");
		render_techniques_manager.reload("silouette_glow");
		render_techniques_manager.reload("deferred_gbuffer");
		render_techniques_manager.reload("deferred_gbuffer_cloth");
		render_techniques_manager.reload("deferred_resolve");
		render_techniques_manager.reload("skin_basic");
		render_techniques_manager.reload("underwater");
		render_techniques_manager.reload("ssao");
		render_techniques_manager.reload("ssao_blur");
		render_techniques_manager.reload("fog");
		render_techniques_manager.reload("lens_flare");
		/*render_techniques_manager.reload("deferred_point_lights");
		render_techniques_manager.reload("deferred_dir_lights");
		render_techniques_manager.reload("deferred_resolve");
		render_techniques_manager.reload("particles");
		render_techniques_manager.reload("particles_dist");
		render_techniques_manager.reload("gen_shadows");
		render_techniques_manager.reload("gen_shadows_skel");
		render_techniques_manager.reload("light_shaft");
		render_techniques_manager.reload("distorsion");
		render_techniques_manager.reload("ssrr");*/
		/*render_techniques_manager.reload("chromatic_aberration");
		render_techniques_manager.reload("deferred_dir_lights");
		render_techniques_manager.reload("skin_basic");
		texture_manager.reload("Foco_albedo");
		texture_manager.reload("Foco_normal");*/
		//render_techniques_manager.reload("underwater");		
		//render_techniques_manager.reload("deferred_point_lights");
	}
#endif
	if (io.becomesReleased(CIOStatus::F4_KEY)) {
		logic_manager.execute("onExtraButtonPressed()");
	}
	// Water level
	CEntity* water = entity_manager.getByName("water");
	if (water) {
		TCompTransform* water_t = water->get<TCompTransform>();
		water_level = XMVectorGetY(water_t->position);
		underwater.water_level = water_level;
		CEntity* player = entity_manager.getByName("PlayerCamera");
		if (player) {
			TCompTransform* player_t = player->get<TCompTransform>();
			float p_y = XMVectorGetY(player_t->position);
			underwater.amount = p_y > water_level ? 0.0f : 1.0f;
		}
	}
	else
	{
		underwater.amount = 0;
	}


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
	//CEntity* e = CEntityManager::get().getByName("Player");
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
	ctes_global.get()->elapsed = elapsed;
	ctes_global.get()->global_water_level = max(ctes_global.get()->global_water_level, water_level);

	int needle_count = 0;
	for (auto& string : CRope_manager::get().getStrings()) {
		TCompRope* rope = string;
		if (rope) {
			XMVECTOR static_pos;
			if (rope->getStaticPosition(static_pos)) {
				ctes_global.get()->static_needles[needle_count] = static_pos;
			}
		}		 
		needle_count++;
	}

	for (int i = 0; i < (4 - needle_count); ++i) {
		ctes_global.get()->static_needles[needle_count + i] = XMVectorSet(-1, -1, -1, -1);
	}

	getObjManager<TCompTransform>()->update(elapsed);
	getObjManager<TCompAABB>()->update(elapsed); // Update objects AABBs
	getObjManager<TCompGNLogic>()->update(elapsed);
	getObjManager<TCompGNItem>()->update(elapsed);
	
	getObjManager<TCompUnityCharacterController>()->update(elapsed);
	getObjManager<TCompCharacterController>()->update(elapsed);
	getObjManager<TCompSkeleton>()->update(elapsed);
	getObjManager<TCompSkeletonLookAt>()->update(elapsed);
	getObjManager<TCompSkeletonIK>()->update(elapsed);
	getObjManager<TCompPlayerController>()->update(elapsed); // Update player transform
	getObjManager<TCompPlayerPivotController>()->update(elapsed);
	getObjManager<TCompCameraPivotController>()->update(elapsed);
	getObjManager<TCompThirdPersonCameraController>()->update(elapsed); // Then update camera transform, wich is relative to the player
	
	getObjManager<TCompViewerCameraController>()->update(elapsed);
	getObjManager<TCompCamera>()->update(elapsed);  // Then, update camera view and projection matrix

	// OTHER
	getObjManager<TCompBossPrefab>()->update(elapsed);
	getObjManager<TCompExplosion>()->update(elapsed);
	getObjManager<TCompSubstituteBoss>()->update(elapsed);

	// AI
	getObjManager<TCompAiFsmBasic>()->update(elapsed);
	getObjManager<TCompBtGrandma>()->update(elapsed);
	getObjManager<TCompBtSoldier>()->update(elapsed);
	getObjManager<TCompAiBoss>()->update(elapsed);

	// SWITCH
	getObjManager<TCompSwitchController>()->update(elapsed);
	getObjManager<TCompSwitchPullController>()->update(elapsed);
	getObjManager<TCompSwitchPushController>()->update(elapsed);

	getObjManager<TCompTrigger>()->update(elapsed);
	//getObjManager<TCompRecastAABB>()->update(elapsed);
	getObjManager<TCompDistanceText>()->update(elapsed);
	getObjManager<TCompBasicPlayerController>()->update(elapsed);

	// PARTICLES
	getObjManager<TCompParticleGroup>()->update(elapsed);
	getObjManager<TCompParticleEditor>()->update(elapsed);
	getObjManager<TCompAnimEditor>()->update(elapsed);

	//AUDIO
	getObjManager<TCompAudioListener>()->update(elapsed);
	getObjManager<TCompAudioSource>()->update(elapsed);

	//GUI
	getObjManager<TCompButton>()->update(elapsed);

	// ANIMATION
	getObjManager<TCompVibration>()->update(elapsed);
	getObjManager<TCompLocalRotation>()->update(elapsed);
	getObjManager<TCompPlayerRope>()->update(elapsed);

#ifdef _DEBUG
	entity_inspector.update();
	entity_lister.update();
	entity_actioner.update();
#endif

	CSoundManager::get().update(elapsed);

	//-----------------------------------------------------------------------------------------
	CNav_mesh_manager::get().checkUpdates();
	CNav_mesh_manager::get().checkDistaceToEnemies();
	//-----------------------------------------------------------------------------------------

	// Camera menu test
	CEntity* e = entity_manager.getByName("camera_menu");
	if (CHandle(e).isValid()) {
		TCompTransform* cam_trans = e->get<TCompTransform>();
		XMVECTOR quat2 = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), io.getMouse().dx * elapsed * -0.0005f);
		XMVECTOR quat1 = XMQuaternionRotationAxis(XMVectorSet(1, 0, 0, 0), io.getMouse().dy * elapsed * -0.0005f);
		cam_trans->rotation = XMQuaternionMultiply(cam_trans->rotation, quat1);
		cam_trans->rotation = XMQuaternionMultiply(cam_trans->rotation, quat2);
		
	}

	logic_manager.update(elapsed);
}

// Physics update
void CApp::fixedUpdate(float elapsed) {

	if (game_state == TGameState::INITIAL_VIDEO)
		return;

	if (pause)
		return;

	physics_manager.gScene->simulate(elapsed);
	physics_manager.gScene->fetchResults(true);

	getObjManager<TCompPlayerController>()->fixedUpdate(elapsed); // Update kinematic player
	getObjManager<TCompEnemyController>()->fixedUpdate(elapsed);
	getObjManager<TCompRope>()->fixedUpdate(elapsed);
	getObjManager<TCompNeedle>()->fixedUpdate(elapsed);
	getObjManager<TCompGoldenNeedle>()->fixedUpdate(elapsed);
	getObjManager<TCompUnityCharacterController>()->fixedUpdate(elapsed);
	//getObjManager<TCompBasicPlayerController>()->fixedUpdate(elapsed);
	getObjManager<TCompPlatformPath>()->fixedUpdate(elapsed);
	getObjManager<TCompCharacterController>()->fixedUpdate(elapsed);
	getObjManager<TCompRigidBody>()->fixedUpdate(elapsed); // Update rigidBodies of the scene
	getObjManager<TCompStaticBody>()->fixedUpdate(elapsed);
	getObjManager<TCompRagdoll>()->fixedUpdate(elapsed);
	getObjManager<TCompParticleGroup>()->fixedUpdate(elapsed);
	getObjManager<TCompAiBoss>()->fixedUpdate(elapsed);
	getObjManager<TCompPlayerRope>()->fixedUpdate(elapsed);
}

void CApp::render() {

	if (game_state == TGameState::INITIAL_VIDEO) {
		bool playVideo = renderVideo();
		if (!playVideo) {
			game_state = TGameState::GAMEPLAY;
			loadScene("data/scenes/scene_1.xml");
			if (bar && bar->joinable()){
				bar->join();
				delete bar;
				bar = nullptr;
			}
		}
		logic_manager.draw();
		::render.swap_chain->Present(0, 0);
		return;
	}

	if (game_state == TGameState::FINAL_VIDEO) {
		bool playVideo = renderVideo();
		if (!playVideo) {
			game_state = TGameState::MAIN_MENU;
			loadScene(menu_scene);
			if (bar && bar->joinable()){
				bar->join();
				delete bar;
				bar = nullptr;
			}
		}
		logic_manager.draw();
		::render.swap_chain->Present(0, 0);
		return;
	}

	CCamera camera = *(TCompCamera*)render_manager.activeCamera;
	{
		CTraceScoped scope1("render_init");
		// Render ---------------------
		float ClearColor[4] = { 0.1f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
		::render.ctx->ClearRenderTargetView(::render.render_target_view, ClearColor);
		::render.ctx->ClearDepthStencilView(::render.depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);

		ctes_global.uploadToGPU();
		ctes_global.activateInVS(2);
		ctes_global.activateInPS(2);
		activateTextureSamplers();


		/*CHandle h_light = entity_manager.getByName("the_light");
		CEntity* e_light = h_light;
		if (e_light) {
		TCompCamera* cam_light = e_light->get<TCompCamera>();
		activateLight(*cam_light, 4);
		}*/

		activateZConfig(ZConfig::ZCFG_DEFAULT);

		// Generate the culling for the active camera
		render_manager.cullActiveCamera();
	}

	// Generate all shadows maps
	{
		CTraceScoped scope("gen_shadows");
		getObjManager<TCompShadows>()->onAll(&TCompShadows::generate);
	}

	deferred.render(&camera, *rt_base);

	setSlotNull(0);

	deferred.rt_albedo->activate();
	activateZConfig(ZConfig::ZCFG_DISABLE_ALL);
	drawTexture2D(0, 0, xres, yres, rt_base);
	activateZConfig(ZConfig::ZCFG_DEFAULT);

	{
		CTraceScoped scope_part("Particles");

		setSlotNull(0);

		rt_base->activate();
		texture_manager.getByName("rt_albedo")->activate(0);
		texture_manager.getByName("noise")->activate(9);
		getObjManager<TCompParticleGroup>()->onAll(&TCompParticleGroup::render);
	}

	setSlotNull(0);

	deferred.rt_albedo->activate();
	activateZConfig(ZConfig::ZCFG_DISABLE_ALL);
	drawTexture2D(0, 0, xres, yres, rt_base);
	activateZConfig(ZConfig::ZCFG_DEFAULT);

	setSlotNull(0);

	rt_base->activate();
	texture_manager.getByName("rt_albedo")->activate(0);
	getObjManager<TCompParticleGroup>()->onAll(&TCompParticleGroup::renderDistorsion);

	activateBlendConfig(BLEND_CFG_DEFAULT);
	renderEntities();

	setSlotNull(0);
	{
		CTraceScoped scope2("transparency");
		activateZConfig(ZCFG_TEST_BUT_NO_WRITE);
		render_manager.renderAll(&camera, false, false);
		activateRSConfig(RSCFG_REVERSE_CULLING);
		render_manager.renderAll(&camera, false, true);
		activateRSConfig(RSCFG_DEFAULT);
		activateZConfig(ZCFG_DEFAULT);
	}
	{
		CTraceScoped scope_post_all("Post procesado");
		activateCamera(camera, 1);
		/*{
			CTraceScoped scope_post1("SSAO");
			ssao.apply(rt_base);
		}*/
		{
			CTraceScoped scope_post1("SSR");
			ssrr.apply(rt_base);
		}
		{
			CTraceScoped scope_post1("Fog");
			fog.apply(ssrr.getOutput());
		}
		{
			CTraceScoped scope_post2("Sharpen");
			sharpen.apply(fog.getOutput());
		}
		{
			CTraceScoped scope_post3("Chromatic aberration");
			chromatic_aberration.apply(sharpen.getOutput());
		}		
		{
			CTraceScoped scope_post5("Depth of field");
			blur.apply(chromatic_aberration.getOutput());
		}
		{
			CTraceScoped scope_post6("Motion blur");
			blur_camera.apply(blur.getOutput());
		}
		{
			CTraceScoped scope_post7("Silouette");
			silouette.apply(blur_camera.getOutput());
			//silouette.apply(blur_camera.getOutput());
		}
		texture_manager.getByName("noise")->activate(9);
		{
			CTraceScoped scope_post8("Glow");
			glow.apply(silouette.getOutput());
		}
		{
			CTraceScoped scope_post4("Underwater");
			underwater.apply(glow.getOutput());
		}
	}

	CTraceScoped scope_final("Final draw");	
	::render.activateBackbuffer();
	static int sz = 150;

	//drawTexture2D(0, 0, xres, yres, texture_manager.getByName("rt_normals"));
	//drawTexture2D(0, 0, sz, sz, texture_manager.getByName("rt_albedo"));

	activateZConfig(ZConfig::ZCFG_DISABLE_ALL);
	//ssao.getOutput()->activate(1);

	//texture_manager.getByName("rt_depth")->activate(2);

	//drawTexture2D(0, 0, xres, yres, rt_base, "sharpen");
	//drawTexture2D(0, 0, xres, yres, texture_manager.getByName("rt_lights"));
	//drawTexture2D(0, 0, xres, yres, texture_manager.getByName("rt_depth")); 

	drawTexture2D(0, 0, xres, yres, underwater.getOutput());

	/*
	CHandle h_light = entity_manager.getByName("the_light");
	CEntity* e_light = h_light;
	TCompShadows* shadow = e_light->get<TCompShadows>();

	*/

	// 0: Nothing, 1: Albedo, 2: Normals, 3: Specular, 4: Gloss, 5: Lights, 6: Depth

//#ifdef _DEBUG
	/*drawTexture2D(0, 0, sz * camera.getAspectRatio(), sz, texture_manager.getByName("rt_depth"));
	drawTexture2D(0, sz, sz * camera.getAspectRatio(), sz, texture_manager.getByName("rt_lights"));
	//drawTexture2D(0, 2*sz, sz * camera.getAspectRatio(), sz, shadow->rt.getZTexture());	
	drawTexture2D(0, 3 * sz, sz * camera.getAspectRatio(), sz, texture_manager.getByName("rt_normals"));
	drawTexture2D(0, 4 * sz, sz * camera.getAspectRatio(), sz, texture_manager.getByName("rt_albedo"));
	drawTexture2D(sz * 2, 0 * sz, sz * camera.getAspectRatio(), sz, texture_manager.getByName("rt_specular"));
	drawTexture2D(sz * 2, 1 * sz, sz * camera.getAspectRatio(), sz, texture_manager.getByName("rt_gloss"));*/

	if (debug_map == 1) { drawTexture2D(0, 0, xres, yres, texture_manager.getByName("rt_albedo")); }
	if (debug_map == 2) { drawTexture2D(0, 0, xres, yres, texture_manager.getByName("rt_normals")); }
	if (debug_map == 3) { drawTexture2D(0, 0, xres, yres, texture_manager.getByName("rt_specular")); }
	if (debug_map == 4) { drawTexture2D(0, 0, xres, yres, texture_manager.getByName("rt_gloss")); }
	if (debug_map == 5) { drawTexture2D(0, 0, xres, yres, texture_manager.getByName("rt_lights")); }
	if (debug_map == 6) { drawTexture2D(0, 0, xres, yres, texture_manager.getByName("rt_depth")); }
//#endif

	/*render_techniques_manager.getByName("basic")->activate();
	activateWorldMatrix(0);
	activateCamera(camera, 1);*/

	setWorldMatrix(XMMatrixIdentity());
	render_techniques_manager.getByName("basic")->activate();
	activateCamera(camera, 1);
	activateWorldMatrix(0);

	activateZConfig(ZConfig::ZCFG_DEFAULT);

	//render_manager.renderAll((TCompCamera*)activeCamera, ((TCompTransform*)((CEntity*)activeCamera.getOwner())->get<TCompTransform>()));
	

	
	

#ifdef _DEBUG
	renderDebugEntities();

	if (renderWireframe) {
		debugTech->activate();
		renderWireframeCurrent = true;
		render_manager.renderAll(&camera, true, false);
		renderWireframeCurrent = false;
	}

	std::string mode = "Render output";

	if (debug_map == 1) { mode = "Albedo"; }
	if (debug_map == 2) { mode = "Normal"; }
	if (debug_map == 3) { mode = "Specular"; }
	if (debug_map == 4) { mode = "Gloss"; }
	if (debug_map == 5) { mode = "Lights"; }
	if (debug_map == 6) { mode = "Depth"; }

	font.print(xres / 2.0f - 30, 10.0f, mode.c_str());

	CTraceScoped t0("AntTweak");
	TwDraw();
	t0.~CTraceScoped();
#endif

	/*TCompName* zone_name = logic_manager.getPlayerZoneName();
	if (zone_name) {
		XMVECTOR size = font.measureString(zone_name->name);
		font.print(xres*0.5f - XMVectorGetZ(size) * 0.5f, 40, zone_name->name);
	}*/

#ifndef FINAL_RELEASE
	std::string s_fps = "FPS: " + std::to_string(fps);
	font.print(xres - 200, 50, s_fps.c_str());
#endif

	// DRAW GUI
	CTraceScoped scope_gui("GUI");
	if (draw_gui) {
		if (h_player.isValid()) {
			int life_val = (int)((TCompLife*)((CEntity*)h_player)->get<TCompLife>())->life;
			life_val /= 10;
			int leng = 50;
			int spearation = 12;
			activateBlendConfig(BLEND_CFG_BY_SRC_ALPHA);
			activateZConfig(ZConfig::ZCFG_DISABLE_ALL);
			int life_count = 0;
			int counter = 0;
			// Life val is over 10
			while (life_count < life_val) {
				// for each 2 of live, draw a full heart, if only one remains, draw a half heart
				if (life_val - life_count == 1)
					drawTexture2D(35 + (leng + spearation)* counter, 30, leng, leng, texture_manager.getByName("life_half"));
				else
					drawTexture2D(35 + (leng + spearation)* counter, 30, leng, leng, texture_manager.getByName("life_full"));
				life_count += 2;
				counter++;
			}

			bool can_throw = ((TCompPlayerController*)((CEntity*)h_player)->get<TCompPlayerController>())->canThrow();
			if (can_throw) {
				if (!CIOStatus::get().isPressed(CIOStatus::MOUSE_LEFT)) {
					drawTexture2D(xres / 2.f - 24, yres / 2.f - 24, 48, 48, texture_manager.getByName("crosshair_can"));
				}
				else {
					drawTexture2D(xres / 2.f - 28, yres / 2.f - 28, 56, 56, texture_manager.getByName("crosshair_can"));
				}
			}
			else {
				drawTexture2D(xres / 2.f - 16, yres / 2.f - 16, 32, 32, texture_manager.getByName("crosshair_cant"));
			}


			activateZConfig(ZConfig::ZCFG_DEFAULT);
			activateBlendConfig(BLEND_CFG_DEFAULT);

			/*
			activateBlendConfig(BLEND_CFG_COMBINATIVE_BY_SRC_ALPHA);
			drawDialogBox3D(camera, ((TCompSkeleton*)((CEntity*)h_player)->get<TCompSkeleton>())->getPositionOfBone(89), 100, 60, texture_manager.getByName("gui_test1"), "gui_dialog_box");
			float x, y;
			if (camera.getScreenCoords(((TCompSkeleton*)((CEntity*)h_player)->get<TCompSkeleton>())->getPositionOfBone(89), &x, &y))
			font.print(x + 20, y + 20, "Aguja");

			drawDialogBox3D(camera, XMVectorSet(-9.98f, 1.14f, 2.96f, 0), 150, 60, texture_manager.getByName("gui_test1"), "gui_dialog_box");
			if (camera.getScreenCoords(XMVectorSet(-9.98f, 1.14f, 2.96f, 0), &x, &y))
			font.print(x + 20, y + 20, "Partículas");
			activateBlendConfig(BLEND_CFG_DEFAULT);*/

			//const CTexture *cross = texture_manager.getByName("crosshair_can");

			//activateBlendConfig(BLEND_CFG_COMBINATIVE_BY_SRC_ALPHA);
			//drawDialogBox3DDynamic(camera, XMVectorSet(3, 3, 0, 0), 3000, 1500, texture_manager.getByName("gui_test1"), "gui_dialog_box");
			//drawDialogBox3D(camera, XMVectorSet(0, 3, 0, 0), 300, 150, texture_manager.getByName("gui_test1"), "gui_dialog_box");
			//drawTexture3DDynamic(camera, XMVectorSet(0, 3, 0, 0), 200, 80, texture_manager.getByName("smoke"));
			//drawTexture3D(camera, XMVectorSet(3, 3, 0, 0), 200, 80, texture_manager.getByName("smoke"));
			//activateBlendConfig(BLEND_CFG_DEFAULT);
		}			
	}	

	logic_manager.draw();

#ifndef FINAL_RELEASE
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
	SIZE_T virtualMemUsedByMe = pmc.WorkingSetSize;
	float mem = (virtualMemUsedByMe / 1024.f) / 1024.f;

	bool alt_pressed = CIOStatus::get().isPressed(CIOStatus::ALT);

	frames_d.front().clamp_physx = physx_clamp;
	frames_d.front().rope_thrown = rope_thrown;
	frames_d.front().ram = mem / 10.f; // 10 MB = 1 pixel
	int x = 0;
	float h = 0.f;
	for (auto frame : frames_d) {
		ctes_global.get()->frame_list[x] = XMVectorSet(alt_pressed ? frame.ram : frame.fps, (frame.rope_thrown ? 1.f : 0.f), (frame.clamp_physx ? 1.f : 0.f), 0);
		h = fps;
		//drawTexture2D(x, yres - h, 1, h, texture_manager.getByName("whitepx"));
		x++;
	}

	rope_thrown = false;
	physx_clamp = false;

	
	font.printf(500, 60, "Memory use: %f MB", mem);
#endif

	::render.swap_chain->Present(0, 0);
}

void CApp::renderEntities() {
	
	CTraceScoped t0("Render entities");
	CCamera camera = *(TCompCamera*)render_manager.activeCamera;

	debugTech->activate();
	const CTexture* rope_tex = texture_manager.getByName("grass");
	rope_tex->activate(0);

	//ctes_global.activateInVS(2);

	activateWorldMatrix(0);
	activateTint(0);

	// Render entities
	int rope_count = 0;
	
	for (int i = 0; i < entity_manager.getEntities().size(); ++i)
	{
		TCompTransform* t = ((CEntity*)entity_manager.getEntities()[i])->get<TCompTransform>();
		TCompMesh* mesh = ((CEntity*)entity_manager.getEntities()[i])->get<TCompMesh>();

		TCompDistanceJoint* djoint = ((CEntity*)entity_manager.getEntities()[i])->get<TCompDistanceJoint>();
		TCompRope* c_rope = ((CEntity*)entity_manager.getEntities()[i])->get<TCompRope>();
		TCompName* name = ((CEntity*)entity_manager.getEntities()[i])->get<TCompName>();


		// Draw the joints

		if (c_rope) {
			rope_count++;
			rope_tex->activate(0);
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
			tension = maxDist < 0.2f ? 0.f : 1.f;

			/*CMesh* m = &rope;
			if (rope_count == 1)
				m = &rope2;
			if (rope_count == 2)
				m = &rope3;
			if (rope_count == 3)
				m = &rope4;*/
			

			createFullString(rope, initialPos, finalPos, tension, c_rope->width);

			ropeTech->activate();

			bool tensed = c_rope->tensed;// dist > maxDist * 0.001f;
			if (tensed)
				setTint(XMVectorSet(0.46f, 0.075f, 0.075f, 1));
			else
				setTint(XMVectorSet(0.45f, 0.8f, 0.63f, 1));

			/*font.printf(60, 150 + 30 * rope_count, "Rope %u: tensed: %u, initPos: (%f, %f, %f), finalPos: (%f, %f, %f)", rope_count, (tensed ? 1 : 0),
				XMVectorGetX(initialPos),
				XMVectorGetY(initialPos),
				XMVectorGetZ(initialPos),
				XMVectorGetX(finalPos),
				XMVectorGetY(finalPos),
				XMVectorGetZ(finalPos)
				);*/

			/*float color_tension = min(dist / maxDist * 0.25f, 1);
			setTint(XMVectorSet(color_tension * 3, (1 - color_tension) * 3, 0, 1));*/
			setWorldMatrix(XMMatrixIdentity());
			rope.activateAndRender();

			/*setWorldMatrix(XMMatrixAffineTransformation(XMVectorSet(0.1f, 0.1f, 0.1f, 0.1f), XMVectorZero(), rot1, initialPos));
			wiredCube.activateAndRender();

			setWorldMatrix(XMMatrixAffineTransformation(XMVectorSet(0.1f, 0.1f, 0.1f, 0.1f), XMVectorZero(), rot2, finalPos));
			wiredCube.activateAndRender();*/
			debugTech->activate();
		}

		// If the component has no transform it can't be rendered
		/*if (!t)
			continue;*/

		/*if (mesh)
		setTint(mesh->color);*/

		/*setWorldMatrix(t->getWorld());

		if (mesh && mesh->active)
		mesh->mesh->activateAndRender();*/

		// Draw texts
		TCompDistanceText* c_text = ((CEntity*)entity_manager.getEntities()[i])->get<TCompDistanceText>();
		if (c_text && t) {
			if (c_text->in_range) {
				float old_size = font.size;
				font.size = c_text->size;
				unsigned int old_col = font.color;
				font.color = c_text->color;

				XMVECTOR edges = font.measureString(c_text->text);
				float x, y;
				float offset = 30;
				if (camera.getScreenCoords(t->position, &x, &y)) {
					activateBlendConfig(BLEND_CFG_COMBINATIVE_BY_SRC_ALPHA);
					drawDialogBox(x, y, XMVectorGetZ(edges) + offset * 2, XMVectorGetW(edges) + offset * 2, texture_manager.getByName("gui_test1"), "gui_dialog_box");
					activateBlendConfig(BLEND_CFG_DEFAULT);

					//font.print3D(t->position, c_text->text);
					font.print(x + offset, y + offset, c_text->text);
					font.size = old_size;
					font.color = old_col;
				}
			}
		}
	}
	
	/*renderNavMesh = true;
	if (renderNavMesh){
		CNav_mesh_manager::get().render_nav_mesh();
		CNav_mesh_manager::get().pathRender();
	}*/
	/*getObjManager<TCompBtSoldier>()->renderDebug3D();
	getObjManager<TCompBtGrandma>()->renderDebug3D();
	getObjManager<TCompPlayerController>()->renderDebug3D();*/
	
}

void CApp::renderDebugEntities() {
	
	std::string s_fps = "FPS: " + std::to_string(fps);
	font.print(500, 30, s_fps.c_str());

	getObjManager<TCompSkeleton>()->renderDebug3D();
	getObjManager<TCompTrigger>()->renderDebug3D();
	getObjManager<TCompBtGrandma>()->renderDebug3D();
	getObjManager<TCompBtSoldier>()->renderDebug3D();
	getObjManager<TCompParticleGroup>()->renderDebug3D();

	//--------- NavMesh render Prueba --------------
	/*if (CIOStatus::get().isPressed(CIOStatus::EXIT)){
		CNav_mesh_manager::get().keep_updating_navmesh = false;
		exit(-1);
	}*/
	renderNavMesh = true;
	if (renderNavMesh){
		CNav_mesh_manager::get().render_nav_mesh();
		CNav_mesh_manager::get().pathRender();
	}
	//----------------------------------------------

	//CNav_mesh_manager::get().pathRender();

	debugTech->activate();
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
		TCompGNLogic* golden_needle = e->get<TCompGNLogic>();
		TCompGNItem* golden_needle_item = e->get<TCompGNItem>();

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
#ifdef _DEBUG
	TwTerminate();
#endif
	deferred.destroy();

	sharpen.destroy();
	ssao.destroy();
	chromatic_aberration.destroy();
	blur.destroy();
	blur_camera.destroy();
	glow.destroy();
	silouette.destroy();
	lens_flare.destroy();
	underwater.destroy();
	ssrr.destroy();
	fog.destroy();
	//rt_base->destroyl();

	CRope_manager::get().clearStrings();
	Citem_manager::get().clear();
	aimanager::get().clear();
	entity_manager.clear();
	render_manager.destroyAllKeys();
	render_manager.clearOcclusionPlanes();
	mesh_manager.destroyAll();
	texture_manager.destroyAll();
	material_manager.destroyAll();
	skeleton_manager.destroyAll();
	render_techniques_manager.destroyAll();
	renderUtilsDestroy();
	ctes_global.destroy();
	particle_groups_manager.destroy();
	logic_manager.destroy();
	CNav_mesh_manager::get().keep_updating_navmesh = false;
	CNav_mesh_manager::get().setNeedNavMesh(false);
	CNav_mesh_manager::get().generate_nav_mesh = false;

	// Destroy app resources
	wiredCube.destroy();
	intersectsWiredCube.destroy();
	rope.destroy();

	font.destroy();
	//rt_base->destroy();
	// Video
	if (mgr != nullptr) {
		delete mgr;
	}
	::render.destroyDevice();
	
	// Destro navmesh
	CNav_mesh_manager().get().destroy();
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
#ifndef _DEBUG
	while (ShowCursor(FALSE) >= 0);
#endif
	// Load picture
	/*renderUtilsDestroy();
	renderUtilsCreate();*/
	float ClearColor[4] = { 0.1f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
	::render.ctx->ClearRenderTargetView(::render.render_target_view, ClearColor);
	::render.ctx->ClearDepthStencilView(::render.depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);
	activateTextureSamplers();
	activateRSConfig(RSCFG_DEFAULT);
	activateZConfig(ZCFG_DISABLE_ALL);

	float load_h = yres;
	float load_w = (1920.0f * load_h) / 1080.f;
	float init_offset = (xres - load_w) * 0.5f;
	drawTexture2D(init_offset, 0, load_w, load_h, texture_manager.getByName("loading_screen"));
	::render.swap_chain->Present(0, 0);

	bool is_ok = true;
	pause = false;
	CNav_mesh_manager::get().setNeedNavMesh(false);

	load_timer.reset();
	aux_timer.reset();
	load_mesh_time = 0;
	load_text_time = 0;
	load_skel_time = 0;
	load_ragdoll_time = 0;

	Citem_manager::get().clear();
	CImporterParser p;
	aimanager::get().clear();
	CNav_mesh_manager::get().keep_updating_navmesh = false;
	while (CNav_mesh_manager::get().getLock()){
		std::string prueba = "1";
	}

	CNav_mesh_manager::get().clearNavMesh();
	

	CRope_manager::get().clearStrings();
	entity_manager.clear();

	// No destruir recursos en el cambio de escena
	//mesh_manager.destroyAll();
	//texture_manager.destroyAllTextures();
	//render_techniques_manager.destroyAll();
	//material_manager.destroyAll();
	//skeleton_manager.destroyAll();

	render_manager.destroyAllKeys();
	render_manager.clearOcclusionPlanes();
	ragdoll_manager.destroyAll();
	entity_lister.resetEventCount();
	logic_manager.clearAnimations();
	physics_manager.loadCollisions();
	CSoundManager::get().clear();

	dbg("Init loads: %g\n", aux_timer.seconds());
	
	current_scene_name = split_string(split_string(scene_name, "/").back(), ".").front();
	XASSERT(p.xmlParseFile(scene_name), "Error loading the scene: %s", scene_name.c_str());

	dbg("Total texture load time: %g\n", load_text_time);
	dbg("Total mesh load time: %g\n", load_mesh_time);
	dbg("Total skeleton load time: %g\n", load_skel_time);
	dbg("Total ragdoll load time: %g\n", load_ragdoll_time);

	aux_timer.reset();
	particle_groups_manager.destroy();
	XASSERT(particle_groups_manager.xmlParseFile("data/particles/particle_groups.xml"), "Error loading the particle groups");
	dbg("Total particle load time: %g\n", aux_timer.seconds());
	aux_timer.reset();

	// public delta time inicialization
	delta_time = 0.f;
	total_time = delta_time;
	fixedUpdateCounter = 0.0f;

	renderAABB = false;
	renderAxis = false;
	renderGrid = false;
	renderNames = false;
	debug_mode = false;
	renderWireframe = false;
	renderWireframeCurrent = false;
	debug_map = 0;

	//physics_manager.init();
	logic_manager.init();

	if (scene_name == "data/scenes/my_file.xml"){
		CNav_mesh_manager::get().setNavMeshClimb(1);
	}
	else if (scene_name == "data/scenes/scene_3_old.xml"){
		CNav_mesh_manager::get().setNavMeshClimb(0);
	}
	// Navmesh initialization
	//Check the scene and change the climb atributte from the navmesh 

	bool valid = CNav_mesh_manager::get().build_nav_mesh();

	CEntity* e = entity_manager.getByName("PlayerCamera");	
	//XASSERT(CHandle(e).isValid(), "Camera not valid");
	if (CHandle(e).isValid()) {
		render_manager.activeCamera = e->get<TCompCamera>();
	}
	
	e = entity_manager.getByName("camera_menu");
	if (CHandle(e).isValid()) {
		render_manager.activeCamera = e->get<TCompCamera>();
	}

	//XASSERT(render_manager.activeCamera.isValid(), "Camera not valid");
	font.camera = render_manager.activeCamera;

	// Ctes ---------------------------

	XASSERT(is_ok, "Error creating deferred targets");

	//ctes_global.world_time = XMVectorSet(0, 0, 0, 0);
	
	ctes_global.get()->added_ambient_color = XMVectorSet(1, 1, 1, 1);
	ctes_global.get()->vignette = 4.0f;
	if (current_scene_name == "scene_menu") {
		ctes_global.get()->added_ambient_color = XMVectorSet(0.7f, 0.5f, 0.5f, 1);
		ctes_global.get()->vignette = 7.0f;
	}
	ctes_global.get()->world_time = 0.f; // XMVectorSet(0, 0, 0, 0);
	

	XASSERT(is_ok, "Error creating global constants");

	XASSERT(is_ok, "Error creating debug meshes");

#ifdef _DEBUG	
	entity_inspector.inspectEntity(CHandle());
#endif

	activateInspectorMode(false);

	initManagers();

	logic_manager.onSceneLoad(current_scene_name);

	//Borrado de mapa de colisiones una vez cargado en sus respectivos colliders
	CPhysicsManager::get().m_collision->clear();

	h_player = entity_manager.getByName("Player");

	//texture_manager.getByName("room_env_test")->activate(8);
	//texture_manager.getByName("sunsetcube1024")->activate(8);
	texture_manager.getByName("storm")->activate(8);

	water_level = -100000;
	CEntity* water = entity_manager.getByName("water");
	if (water) {
		TCompTransform* water_t = water->get<TCompTransform>();
		water_level = XMVectorGetY(water_t->position);
	}

	ctes_global.get()->global_water_level = water_level;

	render_manager.init();
	ctes_global.get()->use_lightmaps = 1;
	ctes_global.get()->exposure = 2.5f;

	//TO DO: Quitar carga de ambientes por nombre de escena y meterlo en exportador
	if (scene_name == "data/scenes/scene_menu.xml"){
		TCompCamera*  cam = (TCompCamera*)render_manager.activeCamera;
		cam->changeZFar(70.f);
		ctes_global.get()->use_lightmaps = 0;
		CSoundManager::get().setSceneID(-1);

		fog.fog_level = -1000;
		fog.fog_distance = 1000;
		fog.fog_color = XMVectorSet(0, 0, 0, 0);
	}
	else if (scene_name == "data/scenes/scene_1.xml"){
		TCompCamera*  cam=(TCompCamera*)render_manager.activeCamera;
		cam->changeZFar(70.f);
		ctes_global.get()->use_lightmaps = 1;
		CSoundManager::get().setSceneID(1);

		fog.fog_level = -1000;
		fog.fog_distance = 50;
		fog.fog_color = XMVectorSet(0.3f, 0.3f, 0.3f, 0.3f);

		CSoundManager::get().playEvent("MUSIC_BASEMENT");
	}
	else if (scene_name == "data/scenes/scene_2.xml"){
		TCompCamera*  cam = (TCompCamera*)render_manager.activeCamera;
		cam->changeZFar(90.f);
		ctes_global.get()->use_lightmaps = 0;		
		CSoundManager::get().setSceneID(2);

		fog.fog_level = -1000;
		fog.fog_distance = 1000;

		CSoundManager::get().playEvent("MUSIC_ESC");
	}
	else if (scene_name == "data/scenes/scene_3.xml"){
		TCompCamera*  cam = (TCompCamera*)render_manager.activeCamera;
		cam->changeZFar(100.f);
		ctes_global.get()->use_lightmaps = 1;
		ctes_global.get()->exposure = 1.5f;
		CSoundManager::get().setSceneID(3);

		fog.fog_level = -1000;
		fog.fog_distance = 1000;

		fog.fog_distance = 50;
		fog.fog_color = XMVectorSet(0.3f, 0.45f, 0.3f, 0.3f);

		CSoundManager::get().playEvent("MUSIC_PIT");
	}
	else if (scene_name == "data/scenes/scene_4.xml"){
		TCompCamera*  cam = (TCompCamera*)render_manager.activeCamera;
		cam->changeZFar(90.f);
		ctes_global.get()->use_lightmaps = 1;
		CSoundManager::get().setSceneID(4);

		fog.fog_level = -1000;
		fog.fog_distance = 1000;
	}
	else if (scene_name == "data/scenes/scene_final_boss.xml"){
		TCompCamera*  cam = (TCompCamera*)render_manager.activeCamera;
		cam->changeZFar(250.0f);
		ctes_global.get()->use_lightmaps = 0;
		CSoundManager::get().setSceneID(5);

		fog.fog_level = -1000;
		fog.fog_distance = 1000;
		fog.fog_color = XMVectorSet(0, 0, 0, 0);
	}

	ctes_global.uploadToGPU();

	dbg("Misc loads: %g\n", aux_timer.seconds());
	dbg("Total load time: %g\n", load_timer.seconds());

	
}

void CApp::loadPrefab(std::string prefab_name) {
	CEntity* e = prefabs_manager.getInstanceByName(prefab_name.c_str());
}

void CApp::slowMotion(float time) {
	time_modifier = 0.05f;
	slow_motion_counter = time;
	CSoundManager::get().activateSlowMo();
}

void CApp::loadVideo(const char* name)
{
#ifndef _DEBUG
	if (mgr == nullptr) {
		mgr = new TheoraVideoManager();
	}
	char full_name[MAX_PATH];
	::sprintf(full_name, "data/videos/%s", name);
	clip = mgr->createVideoClip(full_name, TheoraOutputMode::TH_RGBX, 16);
	
	int w = clip->getWidth();
	int h = clip->getHeight();

	D3D11_TEXTURE2D_DESC desc;
	// Create tex2D
	::ZeroMemory(&desc, sizeof(desc));
	desc.Width = w;
	desc.Height = h;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.MipLevels = 1;

	endframe = 0;

	HRESULT hr = ::render.device->CreateTexture2D(&desc, NULL, &tex);
	if (FAILED(hr)) {
		dbg("FAILED CreateTexture2D\n");
	}
	videoTexture = new CTexture();
	endframe = int(clip->getDuration() * clip->getFPS());
	::render.device->CreateShaderResourceView(tex, 0, &m_shaderResourceView);
	setDbgName(tex, "Video texture");
	setDbgName(m_shaderResourceView, "Video resource view");
	clip->play();
#endif
}

bool CApp::renderVideo()
{
	::render.activateBackbuffer();
	TheoraVideoFrame *frame = clip->getNextFrame();
	
	UINT w = clip->getWidth();
	UINT h = clip->getHeight();

	const float RED[4] = { 0.f, 0.f, 0.f, 1.0 };
	::render.ctx->ClearRenderTargetView(::render.render_target_view, RED);
	::render.ctx->ClearDepthStencilView(::render.depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);

	if (frame){
		D3D11_MAPPED_SUBRESOURCE resource;
		HRESULT hr = ::render.ctx->Map(tex, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		if (FAILED(hr)) {
			return false;
		}
		// Copy full memory block
		unsigned char* buffer = frame->getBuffer();
		BYTE* mappedData = reinterpret_cast<BYTE*>(resource.pData);
		for (UINT i = 0; i < h; ++i)
		{
			memcpy(mappedData, buffer, w * 4);
			mappedData += resource.RowPitch;
			buffer += w * 4;
		}
		// Unlock resource
		::render.ctx->Unmap(tex, 0);
		clip->popFrame();
		if (frame->getFrameNumber() >= (endframe - 1)){
			clip->stop();
			mgr->destroyVideoClip(clip);
			videoTexture->destroy();
			delete videoTexture;
			return false;
		}
	}

	
	videoTexture->setResource(tex);
	videoTexture->setResourceView(m_shaderResourceView);

	// Bandas negras en los lados
	float load_w = xres;
	float load_h = (1080.0f * load_w) / 1920.f;
	float video_h_offset = (yres - load_h) * 0.5f;

	drawTexture2D(0, video_h_offset, load_w, load_h, videoTexture);

	//dbg("elaps: %f\n", elaps);
	mgr->update(delta_time);

	return true;
}

unsigned int CApp::getMaxNumNeedles(){
	return max_num_needles;
}

void CApp::playFinalVideo() {
#ifdef NO_VIDEO
	CLogicManager::get().loadScene(menu_scene);
	game_state = TGameState::MAIN_MENU;
#else
	CLogicManager::get().loadScene("data/scenes/empty_scene.xml");
	loadVideo("final_BP.ogv");
	video_sound_played = false;
	game_state = TGameState::FINAL_VIDEO;
#endif
}

void CApp::playInitialVideo() {
#ifdef NO_VIDEO
	CLogicManager::get().loadScene("data/scenes/scene_1.xml");
	game_state = TGameState::GAMEPLAY;
#else
	CLogicManager::get().loadScene("data/scenes/empty_scene.xml");
	loadVideo("intro_BP.ogv");
	video_sound_played = false;
	game_state = TGameState::INITIAL_VIDEO;
#endif
}

void CApp::exitApp() {
	destroy();
	exit(0);
}

void CApp::setFogAttributes(float level, float distance) {
	fog.fog_distance = distance;
	fog.fog_level = level;
}