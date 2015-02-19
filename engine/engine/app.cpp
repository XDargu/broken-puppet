#include "mcv_platform.h"
#include "app.h"
#include "camera.h"
#include "render/render_utils.h"
#include "entity_manager.h"
#include "doom_controller.h"
#include "font/font.h"
#include "render/texture.h"
#include "importer_parser.h"
#include "physics_manager.h"
#include "handle\handle.h"

using namespace DirectX;
#include "render/ctes/shader_ctes.h"

#include <PxPhysicsAPI.h>
#include <foundation\PxFoundation.h>

using namespace physx;

#include <AntTweakBar.h>
#include "entity_inspector.h"

static CApp the_app;

CEntityManager &entity_manager = CEntityManager::get();
CPhysicsManager &physics_manager = CPhysicsManager::get();

#include "ai\ai_basic_patroller.h"
#include "io\iostatus.h"

ai_basic_patroller aibp;

CApp& CApp::get() {
  return the_app;
}

CApp::CApp()
  : xres(640)
  , yres(480)
{ }

void CApp::loadConfig() {
  // Parse xml file...
  xres = 1024;
  yres = 640;
}

CVertexShader vs_basic;
CVertexShader vs_basic2;
CPixelShader ps_basic;
CPixelShader ps_textured;
CMesh        grid;
CMesh        axis;
CMesh		 wiredCube;
CMesh		 intersectsWiredCube;

TCamera*      camera;
CCamera*	  oldCamera;
CFont         font;

CShaderCte<TCtesGlobal> ctes_global;

float fixedUpdateCounter;

void registerAllComponentMsgs() {
	SUBSCRIBE(TLife, TMsgExplosion, onExplosion);
	SUBSCRIBE(TLife, TMsgDied, onDied);
}

void createManagers() {
	getObjManager<CEntity>()->init(1024);
	getObjManager<TTransform>()->init(1024);
	getObjManager<TController>()->init(32);
	getObjManager<TLife>()->init(32);
	getObjManager<TCompName>()->init(1024);
	getObjManager<TMesh>()->init(1024);
	getObjManager<TCamera>()->init(4);
	getObjManager<TCollider>()->init(512);
	getObjManager<TRigidBody>()->init(512);
	getObjManager<TStaticBody>()->init(512);
	getObjManager<TAABB>()->init(1024);
	getObjManager<TPlayerController>()->init(1);
	getObjManager<TThirdPersonCameraController>()->init(1);
	getObjManager<TCameraPivotController>()->init(1);
	getObjManager<TPlayerPivotController>()->init(1);
	getObjManager<TEnemyWithPhysics>()->init(64);

	registerAllComponentMsgs();
}

void initManagers() {
	getObjManager<TCamera>()->initHandlers();
	getObjManager<TCollider>()->initHandlers();
	getObjManager<TRigidBody>()->initHandlers();
	getObjManager<TStaticBody>()->initHandlers();
	getObjManager<TAABB>()->initHandlers();
	getObjManager<TPlayerController>()->initHandlers();
	getObjManager<TPlayerPivotController>()->initHandlers();
	getObjManager<TCameraPivotController>()->initHandlers();	
	getObjManager<TThirdPersonCameraController>()->initHandlers();
	getObjManager<TEnemyWithPhysics>()->initHandlers();	
}

