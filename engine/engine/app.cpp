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
#include "components\all_components.h"
#include <time.h>

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

CApp& CApp::get() {
  return the_app;
}

CApp::CApp()
  : xres(640)
  , yres(480)
{ }

void CApp::loadConfig() {
  // Parse xml file...
  xres = 1920;
  yres = 1080;
}

CVertexShader vs_basic;
CVertexShader vs_basic2;
CPixelShader ps_basic;
CPixelShader ps_textured;
CMesh        grid;
CMesh        axis;
CMesh		 wiredCube;
CMesh		 intersectsWiredCube;

TCompCamera*      camera;
CCamera*	  oldCamera;
CFont         font;

CShaderCte<TCtesGlobal> ctes_global;

float fixedUpdateCounter;

void registerAllComponentMsgs() {
	//SUBSCRIBE(TLife, TMsgExplosion, onExplosion);
	//SUBSCRIBE(TLife, TMsgDied, onDied);
}

void createManagers() {


	getObjManager<CEntity>()->init(1024);
	getObjManager<TCompTransform>()->init(1024);
	getObjManager<TCompLife>()->init(32);
	getObjManager<TCompName>()->init(1024);
	getObjManager<TCompMesh>()->init(1024);
	getObjManager<TCompColliderMesh>()->init(32);
	getObjManager<TCompCamera>()->init(4);
	getObjManager<TCompCollider>()->init(512);
	getObjManager<TCompRigidBody>()->init(512);
	getObjManager<TCompStaticBody>()->init(512);
	getObjManager<TCompAABB>()->init(1024);
	getObjManager<TCompPlayerController>()->init(1);
	getObjManager<TCompPlayerPivotController>()->init(1);
	getObjManager<TCompCameraPivotController>()->init(1);
	getObjManager<TCompThirdPersonCameraController>()->init(1);
	getObjManager<TCompDistanceJoint>()->init(32);

	// Lights (temporary)
	getObjManager<TCompDirectionalLight>()->init(16);
	getObjManager<TCompAmbientLight>()->init(1);
	getObjManager<TCompPointLight>()->init(64);
	getObjManager<TCompAiFsmBasic>()->init(64);
	getObjManager<TCompEnemyController>()->init(64);
	
	registerAllComponentMsgs();
}

void initManagers() {
	getObjManager<TCompCamera>()->initHandlers();
	getObjManager<TCompCollider>()->initHandlers();
	getObjManager<TCompRigidBody>()->initHandlers();
	getObjManager<TCompStaticBody>()->initHandlers();
	getObjManager<TCompAABB>()->initHandlers();
	getObjManager<TCompPlayerController>()->initHandlers();
	getObjManager<TCompPlayerPivotController>()->initHandlers();
	getObjManager<TCompCameraPivotController>()->initHandlers();
	getObjManager<TCompThirdPersonCameraController>()->initHandlers();
	getObjManager<TCompDistanceJoint>()->initHandlers();
	getObjManager<TCompAiFsmBasic>()->initHandlers();
	getObjManager<TCompEnemyController>()->initHandlers();
}

bool CApp::create() {

  if (!::render.createDevice())
    return false;

  // Start random seed
  srand(time(NULL));

  // public delta time inicialization
  delta_time = 0.f;

  renderAABB = true;
  renderAxis = true;
  renderGrid = true;
  renderNames = true;

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
  camera = e->get<TCompCamera>();

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
  entity_inspector.inspectEntity(nullptr);

  entity_lister.init();
  entity_actioner.init();
  debug_optioner.init();

  activateInspectorMode(false);

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
  delta_time = delta_secs;
  
  float fps = 1.0f / delta_secs;

  before = now;

  // To avoid the fist huge delta time
  if (delta_secs < 0.5) {
	  update(delta_secs);
	  
	  // Fixed update
	  fixedUpdateCounter += delta_secs;

	  while (fixedUpdateCounter > physics_manager.timeStep) {
		  fixedUpdateCounter -= physics_manager.timeStep;
		  fixedUpdate(physics_manager.timeStep);
	  }
	  
	  /*if (fixedUpdateCounter >= physics_manager.timeStep) {
		  fixedUpdate(fixedUpdateCounter);
		  fixedUpdateCounter = 0;		  
	  }*/
  }
  render();
}

