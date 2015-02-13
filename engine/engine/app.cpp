#include "mcv_platform.h"
#include "app.h"
#include "camera.h"
#include "render/render_utils.h"
#include "entity.h"
#include "doom_controller.h"
#include "camera_pivot_controller.h"
#include "font/font.h"
#include "render/texture.h"
#include "camera_pivot_controller.h"
#include "camera_controller.h"
#include <string>
#include <vector>


#include <PxPhysicsAPI.h>
#include <foundation\PxFoundation.h>

using namespace physx;

using namespace DirectX;
#include "render/ctes/shader_ctes.h"
#include "ai\aic_melee_gatekeeper.h"
#include "ai\ai_basic_patroller.h"


using namespace physx;

// Variables de Escena y SDK
PxPhysics *gPhysicsSDK;
PxScene* gScene = NULL;


// Estos calbacks aun me los he de estudiar
static PxDefaultErrorCallback gDefaultErrorCallback;
static PxDefaultAllocator gDefaultAllocatorCallback;
static PxSimulationFilterShader gDefaultFilterShader = PxDefaultSimulationFilterShader;

// Ahora mismo el mundo avanza a 16,6666 ms cada tick ( esto deberia ser variable o fijo ya lo veremos )
PxReal myTimestep = 1.0f / 60.0f;


// Por ahora me guardo uno de los objectos pero mas adelante ya usaremos manager
PxRigidDynamic* object;

static CApp the_app;

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

CMesh		 camera_mesh;
CMesh        grid;
CMesh        axis;
CMesh		 cube;
CMesh		 cubeMini;
const CMesh*		 prota;

CCamera       camera;
CCamera       camera2;
CFont         font;

CThirdPersonController   third_person_controller;
camera_pivot_controller CPC;
CCamera_controller camera_controller;
CAimToController  aim_controller;
CLookAtController lookat_controller;

//Punteros ---------------------------------
CCamera* cam_pointer;
camera_pivot_controller* cam_pivot_pointer;
//------------------------------------------

CEntity*         player;
CEntity*         e3;

CEntity*		 cameraPivot;
CEntity*		 cameraEntity;

CEntity*		 box1;
PxRigidDynamic*  dynamicBox1;
CEntity*		 box2;
PxRigidDynamic*  dynamicBox2;

CEntity*		 box3;
PxRigidStatic*	 staticBox1;

PxRigidDynamic*	 playerRigid;

CShaderCte<TCtesGlobal> ctes_global;

std::vector<CEntity*>	balls;

// AI
aic_melee_gatekeeper	aimg;
ai_basic_patroller		aibp;


// Copy paste mas o menos de la docu
void InitializePhysX()
{

	auto i = 10;
	PxFoundation* foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);

	if (!foundation)
		OutputDebugString("ERROR FOUNDATION!\n");
	else
		OutputDebugString("YOSH!!\n");

	gPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale());
	
	// ------------------------------------
	physx::PxVisualDebuggerConnectionFlags flags =
		physx::PxVisualDebuggerConnectionFlag::eDEBUG
		| physx::PxVisualDebuggerConnectionFlag::ePROFILE
		| physx::PxVisualDebuggerConnectionFlag::eMEMORY;

	if (gPhysicsSDK->getPvdConnectionManager())
	{
		PxVisualDebuggerConnection* gConnection = PxVisualDebuggerExt::createConnection(gPhysicsSDK->getPvdConnectionManager(), "127.0.0.1", 5425, 1000, flags);
	}

	
	gPhysicsSDK->getVisualDebugger()->setVisualizeConstraints(true);
	gPhysicsSDK->getVisualDebugger()->setVisualDebuggerFlag(physx::PxVisualDebuggerFlag::eTRANSMIT_CONTACTS, true);
	gPhysicsSDK->getVisualDebugger()->setVisualDebuggerFlag(physx::PxVisualDebuggerFlag::eTRANSMIT_CONSTRAINTS, true);
	gPhysicsSDK->getVisualDebugger()->setVisualDebuggerFlag(physx::PxVisualDebuggerFlag::eTRANSMIT_SCENEQUERIES, true);
	// ------------------------------------

	PxInitExtensions(*gPhysicsSDK);

	PxSceneDesc sceneDesc(gPhysicsSDK->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.8f, 0.0f);	//Ponemos gravedad terrestre ( Para PhysX Y es vertical )

	if (!sceneDesc.cpuDispatcher){
		PxDefaultCpuDispatcher* mCpuDispatcher = PxDefaultCpuDispatcherCreate(1);
		sceneDesc.cpuDispatcher = mCpuDispatcher;
	}

	// Ni idea de lo que es esto
	if (!sceneDesc.filterShader)
		sceneDesc.filterShader = gDefaultFilterShader;

	gScene = gPhysicsSDK->createScene(sceneDesc);


}