bool CApp::create() {

  if (!::render.createDevice())
    return false;

  renderAABB = false;
  renderAxis = false;
  renderGrid = false;
  renderNames = false;

  createManagers();

  physics_manager.init();

  CImporterParser p;
  p.xmlParseFile("my_file.xml");

  initManagers();
  fixedUpdateCounter = 0.0f;

  bool is_ok = vs_basic.compile("Tutorial04.fx", "VS", vdcl_position_color)
    && ps_basic.compile("Tutorial04.fx", "PS")
    && vs_basic2.compile("Tutorial04.fx", "VSNormal", vdcl_position_uv_normal)
    && ps_textured.compile("Tutorial04.fx", "PSTextured")
  ;
  assert(is_ok);

  CEntity* e = entity_manager.getByName("Camera");
  camera = e->get<TCamera>();

  is_ok = font.create();
  font.camera = camera;
  assert(is_ok);

  // Ctes ---------------------------
  is_ok &= renderUtilsCreate();

  // Initialize the camera
  camera->setViewport(0.f, 0.f, (float)xres, (float)yres);

  //ctes_global.world_time = XMVectorSet(0, 0, 0, 0);
  ctes_global.get()->world_time = 0.f; // XMVectorSet(0, 0, 0, 0);
  is_ok &= ctes_global.create();
  assert(is_ok);

  // Create debug meshes
  is_ok &= createGrid(grid, 10);
  is_ok &= createAxis(axis);
  is_ok &= createUnitWiredCube(wiredCube, XMFLOAT4(1.f, 1.f, 1.f, 1.f));
  is_ok &= createUnitWiredCube(intersectsWiredCube, XMFLOAT4(1.f, 0.f, 0.f, 1.f));

  assert(is_ok);

  // Init AntTweakBar
  TwInit(TW_DIRECT3D11, ::render.device);
  TwWindowSize(xres, yres);

  entity_inspector.init();
  entity_inspector.inspectEntity(entity_manager.getByName("Player"));  

  entity_lister.init();
  entity_actioner.init();
  debug_optioner.init();

  CEntity* e2 = CHandle::create< CEntity >();
  TLife *life = e2->add(CHandle::create<TLife>());
  life->life = 20.f;
  TCompName* cname = e2->add(CHandle::create<TCompName>());
  strcpy(cname->name, "pep");

  CHandle h2(e2);
  CHandle h3 = h2.clone();
  CEntity* e3 = h3;
  TLife *life3 = e3->get<TLife>();


  TMsgExplosion msg1;
  msg1.damage = 3.3f;
  e2->sendMsg(msg1);
  e2->sendMsg(TMsgDied(2));

  // Enemigo SIN componentes
  aibp.entity = old_entity_manager.create("Enemy");
  aibp.entity->setPosition(((TTransform*)((CEntity*)entity_manager.getByName("Enemigo"))->get<TTransform>())->position);
  CEntityOld* wp1 = old_entity_manager.create("EnemyWp1");
  wp1->setPosition(XMVectorSet(10, 0, 10, 0));
  CEntityOld* wp2 = old_entity_manager.create("EnemyWp2");
  wp2->setPosition(XMVectorSet(-10, 0, 10, 0));
  CEntityOld* wp3 = old_entity_manager.create("EnemyWp3");
  wp3->setPosition(XMVectorSet(10, 0, -10, 0));

  vector<CEntityOld*> waypoints;
  waypoints.push_back(wp1);
  waypoints.push_back(wp2);
  waypoints.push_back(wp3);

  aibp.waypoints = waypoints;
  aibp.Init();

  return true;
}

void moveCameraOnEntity(CCamera& camera, CEntityOld *e) {
  camera.lookAt(e->getPosition(), e->getPosition() + e->getFront(), e->getUp());
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
  float delta_secs = delta_ticks.QuadPart * ( 1.0f / freq.LowPart );
  
  float fps = 1.0f / delta_secs;

  before = now;

  // To avoid the fist huge delta time
  if (delta_secs < 0.5) {
	  update(delta_secs);
	  // Fixed update
	  fixedUpdateCounter += delta_secs;
	  if (fixedUpdateCounter >= physics_manager.timeStep) {
		  fixedUpdate(fixedUpdateCounter);
		  fixedUpdateCounter = 0.0f;
	  }
  }
  render();
}

void CApp::update(float elapsed) {

  CIOStatus& io = CIOStatus::get();
  // Update input
  io.update(elapsed);

  if (isKeyPressed('I')) {
	  io.setMousePointer(true);
  }
	
  if (isKeyPressed('O')) {
	  io.setMousePointer(false);
  }
  // Update ---------------------
  //  ctes_global.world_time += XMVectorSet(elapsed,0,0,0);
  ctes_global.get()->world_time += elapsed;

  aibp.Recalc(elapsed);

  getObjManager<TPlayerController>()->update(elapsed); // Update player transform
  getObjManager<TPlayerPivotController>()->update(elapsed);
  getObjManager<TCameraPivotController>()->update(elapsed);
  getObjManager<TThirdPersonCameraController>()->update(elapsed); // Then update camera transform, wich is relative to the player
  getObjManager<TCamera>()->update(elapsed);  // Then, update camera view and projection matrix
  getObjManager<TAABB>()->update(elapsed); // Update objects AABBs

  entity_inspector.update();
  entity_lister.update();
  entity_actioner.update();
  
  ((TTransform*)((CEntity*)entity_manager.getByName("Enemigo"))->get<TTransform>())->position = aibp.entity->getPosition();
  ((TTransform*)((CEntity*)entity_manager.getByName("Enemigo"))->get<TTransform>())->rotation = aibp.entity->getRotation();
}