void CApp::update(float elapsed) {

  CIOStatus& io = CIOStatus::get();
  // Update input
  io.update(elapsed);

  if (io.becomesReleased(CIOStatus::INSPECTOR_MODE)) {
	  if (io.getMousePointer())
		  activateInspectorMode(true);
	  else
		  activateInspectorMode(false);
  }

  if (io.becomesPressed(CIOStatus::THROW_STRING)) {

	  // Get the camera position
	  CEntity* e = CEntityManager::get().getByName("Camera");
	  TCompTransform* t = e->get<TCompTransform>();

	  // Raycast detecting the collider the mouse is pointing at
	  PxRaycastBuffer hit;
	  physics_manager.raycast(t->position, t->getFront(), 1000, hit);

	  static int entitycount = 1;
	  if (hit.hasBlock) {		  
		  PxRaycastHit blockHit = hit.block;		  
		  dbg("Click en un actor en: %f, %f, %f\n", blockHit.actor->getGlobalPose().p.x, blockHit.actor->getGlobalPose().p.y, blockHit.actor->getGlobalPose().p.z);
		  dbg("Punto de click: %f, %f, %f\n", blockHit.position.x, blockHit.position.y, blockHit.position.z);
		  
		  CEntity* new_e = entity_manager.createEmptyEntity();

		  TCompName* new_e_name = CHandle::create<TCompName>();
		  strcpy(new_e_name->name, ("RaycastTarget" + std::to_string(entitycount)).c_str());
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
		  new_e_r->create(1, false, true);
		  

		  entitycount++;
	  }

  }
	
  // Update ---------------------
  //  ctes_global.world_time += XMVectorSet(elapsed,0,0,0);
  ctes_global.get()->world_time += elapsed;

  getObjManager<TCompPlayerController>()->update(elapsed); // Update player transform
  getObjManager<TCompPlayerPivotController>()->update(elapsed);
  getObjManager<TCompCameraPivotController>()->update(elapsed);
  getObjManager<TCompThirdPersonCameraController>()->update(elapsed); // Then update camera transform, wich is relative to the player
  getObjManager<TCompCamera>()->update(elapsed);  // Then, update camera view and projection matrix
  getObjManager<TCompAABB>()->update(elapsed); // Update objects AABBs
  getObjManager<TCompAiFsmBasic>()->update(elapsed);
  

  entity_inspector.update();
  entity_lister.update();
  entity_actioner.update();
}

// Physics update
void CApp::fixedUpdate(float elapsed) {
  physics_manager.gScene->simulate(elapsed);
  physics_manager.gScene->fetchResults(true);

  getObjManager<TCompPlayerController>()->fixedUpdate(elapsed); // Update kinematic player
  getObjManager<TCompRigidBody>()->fixedUpdate(elapsed); // Update rigidBodies of the scene
  getObjManager<TCompEnemyController>()->fixedUpdate(elapsed);
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
	  CEntity* e_dirL001 = entity_manager.getEntities()[i];
	  TCompDirectionalLight* dirL001 = e_dirL001->get<TCompDirectionalLight>();
	  if (dirL001 && dirL001->active) {
		  ctes_global.get()->LightDirections[ctes_global.get()->LightCount] = dirL001->direction;
		  ctes_global.get()->LightColors[ctes_global.get()->LightCount] = dirL001->color;
		  ctes_global.get()->LightCount++;
	  }
  }  
	  
  // Ñapa para luces puntuales
  // Recorrer las luces y añadirlas al array
  ctes_global.get()->OmniLightCount = 0;
  for (int i = 0; i < entity_manager.getEntities().size(); ++i) {
	  CEntity* e_pointL = entity_manager.getEntities()[i];
	  TCompPointLight* pointL = e_pointL->get<TCompPointLight>();
	  if (pointL && pointL->active) {
		  ctes_global.get()->OmniLightColors[ctes_global.get()->OmniLightCount] = pointL->color;
		  ctes_global.get()->OmniLightPositions[ctes_global.get()->OmniLightCount] = pointL->position;
		  ctes_global.get()->OmniLightRadius[ctes_global.get()->OmniLightCount] = XMVectorSet(pointL->radius, 0, 0, 0);
		  ctes_global.get()->OmniLightCount++;
	  }
  }
  int a = ctes_global.get()->OmniLightCount;

  ctes_global.uploadToGPU();
  ctes_global.activateInPS(2);

  //ctes_global.activateInVS(2);

  activateWorldMatrix(0);
  activateTint(0);
  
  //activateCamera(*camera, 1);
  // TODO: Make activate TCamera
  activateCamera(
	  camera->view_projection, 
	  ((TCompTransform*)((CEntity*)CHandle(camera).getOwner())->get<TCompTransform>())->position,
	  1
  );

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
	  TCompTransform* t = ((CEntity*)entity_manager.getEntities()[i])->get<TCompTransform>();
	  TCompMesh* mesh = ((CEntity*)entity_manager.getEntities()[i])->get<TCompMesh>();

	  // If the component has no transform it can't be rendered
	  if (!t)
		  continue;

	  if (mesh)
		setTint(mesh->color);

	  setWorldMatrix(t->getWorld());

	  if (mesh && mesh->active)
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

void CApp::destroy() {
  TwTerminate();
  mesh_manager.destroyAll();
  texture_manager.destroyAll();
  axis.destroy();
  grid.destroy();
  intersectsWiredCube.destroy();
  wiredCube.destroy();
  renderUtilsDestroy();
  vs_basic.destroy();
  vs_basic2.destroy();
  ps_basic.destroy();
  ps_textured.destroy();
  font.destroy();
  ::render.destroyDevice();
}