void throwBall()
{
	CEntity* ball = entity_manager.create("Ball");

	PxMaterial* mMaterial;
	mMaterial = gPhysicsSDK->createMaterial(0.5f, 0.5f, 0.1f);    //static friction, dynamic friction, restitution


	PxVec3 pos = PxVec3(
		XMVectorGetX(player->getPosition()),
		XMVectorGetY(player->getPosition()),
		XMVectorGetZ(player->getPosition())
		);

	PxVec3 delta = PxVec3(
		XMVectorGetX(player->getFront()),
		XMVectorGetY(player->getFront()),
		XMVectorGetZ(player->getFront())
		);

	PxRigidDynamic* ballActor = PxCreateDynamic(*gPhysicsSDK, PxTransform(pos + delta * 1.5f), PxSphereGeometry(0.1f),
		*mMaterial, 1000.0f);
	
	gScene->addActor(*ballActor);
	ballActor->addForce(delta * 100, PxForceMode::eIMPULSE, true);

	ball->rigid = ballActor;
	balls.push_back(ball);
}


PxDistanceJoint* j;

void CreateActors(){

	// Declaramos un material fisico a usar
	PxMaterial* mMaterial;
	mMaterial = gPhysicsSDK->createMaterial(0.5f, 0.5f, 0.1f);    //static friction, dynamic friction, restitution

	// Creamos un rigidbody dinamico que sera una esfera de radio 10 i lo ponemos en posicion 100 en Y
	/*PxRigidDynamic* aSphereActor = PxCreateDynamic(*gPhysicsSDK, PxTransform(PxVec3(0, 400, 0)), PxSphereGeometry(1),
		*mMaterial, 1.0f);*/
	//Aplicamos velocidad vertical y anyadimos objeto a la escena
	//aSphereActor->setLinearVelocity(PxVec3(0, -10, 0));
	//gScene->addActor(*aSphereActor);

	// añadir cajas
	PxRigidDynamic* aBoxActor = PxCreateDynamic(*gPhysicsSDK, PxTransform(PxVec3(0, 2, 0)), PxBoxGeometry(1, 1, 1),
		*mMaterial, 2.0f);

	gScene->addActor(*aBoxActor);
	dynamicBox1 = aBoxActor;

	PxRigidDynamic* aBoxActor2 = PxCreateDynamic(*gPhysicsSDK, PxTransform(PxVec3(0, 10, 5)), PxBoxGeometry(1, 1, 1),
		*mMaterial, 100.0f);

	gScene->addActor(*aBoxActor2);
	dynamicBox2 = aBoxActor2;

	staticBox1 = PxCreateStatic(*gPhysicsSDK, PxTransform(PxVec3(0, 10, 5)), PxBoxGeometry(1, 1, 1), *mMaterial);
	gScene->addActor(*staticBox1);

	// Creamos un suelo para que la esfera no caiga al vacio
	PxRigidStatic* plane = PxCreatePlane(*gPhysicsSDK, PxPlane(PxVec3(0, 1, 0), 0), *mMaterial);
	gScene->addActor(*plane);
	

	// Por ahora me guardo el objeto esfera en un puntero global para verlo despues
	//object = aSphereActor;
	//create a joint  
	j = PxDistanceJointCreate(*gPhysicsSDK, dynamicBox2, PxTransform::createIdentity(), staticBox1, PxTransform::createIdentity());
	j->setDamping(5);
	j->setMaxDistance(5);
	j->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, true);
	j->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);

	// Player kinemático
	playerRigid = PxCreateDynamic(*gPhysicsSDK, PxTransform(PxVec3(0, 0, 0)), /*PxBoxGeometry(1, 1, 1)*/PxCapsuleGeometry(0.2f, 0.5f),
		*mMaterial, 100.0f);

	playerRigid->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

	gScene->addActor(*playerRigid);

	

}


