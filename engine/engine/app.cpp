#include "mcv_platform.h"
#include "app.h"
#include "camera.h"
#include "render/render_utils.h"
#include "entity.h"
#include "doom_controller.h"
#include "font/font.h"
#include "render/texture.h"
#include "importer_parser.h"
#include "physics_manager.h"
#include "handle.h"

using namespace DirectX;
#include "render/ctes/shader_ctes.h"

#include <PxPhysicsAPI.h>
#include <foundation\PxFoundation.h>

using namespace physx;

#include <AntTweakBar.h>

static CApp the_app;

CEntityManager &entity_manager = CEntityManager::get();
CPhysicsManager &physics_manager = CPhysicsManager::get();

CApp& CApp::get() {
  return the_app;
}

CApp::CApp()
  : xres(640)
  , yres(480)
{ }

void CApp::loadConfig() {
  // Parse xml file...
  xres = 800;
  yres = 600;
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
	getObjManager<TPlayerDoomController>()->init(1);
	getObjManager<TThirdPersonCameraController>()->init(8);
}

void initManagers() {
	getObjManager<TCamera>()->initHandlers();
	getObjManager<TCollider>()->initHandlers();
	getObjManager<TRigidBody>()->initHandlers();
	getObjManager<TStaticBody>()->initHandlers();
	getObjManager<TAABB>()->initHandlers();
	getObjManager<TPlayerDoomController>()->initHandlers();
	getObjManager<TThirdPersonCameraController>()->initHandlers();
}

// AntTweakBar button test
void TW_CALL CallbackCreateEntity(void *clientData)
{
	// Create a new entity with some components
	CEntity* e = entity_manager.createEmptyEntity();

	TCompName* n = getObjManager<TCompName>()->createObj();
	std::strcpy(n->name, "Nueva entidad");
	e->add(n);

	TTransform* t = getObjManager<TTransform>()->createObj();
	t->position = XMVectorSet(0, 0, 10, 1);
	t->rotation = XMVectorSet(0, 0, 0, 1);
	t->scale = XMVectorSet(0.5f, 0.5f, 0.5f, 1);
	e->add(t);

	TAABB* aabb = getObjManager<TAABB>()->createObj();
	aabb->setIdentityMinMax(XMVectorSet(-4.5f, 0, -3, 0), XMVectorSet(5.14219f, 4.725f, 3, 0));
	e->add(aabb);
	aabb->init();

	TMesh* m = getObjManager<TMesh>()->createObj();
	m->mesh = mesh_manager.getByName("Teapot");
	e->add(m);
}

bool CApp::create() {

  if (!::render.createDevice())
    return false;

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

  camera = entity_manager.getByName("Camera")->get<TCamera>();

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
  
  // Inspector de entidades
  CEntity* e = entity_manager.getByName("Peter la tetera");
  TTransform* e_transform = e->get<TTransform>();
  TCompName* e_name = e->get<TCompName>();
  TAABB* e_aabb = e->get<TAABB>();
  TCollider* e_collider = e->get<TCollider>();
  TRigidBody* e_rigidbody = e->get<TRigidBody>();

  // Create a tewak bar
  TwBar *bar = TwNewBar("Test bar");
  
  // AntTweakBar test
  int barSize[2] = { 224, 320 };
  TwSetParam(bar, NULL, "size", TW_PARAM_INT32, 2, barSize);
  TwAddButton(bar, "Create Entity", CallbackCreateEntity, NULL, "");

  if (e_name) {
	  TwAddVarRW(bar, "Name", TW_TYPE_CSSTRING(sizeof(e_name->name)), e_name->name, " group=Name" );
	  TwAddSeparator(bar, "Name", "");
  }
  if (e_transform) {
	  TwAddVarRW(bar, "Position", TW_TYPE_DIR3F, &e_transform->position, " group=Transform" );
	  TwAddVarRW(bar, "Rotation", TW_TYPE_QUAT4F, &e_transform->rotation, " group=Transform" );
	  TwAddVarRW(bar, "Scale", TW_TYPE_DIR3F, &e_transform->scale, " group=Transform" );
	  TwAddSeparator(bar, "Transform", "");
  }
  if (e_aabb) {
	  TwAddVarRW(bar, "Min", TW_TYPE_DIR3F, &e_aabb->min, " group=AABB" );
	  TwAddVarRW(bar, "Max", TW_TYPE_DIR3F, &e_aabb->max, " group=AABB" );
	  TwAddSeparator(bar, "AABB", "");
  }
  if (e_collider) {
	  
	  //TwAddVarRO(bar, "Material", TW_TYPE_DIR3F, &e_collider->getMaterialProperties(), " group=Collider ");
  }
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

  // Update ---------------------
  //  ctes_global.world_time += XMVectorSet(elapsed,0,0,0);
  ctes_global.get()->world_time += elapsed;

  getObjManager<TAABB>()->update(elapsed); // Update objects AABBs
  getObjManager<TPlayerDoomController>()->update(elapsed); // Update player transform
  getObjManager<TThirdPersonCameraController>()->update(elapsed); // Then update camera transform, wich is relative to the player
  getObjManager<TCamera>()->update(elapsed);  // Then, update camera view and projection matrix
}

// Physics update
void CApp::fixedUpdate(float elapsed) {
  physics_manager.gScene->simulate(physics_manager.timeStep);
  physics_manager.gScene->fetchResults(true);

  getObjManager<TPlayerDoomController>()->fixedUpdate(elapsed); // Update kinematic player
  getObjManager<TRigidBody>()->fixedUpdate(elapsed); // Update rigidBodies of the scene
}