// Physics update
void CApp::fixedUpdate(float elapsed) {
  physics_manager.gScene->simulate(physics_manager.timeStep);
  physics_manager.gScene->fetchResults(true);

  getObjManager<TPlayerController>()->fixedUpdate(elapsed); // Update kinematic player
  getObjManager<TRigidBody>()->fixedUpdate(elapsed); // Update rigidBodies of the scene
  getObjManager<TEnemyWithPhysics>()->fixedUpdate(elapsed);
}

void CApp::render() {

  // Render ---------------------
  float ClearColor[4] = { 0.1f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
  ::render.ctx->ClearRenderTargetView(::render.render_target_view, ClearColor);
  ::render.ctx->ClearDepthStencilView(::render.depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);

  activateTextureSamplers();

  //vs_basic.activate();
  //ps_basic.activate();  
  vs_basic2.activate();
  ps_textured.activate();
  const CTexture *t = texture_manager.getByName("wood_d");
  t->activate(0);
  ctes_global.get()->lightDirection = XMVectorSet(0, 1, 1, 0);
  ctes_global.uploadToGPU();
  ctes_global.activateInPS(2);

  //ctes_global.activateInVS(2);

  activateWorldMatrix(0);
  activateTint(0);
  
  //activateCamera(*camera, 1);
  // TODO: Make activate TCamera
  activateCamera(camera->view_projection, 1);

  /*drawViewVolume(camera2);
  setWorldMatrix(XMMatrixIdentity());

  static int nframe = 0;
  font.printf(10, 10, "Yaw Is %f", rad2deg( getYawFromVector( e1->getFront())));

  ctes_global.uploadToGPU();

  // Tech con textura
  vs_basic2.activate();
  ps_textured.activate();
  const CTexture *t = texture_manager.getByName("wood_d");
  t->activate(0);
  const CMesh* teapot = mesh_manager.getByName("Box001");
  ctes_global.activateInVS(2);
  teapot->activateAndRender();*/

  renderEntities();
  vs_basic.activate();
  ps_basic.activate();
  renderDebugEntities();
  //renderEntityDebugList();

  TwDraw();

  ::render.swap_chain->Present(0, 0);

}

void CApp::renderEntities() {
  
  // Render entities
  for (int i = 0; i < entity_manager.getEntities().size(); ++i)
  {
	  TTransform* t = ((CEntity*)entity_manager.getEntities()[i])->get<TTransform>();
	  TMesh* mesh = ((CEntity*)entity_manager.getEntities()[i])->get<TMesh>();

	  // If the component has no transform it can't be rendered
	  if (!t)
		  continue;

	  if (mesh)
		setTint(mesh->color);

	  setWorldMatrix(t->getWorld());

	  if (mesh)
		mesh->mesh->activateAndRender();
  }
}

void CApp::renderDebugEntities() {

	setWorldMatrix(XMMatrixIdentity());
	if (renderGrid)
		grid.activateAndRender();
	if (renderAxis)
		axis.activateAndRender();

	// Render entities
	for (int i = 0; i < entity_manager.getEntities().size(); ++i)
	{
		CEntity* e = (CEntity*)entity_manager.getEntities()[i];
		TTransform* t = e->get<TTransform>();
		TCompName* name = e->get<TCompName>();
		TAABB* aabb = e->get<TAABB>();

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
				TAABB* aabb2 = e2->get<TAABB>();
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

void CApp::destroy() {
  TwTerminate();
  mesh_manager.destroyAll();
  texture_manager.destroyAll();
  axis.destroy();
  grid.destroy();
  renderUtilsDestroy();
  vs_basic.destroy();
  vs_basic2.destroy();
  ps_basic.destroy();
  ps_textured.destroy();
  font.destroy();
  ::render.destroyDevice();
}