// Que haga cosas
float throwCounter = 0.0f;

float physxCounter = 0.0f;
const float physxStep = 1.f / 60.f;
bool released = false;
void StepPhysX(float delta){
	physxCounter += delta;
	throwCounter += delta;
	if (physxCounter >= physxStep)
	{
		if (isKeyPressed('C'))
		{
			if (!released)
				j->release();
			released = true;
		}
		if (isKeyPressed(' '))
		{
			if (throwCounter >= .5f)
			{
				throwBall();
				throwCounter = 0;
			}
		}
		physxCounter = 0;
		gScene->simulate(physxStep);
		gScene->fetchResults(true);
	}
	
}


bool CApp::create() {	
	
  if (!::render.createDevice())
    return false;

  bool is_ok = vs_basic.compile("Tutorial04.fx", "VS", vdcl_position_color)
    && ps_basic.compile("Tutorial04.fx", "PS")
    && vs_basic2.compile("Tutorial04.fx", "VSNormal", vdcl_position_uv_normal)
    && ps_textured.compile("Tutorial04.fx", "PSTextured")
  ;
  assert(is_ok);

  is_ok = font.create();
  font.camera = &camera;
  assert(is_ok);

  // Ctes ---------------------------
  is_ok &= renderUtilsCreate();

  // Initialize the world matrix
  camera.setOverlap(1.5f, 2.f);
  camera.lookAt(XMVectorSet(10.f, 8.f, 2.f, 1.f)
    , XMVectorSet(0.f, 0.f, 0.f, 1.f)
    , XMVectorSet(0, 1, 0, 0));
  camera.setPerspective(deg2rad(75.f), 1.f, 1000.f);
  camera.setViewport(0.f, 0.f, (float)xres, (float)yres);

  camera2.lookAt(XMVectorSet(10.f, 0.f, 0.f, 1.f)
	  , XMVectorSet(0.f, 0.f, 0.f, 1.f)
	  , XMVectorSet(0, 1, 0, 0));
  camera2.setPerspective(deg2rad(50.f), 1.f, 500.f);
  camera2.setViewport(0.f, 0.f, (float)xres, (float)yres);


  //ctes_global.world_time = XMVectorSet(0, 0, 0, 0);
  ctes_global.get()->world_time = 0.f;// XMVectorSet(0, 0, 0, 0);
  is_ok &= ctes_global.create();
  assert(is_ok);

  is_ok &= createGrid(grid, 10);
  is_ok &= createAxis(axis);

  assert(is_ok);

  player = entity_manager.create("Player");
  player->setPosition(XMVectorSet(-3, 0, -3, 1));
  player->setRotation(XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), deg2rad(30.f)));

  cameraEntity = entity_manager.create("Third person camera");
  cameraPivot = entity_manager.create("Third person camera pivot");

  //---------- Inicializando controladores ----------
  XMVECTOR offset = { -2.f, 1.5f, -2.f, 0.f };
  CPC.init(offset, player);
  cam_pointer = &camera;
  cam_pivot_pointer = &CPC;
  //--------------------------------------------------

  e3 = entity_manager.create("camera");
  e3->setPosition(XMVectorSet(-5, 4, -3, 1));

  createCube(cube, 1);
  createCube(cubeMini, 0.1f);

  createCamera(camera_mesh);

  box1 = entity_manager.create("Box001");
  box2 = entity_manager.create("Box002");
  box3 = entity_manager.create("Box003");

  prota = mesh_manager.getByName("prota");

  // Gatekeeper
  CEntity* gate = entity_manager.create("Gate");
  gate->setPosition(XMVectorSet(10, 0, 10, 0));
  aimg.player = player;
  aimg.gate = gate;
  aimg.Init();
  aimg.SetEntity(entity_manager.create("Gatekeeper"));
  aimg.entity->setPosition(XMVectorSet(10, 0, 10, 0));

  // Patroller
  // Waypoints
  CEntity* e;
  vector<CEntity*>	 waypoints;
  e = entity_manager.create("Waypoint 1");
  e->setPosition(XMVectorSet(-20, 1, -14, 1));
  waypoints.push_back(e);
  e = entity_manager.create("Waypoint 2");
  e->setPosition(XMVectorSet(-16, 1, 22, 1));
  waypoints.push_back(e);
  e = entity_manager.create("Waypoint 3");
  e->setPosition(XMVectorSet(19, 1, -17, 1));
  waypoints.push_back(e);
  e = entity_manager.create("Waypoint 4");
  e->setPosition(XMVectorSet(14, 1, 27, 1));
  waypoints.push_back(e);
  e = entity_manager.create("Waypoint 5");
  e->setPosition(XMVectorSet(-10, 1, -23, 1));
  waypoints.push_back(e);

  // Basic patroller
  aibp.entity = entity_manager.create("Patroller");
  aibp.entity->setPosition(XMVectorSet(-20, 1, -14, 1));
  aibp.waypoints = waypoints;

  aibp.Init();

  InitializePhysX();
  CreateActors(); 

  playerRigid->setGlobalPose(PxTransform(PxVec3(-3, 1, -3), PxQuat(deg2rad(30), PxVec3(0, 1, 0))), true);
  
  return true;
}