void CApp::render() {

  // Render ---------------------
  float ClearColor[4] = { 0.1f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
  ::render.ctx->ClearRenderTargetView(::render.render_target_view, ClearColor);
  ::render.ctx->ClearDepthStencilView(::render.depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);

  activateTextureSamplers();

  vs_basic.activate();
  ps_basic.activate();  
  //ps_textured.activate();
  const CTexture *t = texture_manager.getByName("wood_d");
  t->activate(0);

  activateWorldMatrix(0);
  //activateCamera(*camera, 1);
  // TODO: Make activate TCamera
  activateCamera(camera->view_projection, 1);

  setWorldMatrix(XMMatrixIdentity());
  grid.activateAndRender();
  axis.activateAndRender();

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
  renderDebugEntities(true);
  renderEntityDebugList();

  TwDraw();

  ::render.swap_chain->Present(0, 0);

}

void CApp::renderEntities() {
  
  // Render entities
	for (int i = 0; i < entity_manager.getEntities().size(); ++i)
  {
	  TTransform* t = entity_manager.getEntities()[i]->get<TTransform>();
	  TMesh* mesh = entity_manager.getEntities()[i]->get<TMesh>();

	  // If the component has no transform it can't be rendered
	  if (!t)
		  continue;

	  setWorldMatrix(t->getWorld());
	  ctes_global.uploadToGPU();

	  if (mesh)
		mesh->mesh->activateAndRender();
  }
}

void CApp::renderDebugEntities(bool draw_names) {
	// Render entities
	for (int i = 0; i < entity_manager.getEntities().size(); ++i)
	{
		TTransform* t = entity_manager.getEntities()[i]->get<TTransform>();
		TCompName* name = entity_manager.getEntities()[i]->get<TCompName>();
		TAABB* aabb = entity_manager.getEntities()[i]->get<TAABB>();

		// If the component has no transform it can't be rendered
		if (!t)
			continue;

		setWorldMatrix(t->getWorld());
		ctes_global.uploadToGPU();
		axis.activateAndRender();

		// If the entity has name, print it
		if (name && draw_names)
			font.print3D(t->position, name->name);

		// If the entity has an AABB, draw it
		if (aabb) {
			bool intersects = false;
			for (int j = 0; j < entity_manager.getEntities().size(); ++j) {
				TAABB* aabb2 = entity_manager.getEntities()[j]->get<TAABB>();
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

void CApp::renderEntityDebugList() {
	font.printf(10, 10, "Entities: %i\nPress <L> to list entities, press <K> to inspect entities in front of you", entity_manager.getEntities().size());	

	TTransform* player_t = entity_manager.getByName("Player")->get<TTransform>();

	font.printf(10, 45, "Player position: (%f, %f, %f)", XMVectorGetX(player_t->position), XMVectorGetY(player_t->position), XMVectorGetZ(player_t->position));
	font.printf(10, 60, "Player rotation: (%f, %f, %f, %f)", XMVectorGetX(player_t->rotation), XMVectorGetY(player_t->rotation), XMVectorGetZ(player_t->rotation), XMVectorGetW(player_t->rotation));

	bool debug_all_entities = isKeyPressed('L');
	bool debug_front_entities = isKeyPressed('K');
	bool entity_in_front = false;
	int line_jump_count = 0;
	int current_jump_count = 0;
	int draw_counter = 0;

	if (debug_all_entities || debug_front_entities)
	{
		std::string s;
		std::string s_name;
		for (int i = 0; i < entity_manager.getEntities().size(); ++i) {
			s = "";
			s_name = "";
			current_jump_count = 0;
			TCompName* name = entity_manager.getEntities()[i]->get<TCompName>();
			TTransform* t = entity_manager.getEntities()[i]->get<TTransform>();
			TMesh* mesh = entity_manager.getEntities()[i]->get<TMesh>();
			TCamera* cam = entity_manager.getEntities()[i]->get<TCamera>();
			TCollider* collider = entity_manager.getEntities()[i]->get<TCollider>();
			TRigidBody* rigid = entity_manager.getEntities()[i]->get<TRigidBody>();			
			TAABB* aabb = entity_manager.getEntities()[i]->get<TAABB>();

			if (debug_all_entities)
			{
				if (name)
					s_name = name->toString() + "\n";
			}
			else if (debug_front_entities)
			{
				if (t && XMVectorGetX(XMVector3Dot(XMVector3Normalize(t->position - player_t->position), player_t->getFront())) > 0.8f)
				{
					if (name)
						s_name = name->toString() + "\n";
					if (t)
						s += "" + t->toString() + "\n";
					if (aabb)
						s += "" + aabb->toString() + "\n";
					if (mesh)
						s += "" + mesh->toString() + "\n";
					if (cam)
						s += "" + cam->toString() + "\n";
					if (collider)
						s += "" + collider->toString() + "\n";
					if (rigid)
						s += "" + rigid->toString() + "\n";
				}
			}

			for (int j = 0; j < s.size(); j++) {
				if (s[j] == '\n') { current_jump_count++; }
			}
			line_jump_count += current_jump_count;

			if (!s.empty() || !s_name.empty()) {
				draw_counter++;
				font.color = 0xffff00ff;
				font.printf(20, 80 + (line_jump_count - current_jump_count + draw_counter) * 20, s_name.c_str());
				font.color = 0xffffffff;
				font.printf(30, 80 + (line_jump_count - current_jump_count + draw_counter + 1) * 20, s.c_str());
			}
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