void moveCameraOnEntity(CCamera& camera, CEntity *e) {
	camera.lookAt(e->getPosition() + e->getUp()*camera.overlapY - e->getFront()*camera.overlapZ, e->getPosition() + e->getFront() / 2, e->getUp());
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
  //dbg("delta is %f fps = %f\n", delta_secs, fps);

  update(delta_secs);
  render();
  
  if (gScene){	  
	  StepPhysX(delta_secs);
  }

}

void CApp::update(float elapsed) {
	
  // Update ---------------------
	third_person_controller.update(player, CPC.getPlayerPivot(), elapsed);
  
  lookat_controller.update(e3, aibp.entity, elapsed);
  ctes_global.get()->world_time += elapsed;

  /*cameraPivot->setPosition(player->getPosition() + player->getUp());
  cameraPivot->setRotation(player->getRotation());

  cameraEntity->setPosition(cameraPivot->getPosition() - cameraPivot->getFront() * 3 + cameraPivot->getUp() * 2);
  cameraEntity->setRotation(cameraPivot->getRotation());
  lookat_controller.update(cameraEntity, cameraPivot, elapsed);*/

  moveCameraOnEntity(camera, CPC.getCamPivot());
  moveCameraOnEntity(camera2, e3);

  // AI
  aimg.Recalc(elapsed);
  aibp.Recalc(elapsed);

  //-------- actualizando controladores camara -----------
  if (isKeyPressed('Z')){
	  exit(-1);
  }

  CPC.update();
  camera_controller.update(cam_pointer, cam_pivot_pointer, elapsed);
  //--------------------------------------------------------------------------------------------
}

void CApp::render() {

  // Render ---------------------
  float ClearColor[4] = { 0.1f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
  ::render.ctx->ClearRenderTargetView(::render.render_target_view, ClearColor);
  ::render.ctx->ClearDepthStencilView(::render.depth_stencil_view, D3D11_CLEAR_DEPTH, 1.0f, 0);

  activateTextureSamplers();

  vs_basic.activate();
  ps_basic.activate();

  activateWorldMatrix(0);
  activateCamera(camera, 1);

  setWorldMatrix(XMMatrixIdentity());
  grid.activateAndRender();
  axis.activateAndRender();

  setWorldMatrix(e3->getWorld());
  camera_mesh.activateAndRender();
  setWorldMatrix(cameraEntity->getWorld());

  camera_mesh.activateAndRender();
  drawViewVolume(camera);
  drawViewVolume(camera2);

  setWorldMatrix(XMMatrixIdentity());

  static int nframe = 0;
  font.printf(10, 10, "Yaw Is %f", rad2deg(getYawFromVector(player->getFront())));

  ctes_global.uploadToGPU();

  // Tech con textura
  /*vs_basic2.activate();
  ps_textured.activate();
  const CTexture *t = texture_manager.getByName("wood_d");
  t->activate(0);
  

  setWorldMatrix(XMMatrixTranslation(0, object->getGlobalPose().p.y, 0));
  const CMesh* teapot = mesh_manager.getByName("Box001");
  ctes_global.activateInVS(2);
  teapot->activateAndRender();*/

  // Player kinemático
  PxQuat rotationPlayer = PxQuat(
	  XMVectorGetX(player->getRotation()),
	  XMVectorGetY(player->getRotation()),
	  XMVectorGetZ(player->getRotation()),
	  XMVectorGetW(player->getRotation()));

  rotationPlayer *= PxQuat(deg2rad(90), PxVec3(0, 0, 1));

  playerRigid->setKinematicTarget(PxTransform(
	  PxVec3(
		XMVectorGetX(player->getPosition() + player->getUp() * 0.5f),
		XMVectorGetY(player->getPosition() + player->getUp() * 0.5f),
		XMVectorGetZ(player->getPosition() + player->getUp() * 0.5f)
	  ),
		rotationPlayer	  
	  ));

  setWorldMatrix(player->getWorld());
  ctes_global.uploadToGPU();
  prota->activateAndRender();

  // Cajas con físicas
  box1->setPosition(XMVectorSet(
	  dynamicBox1->getGlobalPose().p.x,
	  dynamicBox1->getGlobalPose().p.y,
	  dynamicBox1->getGlobalPose().p.z,
	  1
	  ));
  box1->setRotation(XMVectorSet(
	  dynamicBox1->getGlobalPose().q.x,
	  dynamicBox1->getGlobalPose().q.y,
	  dynamicBox1->getGlobalPose().q.z,
	  dynamicBox1->getGlobalPose().q.w
	  ));
  setWorldMatrix(box1->getWorld());
  ctes_global.uploadToGPU();
  cube.activateAndRender();

  box2->setPosition(XMVectorSet(
	  dynamicBox2->getGlobalPose().p.x,
	  dynamicBox2->getGlobalPose().p.y,
	  dynamicBox2->getGlobalPose().p.z,
	  1
	  ));
  box2->setRotation(XMVectorSet(
	  dynamicBox2->getGlobalPose().q.x,
	  dynamicBox2->getGlobalPose().q.y,
	  dynamicBox2->getGlobalPose().q.z,
	  dynamicBox2->getGlobalPose().q.w
	  ));
  setWorldMatrix(box2->getWorld());
  ctes_global.uploadToGPU();
  cube.activateAndRender();

  box3->setPosition(XMVectorSet(
	  staticBox1->getGlobalPose().p.x,
	  staticBox1->getGlobalPose().p.y,
	  staticBox1->getGlobalPose().p.z,
	  1
	  ));
  box3->setRotation(XMVectorSet(
	  staticBox1->getGlobalPose().q.x,
	  staticBox1->getGlobalPose().q.y,
	  staticBox1->getGlobalPose().q.z,
	  staticBox1->getGlobalPose().q.w
	  ));
  setWorldMatrix(box3->getWorld());
  ctes_global.uploadToGPU();
  cube.activateAndRender();

  for (CEntity* e : balls)
  {
	  e->setPosition(XMVectorSet(
		  e->rigid->getGlobalPose().p.x,
		  e->rigid->getGlobalPose().p.y,
		  e->rigid->getGlobalPose().p.z,
		  1
		  ));
	  e->setRotation(XMVectorSet(
		  e->rigid->getGlobalPose().q.x,
		  e->rigid->getGlobalPose().q.y,
		  e->rigid->getGlobalPose().q.z,
		  e->rigid->getGlobalPose().q.w
		  ));
	  setWorldMatrix(e->getWorld());
	  ctes_global.uploadToGPU();
	  cubeMini.activateAndRender();
  }

  // AI
  setWorldMatrix(aimg.entity->getWorld());
  ctes_global.uploadToGPU();
  cube.activateAndRender();

  setWorldMatrix(aibp.entity->getWorld());
  ctes_global.uploadToGPU();
  cube.activateAndRender();

  renderEntities();

  ::render.swap_chain->Present(0, 0);

}

void CApp::renderEntities() {
  // Render entities
  auto& entities = entity_manager.getEntities();
  axis.activate();
  for (auto& e : entities) {
    setWorldMatrix(e->getWorld());
    axis.render();
    font.print3D(e->getPosition(), e->name);
  }
}


void CApp::destroy() {
	gScene->release();
	gPhysicsSDK->release();
	mesh_manager.destroyAll();
	texture_manager.destroyAll();
	axis.destroy();
	grid.destroy();
	cube.destroy();
	cubeMini.destroy();
	renderUtilsDestroy();
	vs_basic.destroy();
	vs_basic2.destroy();
	ps_basic.destroy();
	ps_textured.destroy();
	font.destroy();
	::render.destroyDevice();
}

