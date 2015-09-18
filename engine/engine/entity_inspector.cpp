#include "mcv_platform.h"
#include "entity_inspector.h"
#include <AntTweakBar.h>
#include "handle\handle.h"
#include "components\all_components.h"
#include "components\comp_skeleton.h"
#include "skeletons\skeleton_manager.h"
#include "components\comp_skeleton_ik.h"
#include "importer_parser.h"
#include "ai\logic_manager.h"
#include <locale>
#include <algorithm>
#include "render\all_post_process_effects.h"
#include "particles\particle_system_subtypes.h"
#include "particles\importer_particle_groups.h"

using namespace physx;

TwBar *bar;

XMVECTOR center;
XMVECTOR extents;
XMVECTOR size;

// Rigidbody
PxVec3 linearVelocity;
PxVec3 angularVelocity;

// Particles
TwEnumVal particleEmitterShapeEV[] = { { TParticleEmitterShape::SPHERE, "Sphere" }, { TParticleEmitterShape::SEMISPHERE, "Semisphere" }, { TParticleEmitterShape::RING, "Ring" }, { TParticleEmitterShape::CONE, "Cone" }, { TParticleEmitterShape::BOX, "Box" } };
TwType particleEmitterShape;

TwEnumVal particleRenderModeEV[] = { { TParticleRenderType::BILLBOARD, "Billboard" }, { TParticleRenderType::H_DIR_BILLBOARD, "H-Dir-Billboard" }, { TParticleRenderType::H_BILLBOARD, "H-Billboard" }, { TParticleRenderType::V_BILLBOARD, "V-Billboard" }, { TParticleRenderType::STRETCHED_BILLBOARD, "Stretched Billboard" } };
TwType particleRenderMode;

TwEnumVal particleRenderAnimationModeEV[] = { { 0, "Animation" }, { 1, "Random particle" }, { 2, "Random row animation" } };
TwType particleRenderAnimationMode;

TwEnumVal particleRenderStretchModeEV[] = { { 0, "Normal" }, { 1, "Stretch with speed" } };
TwType particleRenderStretchMode;

TwEnumVal particleCurveEV[] = { { TParticleCurve::LINEAL, "Linear" }, { TParticleCurve::EXPONENTIAL, "Exponential" }, { TParticleCurve::LOGARITHM, "Logarithmic" } };
TwType particleCurve;

CEntityInspector::CEntityInspector() { }

CEntityInspector::~CEntityInspector() {
	SAFE_DELETE(particleRenderTextureListEV);
}

static CEntityInspector entity_inspector;

CEntityInspector& CEntityInspector::get() {
	return entity_inspector;
}

void CEntityInspector::init() {
	// Create a tewak bar
	bar = TwNewBar("Inspector");
	CApp &app = CApp::get();
	
	// AntTweakBar test
	int barSize[2] = { 224, app.yres };
	int varPosition[2] = { app.xres - barSize[0], 0 };
	TwSetParam(bar, NULL, "size", TW_PARAM_INT32, 2, barSize);
	TwSetParam(bar, NULL, "position", TW_PARAM_INT32, 2, varPosition);
	TwDefine(" Inspector label='Entity inspector' ");
	TwDefine(" Inspector refresh='0.1' ");
	TwDefine(" TW_HELP visible=false ");

	// Particles
	particleEmitterShape = TwDefineEnum("ParticlEmitterShape", particleEmitterShapeEV, 5);
	particleRenderMode = TwDefineEnum("particleRenderMode", particleRenderModeEV, 5);
	particleRenderAnimationMode = TwDefineEnum("particleRenderAnimationMode", particleRenderAnimationModeEV, 3);
	particleRenderStretchMode = TwDefineEnum("particleRenderStretchMode", particleRenderStretchModeEV, 2);
	particleCurve = TwDefineEnum("particleCurve", particleCurveEV, 3);

	// List of textures
	files_in_directory("data/textures/particles", texture_list);

	particleRenderTextureListEV = new TwEnumVal[texture_list.size()];
	for (int j = 0; j < texture_list.size(); ++j) {
		texture_list[j] = texture_list[j].substr(0, texture_list[j].size() - 4);
		TwEnumVal val;
		val.Label = texture_list[j].c_str();
		val.Value = j;
		particleRenderTextureListEV[j] = val;
	}

}

// ---------------------------- GET AND SET COMPONENTS --------------------------

// TRANSFORM
void TW_CALL GetPitch(void *value, void *clientData)
{
	*static_cast<float *>(value) = rad2deg( getPitchFromVector(static_cast<TCompTransform *>(clientData)->getFront()) );
}

// COLLIDER
void TW_CALL SetStaticFriction(const void *value, void *clientData)
{
	static_cast<TCompColliderBox *>(clientData)->getMaterial()->setStaticFriction(*static_cast<const float *>(value));
}
void TW_CALL GetStaticFriction(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<CCollider *>(clientData)->getMaterial()->getStaticFriction();
}

void TW_CALL SetDynamicFriction(const void *value, void *clientData)
{
	static_cast<CCollider *>(clientData)->getMaterial()->setDynamicFriction(*static_cast<const float *>(value));
}
void TW_CALL GetDynamicFriction(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<CCollider *>(clientData)->getMaterial()->getDynamicFriction();
}

void TW_CALL SetRestitution(const void *value, void *clientData)
{
	static_cast<CCollider *>(clientData)->getMaterial()->setRestitution(*static_cast<const float *>(value));
}
void TW_CALL GetRestitution(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<CCollider *>(clientData)->getMaterial()->getRestitution();
}

// RIGIDBODY
void TW_CALL SetMass(const void *value, void *clientData)
{
	PxRigidBodyExt::setMassAndUpdateInertia(*static_cast<TCompRigidBody *>(clientData)->rigidBody, *static_cast<const float *>(value), NULL, false);
}
void TW_CALL GetMass(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TCompRigidBody *>(clientData)->rigidBody->getMass();
}

void TW_CALL SetGravity(const void *value, void *clientData)
{
	static_cast<TCompRigidBody *>(clientData)->setUseGravity(*static_cast<const bool *>(value));
}
void TW_CALL GetGravity(void *value, void *clientData)
{
	*static_cast<bool *>(value) = static_cast<TCompRigidBody *>(clientData)->isUsingGravity();
}

void TW_CALL SetKinematic(const void *value, void *clientData)
{
	static_cast<TCompRigidBody *>(clientData)->setKinematic(*static_cast<const bool *>(value));
}
void TW_CALL GetKinematic(void *value, void *clientData)
{
	*static_cast<bool *>(value) = static_cast<TCompRigidBody *>(clientData)->isKinematic();
}

void TW_CALL SetLinearDamping(const void *value, void *clientData)
{
	static_cast<TCompRigidBody *>(clientData)->rigidBody->setLinearDamping(*static_cast<const float *>(value));
}
void TW_CALL GetLinearDamping(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TCompRigidBody *>(clientData)->rigidBody->getLinearDamping();
}

void TW_CALL SetAngularDamping(const void *value, void *clientData)
{
	static_cast<TCompRigidBody *>(clientData)->rigidBody->setAngularDamping(*static_cast<const float *>(value));
}
void TW_CALL GetAngularDamping(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TCompRigidBody *>(clientData)->rigidBody->getAngularDamping();
}

// CAMERA
void TW_CALL GetZNear(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TCompCamera *>(clientData)->getZNear();
}

void TW_CALL GetZFar(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TCompCamera *>(clientData)->getZFar();
}

void TW_CALL GetFov(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TCompCamera *>(clientData)->getFov();
}

void TW_CALL GetAspectRatio(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TCompCamera *>(clientData)->getAspectRatio();
}

// DISTANCE JINT
void TW_CALL SetDistanceJointDamping(const void *value, void *clientData)
{
	static_cast<TCompDistanceJoint *>(clientData)->joint->setDamping(*static_cast<const float *>(value));
}
void TW_CALL GetDistanceJointDamping(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TCompDistanceJoint *>(clientData)->joint->getDamping();
}

void TW_CALL SetDistanceJointStiffness(const void *value, void *clientData)
{
	static_cast<TCompDistanceJoint *>(clientData)->joint->setStiffness(*static_cast<const float *>(value));
}
void TW_CALL GetDistanceJointStiffness(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TCompDistanceJoint *>(clientData)->joint->getStiffness();
}

void TW_CALL GetDistanceJointDistance(void *value, void *clientData)
{
	*static_cast<float *>(value) = sqrt(static_cast<TCompDistanceJoint *>(clientData)->joint->getDistance());
}

// AI
void TW_CALL GetAIFSMState(void *value, void *clientData)
{
	std::string *destPtr = static_cast<std::string *>(value);
	//TwCopyStdStringToLibrary(*destPtr, static_cast<TCompAiFsmBasic *>(clientData)->m_ai_controller.GetState());
	TwCopyStdStringToLibrary(*destPtr, static_cast<TCompAiFsmBasic *>(clientData)->m_ai_controller->getCurrentNode());

}

void TW_CALL GetGrandmaState(void *value, void *clientData)
{
	std::string *destPtr = static_cast<std::string *>(value);
	//TwCopyStdStringToLibrary(*destPtr, static_cast<TCompAiFsmBasic *>(clientData)->m_ai_controller.GetState());
	TwCopyStdStringToLibrary(*destPtr, static_cast<TCompBtGrandma *>(clientData)->m_ai_controller->getCurrentNode());

}



void TW_CALL GetPlayerFSMLegsState(void *value, void *clientData)
{
	std::string *destPtr = static_cast<std::string *>(value);
	TwCopyStdStringToLibrary(*destPtr, static_cast<TCompPlayerController *>(clientData)->fsm_player_legs.getCurrentNode());

}

void TW_CALL GetPlayerFSMTorsoState(void *value, void *clientData)
{
	std::string *destPtr = static_cast<std::string *>(value);
	TwCopyStdStringToLibrary(*destPtr, static_cast<TCompPlayerController *>(clientData)->fsm_player_torso.getCurrentNode());

}


// ---------------------------- PARTICLES CALLBACKS --------------------------

void TW_CALL CallBackParticleSystemCreate(void *clientData) {
	CEntity* e = CHandle(static_cast<TCompParticleGroup *>(clientData)).getOwner();
	TCompTransform* m_trans = e->get<TCompTransform>();

	TParticleSystem ps;
	ps.h_transform = m_trans;
	ps.h_pg = CHandle(static_cast<TCompParticleGroup *>(clientData));
	ps.loadDefaultPS();
	static_cast<TCompParticleGroup *>(clientData)->particle_systems->push_back(ps);
	entity_inspector.inspectEntity(entity_inspector.getInspectedEntity());
}

void TW_CALL CallBackParticleSystemSave(void *clientData) {
	TCompParticleGroup* pg = static_cast<TCompParticleGroup *>(clientData);
	CEntity* e = CHandle(pg).getOwner();

	particle_groups_manager.updateParticleGroupFromEntity(e, pg->def_name);
	particle_groups_manager.saveToDisk();
}

void TW_CALL CallBackParticleSystemRestart(void *clientData) {
	TCompParticleGroup* pg = static_cast<TCompParticleGroup *>(clientData);
	
	pg->restart();
}

void TW_CALL CallBackParticleSystemRemove(void *clientData) {
	TParticleSystem* ps = static_cast<TParticleSystem *>(clientData);
	TCompTransform* e_transform = ps->h_transform;		
	CEntity* e = CHandle(e_transform).getOwner();
	TCompParticleGroup* pg = e->get<TCompParticleGroup>();

	pg->removeParticleSystem(ps);

	entity_inspector.inspectEntity(entity_inspector.getInspectedEntity());
}

void TW_CALL SetParticleSystemLimit(const void *value, void *clientData)
{
	static_cast<TParticleSystem *>(clientData)->changeLimit(*static_cast<const int *>(value));
}

void TW_CALL GetParticleSystemLimit(void *value, void *clientData)
{
	*static_cast<int *>(value) = static_cast<TParticleSystem *>(clientData)->getLimit();
}

void TW_CALL SetParticleSystemShape(const void *value, void *clientData)
{
	static_cast<TParticleSystem *>(clientData)->emitter_generation->shape = (*static_cast<const TParticleEmitterShape *>(value));
	entity_inspector.inspectEntity(entity_inspector.getInspectedEntity());
}

void TW_CALL GetParticleSystemShape(void *value, void *clientData)
{
	*static_cast<TParticleEmitterShape *>(value) = static_cast<TParticleSystem *>(clientData)->emitter_generation->shape;
}

void TW_CALL SetParticleRenderTexture(const void *value, void *clientData)
{	
	std::string path = "particles/" + std::string(CEntityInspector::get().particleRenderTextureListEV[*static_cast<const int *>(value)].Label);
	std::strcpy(
		static_cast<TParticleSystem *>(clientData)->renderer->texture,
		path.c_str()
		);
	 
}
void TW_CALL GetParticleRenderTexture(void *value, void *clientData)
{
	CEntityInspector &inspector = CEntityInspector::get();
	std::string name = std::string(static_cast<TParticleSystem *>(clientData)->renderer->texture);
	int m_value = 0;
	for (int i = 0; i < inspector.texture_list.size(); ++i) {
		std::string label_name = "particles/" + std::string(inspector.particleRenderTextureListEV[i].Label);
		if (name == label_name) {
			m_value = inspector.particleRenderTextureListEV[i].Value;
			break;
		}
	}
	*static_cast<int *>(value) = m_value;
}

void TW_CALL CallbackRemoveUpdaterNoise(void *clientData)
{
	SAFE_DELETE(static_cast<TParticleSystem *>(clientData)->updater_noise);
	static_cast<TParticleSystem *>(clientData)->updater_noise = nullptr;
	entity_inspector.inspectEntity(entity_inspector.getInspectedEntity());
}

void TW_CALL CallbackRemoveUpdaterColor(void *clientData)
{
	SAFE_DELETE(static_cast<TParticleSystem *>(clientData)->updater_color);
	static_cast<TParticleSystem *>(clientData)->updater_color = nullptr;
	entity_inspector.inspectEntity(entity_inspector.getInspectedEntity());
}

void TW_CALL CallbackRemoveUpdaterSize(void *clientData)
{
	SAFE_DELETE(static_cast<TParticleSystem *>(clientData)->updater_size);
	static_cast<TParticleSystem *>(clientData)->updater_size = nullptr;
	entity_inspector.inspectEntity(entity_inspector.getInspectedEntity());
}

void TW_CALL CallbackRemoveUpdaterGravity(void *clientData)
{
	SAFE_DELETE(static_cast<TParticleSystem *>(clientData)->updater_gravity);
	static_cast<TParticleSystem *>(clientData)->updater_gravity = nullptr;
	entity_inspector.inspectEntity(entity_inspector.getInspectedEntity());
}

void TW_CALL CallbackRemoveUpdaterRotation(void *clientData)
{
	SAFE_DELETE(static_cast<TParticleSystem *>(clientData)->updater_rotation);
	static_cast<TParticleSystem *>(clientData)->updater_rotation = nullptr;
	entity_inspector.inspectEntity(entity_inspector.getInspectedEntity());
}

void TW_CALL CallbackRemoveUpdaterMovement(void *clientData)
{
	SAFE_DELETE(static_cast<TParticleSystem *>(clientData)->updater_movement);
	static_cast<TParticleSystem *>(clientData)->updater_movement = nullptr;
	entity_inspector.inspectEntity(entity_inspector.getInspectedEntity());
}

void TW_CALL CallbackRemoveUpdaterLifetime(void *clientData)
{
	SAFE_DELETE(static_cast<TParticleSystem *>(clientData)->updater_lifetime);
	static_cast<TParticleSystem *>(clientData)->updater_lifetime = nullptr;
	entity_inspector.inspectEntity(entity_inspector.getInspectedEntity());
}

void TW_CALL CallbackRemoveSubemitter(void *clientData)
{
	SAFE_DELETE(static_cast<TParticleSystem *>(clientData)->subemitter);
	static_cast<TParticleSystem *>(clientData)->subemitter = nullptr;
	entity_inspector.inspectEntity(entity_inspector.getInspectedEntity());
}

void TW_CALL CallbackAddUpdaterNoise(void *clientData)
{
	static_cast<TParticleSystem *>(clientData)->updater_noise = new TParticleUpdaterNoise(XMVectorSet(-0.1f, -0.1f, -0.1f, 0), XMVectorSet(0.1f, 0.1f, 0.1f, 0));
	entity_inspector.inspectEntity(entity_inspector.getInspectedEntity());
}

void TW_CALL CallbackAddUpdaterColor(void *clientData)
{
	static_cast<TParticleSystem *>(clientData)->updater_color = new TParticleUpdaterColor();
	entity_inspector.inspectEntity(entity_inspector.getInspectedEntity());
}

void TW_CALL CallbackAddUpdaterSize(void *clientData)
{
	static_cast<TParticleSystem *>(clientData)->updater_size = new TParticleUpdaterSize(1, 2);
	entity_inspector.inspectEntity(entity_inspector.getInspectedEntity());
}

void TW_CALL CallbackAddUpdaterGravity(void *clientData)
{
	static_cast<TParticleSystem *>(clientData)->updater_gravity = new TParticleUpdaterGravity(0.001f, false);
	entity_inspector.inspectEntity(entity_inspector.getInspectedEntity());
}

void TW_CALL CallbackAddUpdaterMovement(void *clientData)
{
	static_cast<TParticleSystem *>(clientData)->updater_movement = new TParticleUpdaterMovement();
	entity_inspector.inspectEntity(entity_inspector.getInspectedEntity());
}

void TW_CALL CallbackAddUpdaterRotation(void *clientData)
{
	static_cast<TParticleSystem *>(clientData)->updater_rotation = new TParticleUpdaterRotation(0.01f);
	entity_inspector.inspectEntity(entity_inspector.getInspectedEntity());
}

void TW_CALL CallbackAddUpdaterLifetime(void *clientData)
{
	static_cast<TParticleSystem *>(clientData)->updater_lifetime = new TParticleUpdaterLifeTime();
	entity_inspector.inspectEntity(entity_inspector.getInspectedEntity());
}

void TW_CALL CallbackAddSubemitter(void *clientData)
{
	static_cast<TParticleSystem *>(clientData)->subemitter = new TParticleSubemitter();
	entity_inspector.inspectEntity(entity_inspector.getInspectedEntity());
}

// ---------------------------- ADD COMPONENT CALLBACKS --------------------------
void TW_CALL AddTransform(void *clientData) {

	TCompTransform* t = CHandle::create<TCompTransform>();
	static_cast<CEntity *>(clientData)->add(t);
	entity_inspector.inspectEntity(static_cast<CEntity *>(clientData));
}

void TW_CALL AddDirectionalLight(void *clientData) {

	TCompDirectionalLight* l = CHandle::create<TCompDirectionalLight>();
	l->color = XMVectorSet(1, 1, 1, 0.5f);
	l->intensity = 1;
	static_cast<CEntity *>(clientData)->add(l);
	entity_inspector.inspectEntity(static_cast<CEntity *>(clientData));
}

void TW_CALL AddPointLight(void *clientData) {
	// Check if the entity has a Transform, then add the light to the position of the entity
	// Else, create the light in the origin
	TCompPointLight* l = CHandle::create<TCompPointLight>();
	l->color = XMVectorSet(1, 1, 1, 0.5f);
	l->radius = 10;
	l->intensity = 1;
	static_cast<CEntity *>(clientData)->add(l);
	entity_inspector.inspectEntity(static_cast<CEntity *>(clientData));
}



void CEntityInspector::update() {	
	if (!CHandle(target_entity).isValid()) {
		TwRemoveAllVars(bar);
		inspectEntity(CHandle());		
		return;
	}

	TCompAABB* e_aabb = ((CEntity*)target_entity)->get<TCompAABB>();
	TCompRigidBody* e_rigidbody = ((CEntity*)target_entity)->get<TCompRigidBody>();

	if (e_aabb) {
		center = e_aabb->getCenter();
		extents = e_aabb->getExtents();
		size = e_aabb->getSize();
	}
	if (e_rigidbody) {
		linearVelocity = e_rigidbody->rigidBody->getLinearVelocity();
		angularVelocity = e_rigidbody->rigidBody->getAngularVelocity();
	}
}

void CEntityInspector::inspectEntity(CHandle the_entity) {
	target_entity = the_entity;
	TwRemoveAllVars(bar);

	if (!target_entity.isValid())
		return;

	CEntity* inspectedEntity = target_entity;

	// Basic
	TCompTransform* e_transform = inspectedEntity->get<TCompTransform>();
	TCompName* e_name = inspectedEntity->get<TCompName>();
	TCompAABB* e_aabb = inspectedEntity->get<TCompAABB>();
	TCompLife* e_life = inspectedEntity->get<TCompLife>();

	// Render	
	TCompRender* e_render = inspectedEntity->get<TCompRender>();
	TCompCamera* e_camera = inspectedEntity->get<TCompCamera>();
	TCompDirectionalLight* e_directional_light = inspectedEntity->get<TCompDirectionalLight>();
	TCompAmbientLight* e_ambient_light = inspectedEntity->get<TCompAmbientLight>();
	TCompPointLight* e_point_light = inspectedEntity->get<TCompPointLight>();

	// Animation
	TCompSkeleton* e_skel = inspectedEntity->get<TCompSkeleton>();
	TCompSkeletonIK* e_skel_ik = inspectedEntity->get<TCompSkeletonIK>();

	// Physics	
	TCompColliderBox* e_collider_box = inspectedEntity->get<TCompColliderBox>();
	TCompColliderMesh* e_collider_mesh = inspectedEntity->get<TCompColliderMesh>();
	TCompColliderSphere* e_collider_sphere = inspectedEntity->get<TCompColliderSphere>();
	TCompColliderCapsule* e_collider_capsule = inspectedEntity->get<TCompColliderCapsule>();
	TCompRigidBody* e_rigidbody = inspectedEntity->get<TCompRigidBody>();
	TCompDistanceJoint* e_distance_joint = inspectedEntity->get<TCompDistanceJoint>();

	// Misc
	TCompRope* e_rope = inspectedEntity->get<TCompRope>();
	TCompNeedle* e_needle = inspectedEntity->get<TCompNeedle>();

	// Controllers
	TCompPlayerController* e_player_controller = inspectedEntity->get<TCompPlayerController>();
	TCompPlayerPivotController* e_player_pivot_controller = inspectedEntity->get<TCompPlayerPivotController>();
	TCompCameraPivotController* e_camera_pivot_controller = inspectedEntity->get<TCompCameraPivotController>();
	TCompThirdPersonCameraController* e_third_person_camera_controller = inspectedEntity->get<TCompThirdPersonCameraController>();
	
	// AI
	// Player
	TCompAiFsmBasic* e_comp_ai_fsm_basic = inspectedEntity->get<TCompAiFsmBasic>();

	// Enemies
	TCompBtGrandma* e_comp_bt_grandma = inspectedEntity->get<TCompBtGrandma>();


	// Logic
	TCompTrigger* e_trigger = inspectedEntity->get<TCompTrigger>();

	// Particle group
	TCompParticleGroup* e_particle_group = inspectedEntity->get<TCompParticleGroup>();

	TwAddVarRW(bar, "ETag", TW_TYPE_CSSTRING(sizeof(inspectedEntity->tag)), &inspectedEntity->tag, " label='Tag'");

	if (e_name) {
		TwAddVarRW(bar, "NActive", TW_TYPE_BOOL8, &e_name->active, " group=Name label='Active'");
		TwAddVarRW(bar, "CName", TW_TYPE_CSSTRING(sizeof(e_name->name)), e_name->name, " group=Name label='Name'");
		TwAddSeparator(bar, "Name", "");
	}
	if (e_transform) {
		TwAddVarRW(bar, "TActive", TW_TYPE_BOOL8, &e_transform->active, " group=Transform label='Active'");
		TwAddVarRW(bar, "Position", TW_TYPE_DIR3F, &e_transform->position, " group=Transform axisx=-x axisz=-z");
		TwAddVarRW(bar, "Rotation", TW_TYPE_QUAT4F, &e_transform->rotation, " group=Transform axisx=-x axisz=-z");
		// Yaw Pitch Roll
		TwAddVarCB(bar, "Pich", TW_TYPE_FLOAT, NULL, GetPitch, e_transform, " group=Transform");
		TwAddVarRW(bar, "Scale", TW_TYPE_DIR3F, &e_transform->scale, " group=Transform axisx=-x axisz=-z");
		TwAddSeparator(bar, "Transform", "");
	}
	if (e_life) {
		TwAddVarRW(bar, "LifeActive", TW_TYPE_BOOL8, &e_life->active, " group=Life label='Active'");
		TwAddVarRW(bar, "LifeLife", TW_TYPE_FLOAT, &e_life->life, " group=Life");
	}
	if (e_render) {
		TwAddVarRW(bar, "RenderActive", TW_TYPE_BOOL8, &e_render->active, " group=Render label='Active'");
		TwAddVarRW(bar, "RenderEmissiveOn", TW_TYPE_BOOL8, &e_render->emissive_on, " group=Render label='Emissive on'");
		std::string aux = "";
		for (int i = 0; i < e_render->keys.size(); ++i) {
			aux = "RenderMesh" + i;
			TwAddVarRO(bar, aux.c_str(), TW_TYPE_STDSTRING, &e_render->keys[i].meshName, " group=Render label='Mesh'");
			aux = "RenderMat" + i;
			TwAddVarRO(bar, aux.c_str(), TW_TYPE_STDSTRING, &e_render->keys[i].matName, " group=Render label='Material'");
			aux = "RenderGroupID" + i;
			TwAddVarRO(bar, aux.c_str(), TW_TYPE_INT32, &e_render->keys[i].subMeshId, " group=Render label='Subgroup ID'");

			if (i < e_render->keys.size() - 1)
				TwAddSeparator(bar, "", "group=Render");
		}
	}
	if (e_collider_box) {
		TwAddVarRW(bar, "CBoxActive", TW_TYPE_BOOL8, &e_collider_box->active, " group='Box Collider' label='Active'");
		TwAddVarCB(bar, "CBoxStatic friction", TW_TYPE_FLOAT, SetStaticFriction, GetStaticFriction, e_collider_box, " min=0 max=1 step=0.1 group='Box Collider'");
		TwAddVarCB(bar, "CBoxDynamic friction", TW_TYPE_FLOAT, SetDynamicFriction, GetDynamicFriction, e_collider_box, " min=0 max=1 step=0.1 group='Box Collider'");
		TwAddVarCB(bar, "CBoxRestitution", TW_TYPE_FLOAT, SetRestitution, GetRestitution, e_collider_box, " min=0 max=1 step=0.1 group='Box Collider'");
	}
	if (e_collider_mesh) {
		TwAddVarRW(bar, "CMeshActive", TW_TYPE_BOOL8, &e_collider_mesh->active, " group='Mesh Collider' label='Active'");
		TwAddVarRO(bar, "CMeshPath", TW_TYPE_CSSTRING(sizeof(e_collider_mesh->path)), &e_collider_mesh->path, " group='Mesh Collider' label='Path'");
		TwAddVarCB(bar, "CMeshStatic friction", TW_TYPE_FLOAT, SetStaticFriction, GetStaticFriction, e_collider_mesh, " min=0 max=1 step=0.1 group='Mesh Collider'");
		TwAddVarCB(bar, "CMeshDynamic friction", TW_TYPE_FLOAT, SetDynamicFriction, GetDynamicFriction, e_collider_mesh, " min=0 max=1 step=0.1 group='Mesh Collider'");
		TwAddVarCB(bar, "CMeshRestitution", TW_TYPE_FLOAT, SetRestitution, GetRestitution, e_collider_mesh, " min=0 max=1 step=0.1 group='Mesh Collider'");
	}
	if (e_collider_sphere) {
		TwAddVarRW(bar, "CSphereActive", TW_TYPE_BOOL8, &e_collider_sphere->active, " group='Sphere Collider' label='Active'");
		TwAddVarCB(bar, "CSphereStatic friction", TW_TYPE_FLOAT, SetStaticFriction, GetStaticFriction, e_collider_sphere, " min=0 max=1 step=0.1 group='Sphere Collider'");
		TwAddVarCB(bar, "CSphereDynamic friction", TW_TYPE_FLOAT, SetDynamicFriction, GetDynamicFriction, e_collider_sphere, " min=0 max=1 step=0.1 group='Sphere Collider'");
		TwAddVarCB(bar, "CSphereRestitution", TW_TYPE_FLOAT, SetRestitution, GetRestitution, e_collider_sphere, " min=0 max=1 step=0.1 group='Sphere Collider'");
	}
	if (e_collider_capsule) {
		TwAddVarRW(bar, "CCapsuleActive", TW_TYPE_BOOL8, &e_collider_capsule->active, " group='Capsule Collider' label='Active'");
		TwAddVarCB(bar, "CCapsuleStatic friction", TW_TYPE_FLOAT, SetStaticFriction, GetStaticFriction, e_collider_capsule, " min=0 max=1 step=0.1 group='Capsule Collider'");
		TwAddVarCB(bar, "CCapsuleDynamic friction", TW_TYPE_FLOAT, SetDynamicFriction, GetDynamicFriction, e_collider_capsule, " min=0 max=1 step=0.1 group='Capsule Collider'");
		TwAddVarCB(bar, "CCapsuleRestitution", TW_TYPE_FLOAT, SetRestitution, GetRestitution, e_collider_capsule, " min=0 max=1 step=0.1 group='Capsule Collider'");
	}
	if (e_rigidbody) {
		TwAddVarRW(bar, "RActive", TW_TYPE_BOOL8, &e_rigidbody->active, " group=Rigidbody label='Active'");
		TwAddVarCB(bar, "Mass", TW_TYPE_FLOAT, SetMass, GetMass, e_rigidbody, " min=0.1 group=Rigidbody");
		TwAddVarCB(bar, "Damping", TW_TYPE_FLOAT, SetLinearDamping, GetLinearDamping, e_rigidbody, " min=0 group=Rigidbody");
		TwAddVarCB(bar, "Angular Damping", TW_TYPE_FLOAT, SetAngularDamping, GetAngularDamping, e_rigidbody, " min=0 group=Rigidbody");
		TwAddVarCB(bar, "Use Gravity", TW_TYPE_BOOL8, SetGravity, GetGravity, e_rigidbody, " group=Rigidbody");
		TwAddVarCB(bar, "Is Kinematic", TW_TYPE_BOOL8, SetKinematic, GetKinematic, e_rigidbody, " group=Rigidbody");
		TwAddVarRO(bar, "Linear velocity", TW_TYPE_DIR3F, &linearVelocity, " group=Rigidbody axisx=-x axisz=-z");
		TwAddVarRO(bar, "Angular velocity", TW_TYPE_DIR3F, &angularVelocity, " group=Rigidbody axisx=-x axisz=-z");
	}	
	if (e_aabb) {
		TwAddVarRW(bar, "AABBActive", TW_TYPE_BOOL8, &e_aabb->active, " group=AABB label='Active'");
		TwAddVarRW(bar, "Min", TW_TYPE_DIR3F, &e_aabb->min, " group=AABB axisx=-x axisz=-z");
		TwAddVarRW(bar, "Max", TW_TYPE_DIR3F, &e_aabb->max, " group=AABB axisx=-x axisz=-z");
		TwAddVarRO(bar, "Center", TW_TYPE_DIR3F, &center, " group=AABB axisx=-x axisz=-z");
		TwAddVarRO(bar, "Extents", TW_TYPE_DIR3F, &extents, " group=AABB axisx=-x axisz=-z");
		TwAddVarRO(bar, "Size", TW_TYPE_DIR3F, &size, " group=AABB axisx=-x axisz=-z");
	}

	if (e_camera) {
		TwAddVarRW(bar, "CameraActive", TW_TYPE_BOOL8, &e_camera->active, " group=Camera label='Active'");
		/*TwAddVarCB(bar, "Fov", TW_TYPE_STDSTRING, NULL, GetFov, e_camera, " group=Camera min=15 max=180");
		TwAddVarCB(bar, "Aspect ratio", TW_TYPE_STDSTRING, NULL, GetAspectRatio, e_camera, " group=Camera");
		TwAddVarCB(bar, "Znear", TW_TYPE_STDSTRING, NULL, GetZNear, e_camera, " group=Camera min=0.1");
		TwAddVarCB(bar, "Zfar", TW_TYPE_STDSTRING, NULL, GetZFar, e_camera, " group=Camera");*/
	}

	if (e_player_controller) {
		TwAddVarRW(bar, "PlayerControllerActive", TW_TYPE_BOOL8, &e_player_controller->active, " group='Player Controller' label='Active'");
		TwAddVarCB(bar, "PlayerControllerLegsState", TW_TYPE_STDSTRING, NULL, GetPlayerFSMLegsState, e_player_controller, " group='Player Controller' label='Legs State'");
		TwAddVarCB(bar, "PlayerControllerTorsoState", TW_TYPE_STDSTRING, NULL, GetPlayerFSMTorsoState, e_player_controller, " group='Player Controller' label='Torso State'");
	}

	if (e_player_pivot_controller) {
		TwAddVarRW(bar, "PlayerPivotControllerActive", TW_TYPE_BOOL8, &e_player_pivot_controller->active, " group='Player Pivot Controller' label='Active' ");
		TwAddVarRW(bar, "PlayerPivotRotation velocity", TW_TYPE_FLOAT, &e_player_pivot_controller->rotation_velocity, " group='Player Pivot Controller' label='Rotation velocity'");
	}

	if (e_camera_pivot_controller) {
		TwAddVarRW(bar, "CameraPivotControllerActive", TW_TYPE_BOOL8, &e_camera_pivot_controller->active, " group='Camera Pivot Controller' label='Active' ");
		TwAddVarRW(bar, "CameraPivotControllerOffset", TW_TYPE_DIR3F, &e_camera_pivot_controller->offset, " group='Camera Pivot Controller' label='Offset' axisx=-x axisz=-z");
		TwAddVarRW(bar, "Tilt velocity", TW_TYPE_FLOAT, &e_camera_pivot_controller->tilt_velocity, " group='Camera Pivot Controller' label='Tilt velocity'");
		TwAddVarRW(bar, "Min tilt", TW_TYPE_FLOAT, &e_camera_pivot_controller->min_tilt, " group='Camera Pivot Controller' label='Min tilt'");
		TwAddVarRW(bar, "Max tilt", TW_TYPE_FLOAT, &e_camera_pivot_controller->max_tilt, " group='Camera Pivot Controller' label='Max tilt'");
	}

	if (e_third_person_camera_controller) {
		TwAddVarRW(bar, "ThirdPersonCameraControllerActive", TW_TYPE_BOOL8, &e_third_person_camera_controller->active, " group='Third Person Camera Controller' label='Active' ");
		TwAddVarRW(bar, "ThirdPersonCameraControllerMediumShot", TW_TYPE_BOOL8, &e_third_person_camera_controller->medium_shot, " group='Third Person Camera Controller' label='Medium shot' ");
		TwAddVarRW(bar, "ThirdPersonCameraControllerLongShot", TW_TYPE_BOOL8, &e_third_person_camera_controller->long_shot, " group='Third Person Camera Controller' label='Long shot' ");
		TwAddVarRW(bar, "ThirdPersonCameraControllerOffset", TW_TYPE_DIR3F, &e_third_person_camera_controller->offset, " group='Third Person Camera Controller' label='Offset' axisx=-x axisz=-z");
	}
	if (e_directional_light) {
		TwAddVarRW(bar, "DirectionalLightActive", TW_TYPE_BOOL8, &e_directional_light->active, " group='Directional Light' label='Active' ");
		TwAddVarRW(bar, "DirectionalLightIntensity", TW_TYPE_FLOAT, &e_directional_light->intensity, " min=0 max=10 group='Directional Light' label='Intensity' ");
		TwAddVarRW(bar, "DirectionalLightColor", TW_TYPE_COLOR3F, &e_directional_light->color, " group='Directional Light' label='Color' ");		
	}
	if (e_ambient_light) {
		TwAddVarRW(bar, "AmbientLightActive", TW_TYPE_BOOL8, &e_ambient_light->active, " group='Ambient Light' label='Active' ");
		TwAddVarRW(bar, "AmbientLightColor", TW_TYPE_COLOR4F, &e_ambient_light->color, " group='Ambient Light' label='Color' ");
	}
	if (e_point_light) {
		TwAddVarRW(bar, "PointLightActive", TW_TYPE_BOOL8, &e_point_light->active, " group='Point Light' label='Active' ");
		TwAddVarRW(bar, "PointLightIntensity", TW_TYPE_FLOAT, &e_point_light->intensity, " min=0 max=100 group='Point Light' label='Intensity' ");
		TwAddVarRW(bar, "PointLightColor", TW_TYPE_COLOR3F, &e_point_light->color, " group='Point Light' label='Color' ");
		TwAddVarRW(bar, "PointLightRadius", TW_TYPE_FLOAT, &e_point_light->radius, " group='Point Light' label='Radius' min=0.1");
	}
	if (e_comp_ai_fsm_basic) {
		TwAddVarCB(bar, "AIFSMBasicState", TW_TYPE_STDSTRING, NULL, GetAIFSMState, e_comp_ai_fsm_basic, " group='AI FSM Basic' label='State'");
	}
	if (e_comp_bt_grandma) {
		TwAddVarCB(bar, "BtGrandmaState", TW_TYPE_STDSTRING, NULL, GetGrandmaState, e_comp_bt_grandma, " group='Bt grandma' label='State'");
	}
	if (e_distance_joint) {
		TwAddVarRW(bar, "DistanceJointActive", TW_TYPE_BOOL8, &e_distance_joint->active, " group='Distance Joint' label='Active' ");
		TwAddVarCB(bar, "DistanceJointDamping", TW_TYPE_FLOAT, SetDistanceJointDamping, GetDistanceJointDamping, e_distance_joint, " group='Distance Joint' label='Damping'");
		TwAddVarCB(bar, "DistanceJointStiffness", TW_TYPE_FLOAT, SetDistanceJointStiffness, GetDistanceJointStiffness, e_distance_joint, " group='Distance Joint' label='Stiffness'");
		TwAddVarCB(bar, "DistanceJointDistance", TW_TYPE_FLOAT, NULL, GetDistanceJointDistance, e_distance_joint, " group='Distance Joint' label='Distance'");
	}
	if (e_rope) {
		TwAddVarRW(bar, "RopeActive", TW_TYPE_BOOL8, &e_rope->active, " group='Rope' label='Active' ");
		TwAddVarRW(bar, "RopeWidth", TW_TYPE_FLOAT, &e_rope->width, " min=0.01 max=0.5 step=0.01 group='Rope' label='Width' ");
		TwAddVarRW(bar, "RopeMaxDistance", TW_TYPE_FLOAT, &e_rope->max_distance, " min=1 max=1000 step=0.5 group='Rope' label='Max distance' ");
	}
	if (e_needle) {
		TwAddVarRW(bar, "NeedleActive", TW_TYPE_BOOL8, &e_needle->active, " group='Needle' label='Active' ");
	}
	if (e_trigger) {
		TwAddVarRW(bar, "TriggerActive", TW_TYPE_BOOL8, &e_trigger->active, " group='Trigger' label='Active' ");
	}
	if (e_skel) {
		TwAddVarRW(bar, "SkeletonActive", TW_TYPE_BOOL8, &e_skel->active, " group='Skeleton' label='Active' ");

		std::string aux = "";
		std::string aux2 = "";

		// Blend times		
		for (int i = 0; i < e_skel->getCCoreModel()->animation_names.size(); ++i) {
			aux = "SkeletonAnim" + i;
			aux2 = "group=Skeleton label=' " + e_skel->getCCoreModel()->animation_names[i] + "'";
			TwAddButton(bar, aux.c_str(), NULL, NULL, aux2.c_str());
			aux = "SkeletonAnimBlendIn" + i;
			TwAddVarRW(bar, aux.c_str(), TW_TYPE_FLOAT, &e_skel->getCCoreModel()->animation_blend_times[i], "group='Skeleton' label='Blend in'");
			aux = "SkeletonAnimBlendOut" + i;
			TwAddVarRW(bar, aux.c_str(), TW_TYPE_FLOAT, &e_skel->getCCoreModel()->animation_blend_out_times[i], "group='Skeleton' label='Blend out'");
		}
	}
	if (e_skel_ik) {
		TwAddVarRW(bar, "SkeletonIKActive", TW_TYPE_BOOL8, &e_skel_ik->active, " group='Skeleton IK' label='Active' ");
	}

	if (e_particle_group) {
		TwAddVarRW(bar, "PGActive", TW_TYPE_BOOL8, &e_particle_group->active, " group=PG label='Active'");		
		std::string aux = "";
		std::string aux2 = "";

		TwAddButton(bar, "PGAddBtn", CallBackParticleSystemCreate, e_particle_group, "group=PG label='Add particle system'");
		TwAddButton(bar, "PGSaveBtn", CallBackParticleSystemSave, e_particle_group, "group=PG label='Save particle group'");
		TwAddButton(bar, "PGRestartBtn", CallBackParticleSystemRestart, e_particle_group, "group=PG label='Restart particle group'");

		// For each particle system
		for (int i = 0; i < e_particle_group->particle_systems->size(); ++i) {

			if ((*e_particle_group->particle_systems)[i].dirty_destroy_group) { continue; }
			
			TwAddSeparator(bar, "", "group=PG");
			aux = "ParticleSystem" + i;
			aux2 = "group=PG label='PARTICLE SYSTEM " + std::to_string(i) + "'";
			TwAddButton(bar, aux.c_str(), NULL, NULL, aux2.c_str());
			aux = "ParticleSystemRemove" + i;
			TwAddButton(bar, aux.c_str(), CallBackParticleSystemRemove, &(*e_particle_group->particle_systems)[i], "group=PG label='Remove'");
			TwAddSeparator(bar, "", "group=PG");			

			// Emitter
			aux = "Emitter" + i;
			TwAddButton(bar, aux.c_str(), NULL, NULL, "group=PG label='Emitter'");
			aux = "PGEmitterShape" + i;
			TwAddVarCB(bar, aux.c_str(), particleEmitterShape, SetParticleSystemShape, GetParticleSystemShape, &(*e_particle_group->particle_systems)[i], " group=PG label='Shape'");

			TParticleEmitterShape m_shape = (*e_particle_group->particle_systems)[i].emitter_generation->shape;

			if (m_shape == TParticleEmitterShape::SPHERE || m_shape == TParticleEmitterShape::SEMISPHERE || m_shape == TParticleEmitterShape::CONE || m_shape == TParticleEmitterShape::RING) {
				aux = "PGEmitterRadius" + i;
				TwAddVarRW(bar, aux.c_str(), TW_TYPE_FLOAT, &(*e_particle_group->particle_systems)[i].emitter_generation->radius, " group=PG label='Radius' min=0.01 step=0.05");
			}
			if (m_shape == TParticleEmitterShape::CONE) {
				aux = "PGEmitterAngle" + i;
				TwAddVarRW(bar, aux.c_str(), TW_TYPE_FLOAT, &(*e_particle_group->particle_systems)[i].emitter_generation->angle, " group=PG label='Angle' min=0.01 step=0.01");
			}
			if (m_shape == TParticleEmitterShape::BOX) {
				aux = "PGEmitterBoxSize" + i;
				TwAddVarRW(bar, aux.c_str(), TW_TYPE_FLOAT, &(*e_particle_group->particle_systems)[i].emitter_generation->box_size, " group=PG label='Box Size' min=0.01 step=0.05");
			}
			if (m_shape == TParticleEmitterShape::RING) {
				aux = "PGEmitterInnerRadius" + i;
				TwAddVarRW(bar, aux.c_str(), TW_TYPE_FLOAT, &(*e_particle_group->particle_systems)[i].emitter_generation->inner_radius, " group=PG label='Inner Radius' min=0.01 step=0.05");
			}
			aux = "PGEmitterLimit" + i;
			//TwAddVarRO(bar, aux.c_str(), TW_TYPE_INT32, &e_particle_group->particle_systems[i].emitter_generation->limit, " group=PG label='Limit'");
			TwAddVarCB(bar, aux.c_str(), TW_TYPE_INT32, SetParticleSystemLimit, GetParticleSystemLimit, &(*e_particle_group->particle_systems)[i], " group=PG label='Limit' min=1");
			aux = "PGEmitterRate" + i;
			TwAddVarRW(bar, aux.c_str(), TW_TYPE_FLOAT, &(*e_particle_group->particle_systems)[i].emitter_generation->rate, " group=PG label='Rate' min=0.0001 step=0.01");
			aux = "PGEmitterMinLT" + i;
			TwAddVarRW(bar, aux.c_str(), TW_TYPE_FLOAT, &(*e_particle_group->particle_systems)[i].emitter_generation->min_life_time, " group=PG label='Min lifeTime' min=0 step=0.1");
			aux = "PGEmitterMaxLT" + i;
			TwAddVarRW(bar, aux.c_str(), TW_TYPE_FLOAT, &(*e_particle_group->particle_systems)[i].emitter_generation->max_life_time, " group=PG label='Max lifeTime' min=0 step=0.1");
			aux = "PGEmitterBurstTime" + i;
			TwAddVarRW(bar, aux.c_str(), TW_TYPE_FLOAT, &(*e_particle_group->particle_systems)[i].emitter_generation->burst_time, " group=PG label='Burst Time' min=0 step=0.1");
			aux = "PGEmitterBurstAmount" + i;
			TwAddVarRW(bar, aux.c_str(), TW_TYPE_INT32, &(*e_particle_group->particle_systems)[i].emitter_generation->burst_amount, " group=PG label='Burst Amount' min=1");
			aux = "PGEmitterDelay" + i;
			TwAddVarRW(bar, aux.c_str(), TW_TYPE_FLOAT, &(*e_particle_group->particle_systems)[i].emitter_generation->delay, " group=PG label='Delay' min=0 step=0.01");
			aux = "PGEmitterLoop" + i;
			TwAddVarRW(bar, aux.c_str(), TW_TYPE_BOOL8, &(*e_particle_group->particle_systems)[i].emitter_generation->loop, " group=PG label='Loop'");
			aux = "PGEmitterFillInitial" + i;
			TwAddVarRW(bar, aux.c_str(), TW_TYPE_BOOL8, &(*e_particle_group->particle_systems)[i].emitter_generation->fill_initial, " group=PG label='Fill initial'");
			aux = "PGEmitterRandomRot" + i;
			TwAddVarRW(bar, aux.c_str(), TW_TYPE_BOOL8, &(*e_particle_group->particle_systems)[i].emitter_generation->random_rotation, " group=PG label='Random rotation'");

			// Updaters
			if ((*e_particle_group->particle_systems)[i].updater_lifetime != nullptr) {
				aux = "Lifetime" + i;
				TwAddButton(bar, aux.c_str(), NULL, NULL, "group=PG label='Lifetime'");
				aux = "RemoveLifetime" + i;
				TwAddButton(bar, aux.c_str(), CallbackRemoveUpdaterLifetime, &(*e_particle_group->particle_systems)[i], "group=PG label='Remove'");
			}

			if ((*e_particle_group->particle_systems)[i].updater_color != nullptr) {
				aux = "Color over life" + i;
				TwAddButton(bar, aux.c_str(), NULL, NULL, "group=PG label='Color over life'");
				aux = "PGUpdaterColorIC" + i;
				TwAddVarRW(bar, aux.c_str(), TW_TYPE_COLOR3F, &(*e_particle_group->particle_systems)[i].updater_color->initial_color, " group=PG label='Initial Color'");
				aux = "PGUpdaterColorFC" + i;
				TwAddVarRW(bar, aux.c_str(), TW_TYPE_COLOR3F, &(*e_particle_group->particle_systems)[i].updater_color->final_color, " group=PG label='Final Color'");
				aux = "PGUpdaterColorCurve" + i;
				TwAddVarRW(bar, aux.c_str(), particleCurve, &(*e_particle_group->particle_systems)[i].updater_color->curve, " group=PG label='Curve'");
				aux = "PGUpdaterColorCurveVal" + i;
				TwAddVarRW(bar, aux.c_str(), TW_TYPE_FLOAT, &(*e_particle_group->particle_systems)[i].updater_color->curve_val, " group=PG label='Curve value' min=0.01 step=0.01");
				aux = "RemovePGUpdaterColor" + i;
				TwAddButton(bar, aux.c_str(), CallbackRemoveUpdaterColor, &(*e_particle_group->particle_systems)[i], "group=PG label='Remove'");
			}

			if ((*e_particle_group->particle_systems)[i].updater_size != nullptr) {
				aux = "Size over life" + i;
				TwAddButton(bar, aux.c_str(), NULL, NULL, "group=PG label='Size over life'");
				aux = "PGUpdaterSizeIS" + i;
				TwAddVarRW(bar, aux.c_str(), TW_TYPE_FLOAT, &(*e_particle_group->particle_systems)[i].updater_size->initial_size, " group=PG label='Initial Size' min=0.01 step=0.01");
				aux = "PGUpdaterSizeFS" + i;
				TwAddVarRW(bar, aux.c_str(), TW_TYPE_FLOAT, &(*e_particle_group->particle_systems)[i].updater_size->final_size, " group=PG label='Final Size' min=0.01 step=0.01");
				aux = "RemovePGUpdaterSize" + i;
				TwAddButton(bar, aux.c_str(), CallbackRemoveUpdaterSize, &(*e_particle_group->particle_systems)[i], "group=PG label='Remove'");
			}			

			if ((*e_particle_group->particle_systems)[i].updater_movement != nullptr) {
				aux = "Initial movement" + i;
				TwAddButton(bar, aux.c_str(), NULL, NULL, "group=PG label='Initial movement'");
				aux = "PGUpdaterMovementSpeed" + i;
				TwAddVarRW(bar, aux.c_str(), TW_TYPE_FLOAT, &(*e_particle_group->particle_systems)[i].updater_movement->speed, " group=PG label='Initial Speed' step=0.1");
				aux = "RemovePGUpdaterMovement" + i;
				TwAddButton(bar, aux.c_str(), CallbackRemoveUpdaterMovement, &(*e_particle_group->particle_systems)[i], "group=PG label='Remove'");
			}

			if ((*e_particle_group->particle_systems)[i].updater_rotation != nullptr) {
				aux = "PGRotation" + i;
				TwAddButton(bar, aux.c_str(), NULL, NULL, "group=PG label='Rotation'");
				aux = "PGUpdaterRotationSpeed" + i;
				TwAddVarRW(bar, aux.c_str(), TW_TYPE_FLOAT, &(*e_particle_group->particle_systems)[i].updater_rotation->angular_speed, " group=PG label='Speed' step=0.005");
				aux = "RemovePGUpdaterRotation" + i;
				TwAddButton(bar, aux.c_str(), CallbackRemoveUpdaterRotation, &(*e_particle_group->particle_systems)[i], "group=PG label='Remove'");
			}

			if ((*e_particle_group->particle_systems)[i].updater_noise != nullptr) {
				aux = "Random speed over time" + i;
				TwAddButton(bar, aux.c_str(), NULL, NULL, "group=PG label='Random speed over time'");
				aux = "PGUpdaterNoiseMinNoise" + i;
				TwAddVarRW(bar, aux.c_str(), TW_TYPE_DIR3F, &(*e_particle_group->particle_systems)[i].updater_noise->min_noise, " group=PG label='Min Speed' min=0.01 step=0.01");
				aux = "PGUpdaterNoiseMaxNoise" + i;
				TwAddVarRW(bar, aux.c_str(), TW_TYPE_DIR3F, &(*e_particle_group->particle_systems)[i].updater_noise->max_noise, " group=PG label='Max Speed' min=0.01 step=0.01");
				aux = "RemovePGUpdaterNoise" + i;
				TwAddButton(bar, aux.c_str(), CallbackRemoveUpdaterNoise, &(*e_particle_group->particle_systems)[i], "group=PG label='Remove'");
			}
			
			if ((*e_particle_group->particle_systems)[i].updater_gravity != nullptr) {
				aux = "Gravity" + i;
				TwAddButton(bar, aux.c_str(), NULL, NULL, "group=PG label='Gravity'");
				aux = "PGUpdaterGravityForce" + i;
				TwAddVarRW(bar, aux.c_str(), TW_TYPE_FLOAT, &(*e_particle_group->particle_systems)[i].updater_gravity->gravity, " group=PG label='Force' step=0.005");
				aux = "PGUpdaterGravityConstant" + i;
				TwAddVarRW(bar, aux.c_str(), TW_TYPE_BOOL8, &(*e_particle_group->particle_systems)[i].updater_gravity->constant, " group=PG label='Constant'");
				aux = "RemovePGUpdaterGravity" + i;
				TwAddButton(bar, aux.c_str(), CallbackRemoveUpdaterGravity, &(*e_particle_group->particle_systems)[i], "group=PG label='Remove'");
			}

			// Subemitter
			if ((*e_particle_group->particle_systems)[i].subemitter != nullptr) {
				aux = "Subemitter" + i;
				TwAddButton(bar, aux.c_str(), NULL, NULL, "group=PG label='Subemitters'");
				aux = "SubemitterDeath" + i;
				TwAddVarRW(bar, aux.c_str(), TW_TYPE_CSSTRING(sizeof((*e_particle_group->particle_systems)[i].subemitter->death_emitter)), &(*e_particle_group->particle_systems)[i].subemitter->death_emitter, " group=PG label='onDeath'");
				aux = "RemovePGSubemitter" + i;
				TwAddButton(bar, aux.c_str(), CallbackRemoveSubemitter, &(*e_particle_group->particle_systems)[i], "group=PG label='Remove'");
			}

			// Renderer
			aux = "Renderer" + i;
			TwAddButton(bar, aux.c_str(), NULL, NULL, "group=PG label='Renderer'");
			
			particleRenderTextureList = TwDefineEnum("ParticleRenderTextureList", particleRenderTextureListEV, (int)texture_list.size());

			aux = "PGRendererTextureList" + i;
			TwAddVarCB(bar, aux.c_str(), particleRenderTextureList, SetParticleRenderTexture, GetParticleRenderTexture, &(*e_particle_group->particle_systems)[i], " group=PG label='Texture'");

			aux = "PGRendererAdditive" + i;
			TwAddVarRW(bar, aux.c_str(), TW_TYPE_BOOL8, &(*e_particle_group->particle_systems)[i].renderer->additive, " group=PG label='Additive'");

			aux = "PGRendererDistorsion" + i;
			TwAddVarRW(bar, aux.c_str(), TW_TYPE_BOOL8, &(*e_particle_group->particle_systems)[i].renderer->distorsion, " group=PG label='Distorsion'");

			aux = "PGRendererDistorsionAmount" + i;
			TwAddVarRW(bar, aux.c_str(), TW_TYPE_FLOAT, &(*e_particle_group->particle_systems)[i].renderer->distorsion_amount, " group=PG label='Distorsion amount' min=0 step=0.01");

			aux = "PGRendererNAnimX" + i;
			TwAddVarRW(bar, aux.c_str(), TW_TYPE_INT32, &(*e_particle_group->particle_systems)[i].renderer->n_anim_x, " group=PG label='Animation columns'");
			aux = "PGRendererNAnimY" + i;
			TwAddVarRW(bar, aux.c_str(), TW_TYPE_INT32, &(*e_particle_group->particle_systems)[i].renderer->n_anim_y, " group=PG label='Animation rows'");

			aux = "PGRendererMode" + i;
			TwAddVarRW(bar, aux.c_str(), particleRenderMode, &(*e_particle_group->particle_systems)[i].renderer->render_type, " group=PG label='Render mode'");

			aux = "PGRendererAnimMode" + i;
			TwAddVarRW(bar, aux.c_str(), particleRenderAnimationMode, &(*e_particle_group->particle_systems)[i].renderer->particle_animation_mode, " group=PG label='Animation mode'");

			// TODO: Hacer que solo aparezca en modo stretch
			aux = "PGRendererStretch" + i;
			TwAddVarRW(bar, aux.c_str(), TW_TYPE_FLOAT, &(*e_particle_group->particle_systems)[i].renderer->stretch, " group=PG label='Stretch' step=0.1");

			aux = "PGRendererStretchMode" + i;
			TwAddVarRW(bar, aux.c_str(), particleRenderStretchMode, &(*e_particle_group->particle_systems)[i].renderer->stretch_mode, " group=PG label='Stretch mode'");
						
			TwAddSeparator(bar, "", "group=PG");

			// Updaters

			if ((*e_particle_group->particle_systems)[i].updater_lifetime == nullptr) {
				aux = "AddPGLifetime" + i;
				TwAddButton(bar, aux.c_str(), CallbackAddUpdaterLifetime, &(*e_particle_group->particle_systems)[i], "group=PG label='Add lifetime updater'");
			}

			if ((*e_particle_group->particle_systems)[i].updater_color == nullptr) {
				aux = "AddPGColor" + i;
				TwAddButton(bar, aux.c_str(), CallbackAddUpdaterColor, &(*e_particle_group->particle_systems)[i], "group=PG label='Add color updater'");
			}

			if ((*e_particle_group->particle_systems)[i].updater_size == nullptr) {
				aux = "AddPGSize" + i;
				TwAddButton(bar, aux.c_str(), CallbackAddUpdaterSize, &(*e_particle_group->particle_systems)[i], "group=PG label='Add size updater'");
			}

			if ((*e_particle_group->particle_systems)[i].updater_movement == nullptr) {
				aux = "AddPGMovement" + i;
				TwAddButton(bar, aux.c_str(), CallbackAddUpdaterMovement, &(*e_particle_group->particle_systems)[i], "group=PG label='Add movement updater'");
			}

			if ((*e_particle_group->particle_systems)[i].updater_rotation == nullptr) {
				aux = "AddPGRotation" + i;
				TwAddButton(bar, aux.c_str(), CallbackAddUpdaterRotation, &(*e_particle_group->particle_systems)[i], "group=PG label='Add rotation updater'");
			}

			if ((*e_particle_group->particle_systems)[i].updater_noise == nullptr) {
				aux = "AddPGNoise" + i;
				TwAddButton(bar, aux.c_str(), CallbackAddUpdaterNoise, &(*e_particle_group->particle_systems)[i], "group=PG label='Add noise updater'");
			}

			if ((*e_particle_group->particle_systems)[i].updater_gravity == nullptr) {
				aux = "AddPGGravity" + i;
				TwAddButton(bar, aux.c_str(), CallbackAddUpdaterGravity, &(*e_particle_group->particle_systems)[i], "group=PG label='Add gravity updater'");
			}

			// Subemitter
			if ((*e_particle_group->particle_systems)[i].subemitter == nullptr) {
				aux = "AddPGSubemitter" + i;
				TwAddButton(bar, aux.c_str(), CallbackAddSubemitter, &(*e_particle_group->particle_systems)[i], "group=PG label='Add subemitters'");
			}

			/*if (i < e_particle_group->particle_systems.size() - 1) {
				TwAddSeparator(bar, "", "group=PG");
			}*/
		}
	}

	TwAddSeparator(bar, "", "");
	TwAddButton(bar, "Add components", NULL, NULL, "");

	if (!e_transform) {
		TwAddButton(bar, "Transform", AddTransform, inspectedEntity, "");
	}
	if (!e_directional_light) {
		TwAddButton(bar, "Directional Light", AddDirectionalLight, inspectedEntity, "");
	}
	if (!e_point_light) {
		TwAddButton(bar, "Point Light", AddPointLight, inspectedEntity, "");
	}
}


// ----------------------------

TwBar *lister_bar;
std::vector<std::string> component_names;

CEntityLister::CEntityLister() { searchIn = ""; }

CEntityLister::~CEntityLister() { }

static CEntityLister entity_lister;

CEntityLister& CEntityLister::get() {
	return entity_lister;
}

void TW_CALL CallbackInspectEntity(void *clientData) {
	entity_inspector.inspectEntity(static_cast<CEntity *>(clientData));
}

void TW_CALL CopyStdStringToClient(std::string& destinationClientString, const std::string& sourceLibraryString)
{
	destinationClientString = sourceLibraryString;
}

size_t LevenshteinDistance(std::string s1, std::string s2)
{
	const size_t m(s1.size());
	const size_t n(s2.size());

	if (m == 0) return n;
	if (n == 0) return m;

	size_t *costs = new size_t[n + 1];

	for (size_t k = 0; k <= n; k++) costs[k] = k;

	size_t i = 0;
	for (std::string::const_iterator it1 = s1.begin(); it1 != s1.end(); ++it1, ++i)
	{
		costs[0] = i + 1;
		size_t corner = i;

		size_t j = 0;
		for (std::string::const_iterator it2 = s2.begin(); it2 != s2.end(); ++it2, ++j)
		{
			size_t upper = costs[j + 1];
			if (*it1 == *it2)
			{
				costs[j + 1] = corner;
			}
			else
			{
				size_t t(upper<corner ? upper : corner);
				costs[j + 1] = (costs[j]<t ? costs[j] : t) + 1;
			}

			corner = upper;
		}
	}

	size_t result = costs[n];
	delete[] costs;

	return result;
}

// Compare strings for search purposes, tests if the strings are equal, are contained or similar (by Levenshtein distance)
bool searchCompare(std::string s1, std::string s2) {
	
	std::transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
	std::transform(s2.begin(), s2.end(), s2.begin(), ::tolower);

	if (s1 == s2)
		return true;

	bool contains1 = s1.find(s2) != std::string::npos;
	bool contains2 = s2.find(s1) != std::string::npos;

	if (contains1 || contains2)
		return true;

	size_t l_dist = LevenshteinDistance(s1, s2);
	if (l_dist < 4)
		return true;
	return false;
}

/* 
Predictive input. Given an string input and a list of strings, returns the string from the list which
starts with the input script, if there is only one posibility
*/ 
std::string predictiveInput(std::string input, std::vector<std::string> list) {

	std::string ret;
	int count = 0;

	for (int i = 0; i < list.size(); ++i) {
		if (list[i].find(input) == 0) {
			ret = list[i];
			count++;
		}
	}

	return count == 1 ? ret : "";
}

// Entity has componente (by name)
bool entityHasComponent(CEntity* e, std::string component_name) {
	// Input string to lower case
	std::transform(component_name.begin(), component_name.end(), component_name.begin(), ::tolower);

	// Gets the predictive input
	component_name = predictiveInput(component_name, component_names);

	if (component_name == ":name")
		return e->has<TCompName>();
	if (component_name == ":transform")
		return e->has<TCompTransform>();
	if (component_name == ":mesh")
		return e->has<TCompMesh>();
	if (component_name == ":aabb")
		return e->has<TCompAABB>();
	if (component_name == ":camera")
		return e->has<TCompCamera>();
	if (component_name == ":collider")
		return e->has<TCompColliderBox>();
	if (component_name == ":rigidbody")
		return e->has<TCompRigidBody>();
	if (component_name == ":staticbody")
		return e->has<TCompStaticBody>();
	if (component_name == ":ambientlight")
		return e->has<TCompAmbientLight>();
	if (component_name == ":directionallight")
		return e->has<TCompDirectionalLight>();
	if (component_name == ":pointlight")
		return e->has<TCompPointLight>();
	if (component_name == ":playercontroller")
		return e->has<TCompPlayerController>();
	if (component_name == ":playerpivotcontroller")
		return e->has<TCompPlayerController>();
	if (component_name == ":camerapivotcontroller")
		return e->has<TCompCameraPivotController>();
	if (component_name == ":thirdpersoncameracontroller")
		return e->has<TCompThirdPersonCameraController>();

	if (component_name == ":light") {
		return e->has<TCompAmbientLight>() || e->has<TCompDirectionalLight>() || e->has<TCompPointLight>();
	}

	return false;
}

void CEntityLister::init() {
	// Create a tewak bar
	lister_bar = TwNewBar("Lister");
	TwCopyStdStringToClientFunc(CopyStdStringToClient);
	
	CApp &app = CApp::get();

	// AntTweakBar test
	int barSize[2] = { 224, app.yres - 120 };
	int varPosition[2] = { 0, 0 };
	TwSetParam(lister_bar, NULL, "size", TW_PARAM_INT32, 2, barSize);
	TwSetParam(lister_bar, NULL, "position", TW_PARAM_INT32, 2, varPosition);
	TwDefine(" Lister label='Entity list' ");
	TwDefine(" Lister refresh='0.3' ");
	TwDefine(" TW_HELP visible=false ");

	TwAddButton(lister_bar, "Search", NULL, NULL, "");
	TwAddVarRW(lister_bar, "EntitySearchInput", TW_TYPE_STDSTRING, &searchIn, "label='Search entities'");
	TwAddSeparator(lister_bar, "", "");
	TwAddButton(lister_bar, "Entities", NULL, NULL, "");

	// Entity event counter
	m_entity_event_count = -1;

	// Previus search
	prevSearch = "";

	// Fills the component name vector
	std::string components[] = { 
		  ":name"
		, ":transform"
		, ":mesh" 
		, ":aabb"
		, ":collider"
		, ":rigidbody"
		, ":staticbody"
		, ":ambientlight"
		, ":directionallight"
		, ":pointlight"
		, ":playercontroller"
		, ":playerpivotcontroller"
		, ":camerapivotcontroller"
		, ":thirdpersoncameracontroller"
		, ":light"
	};
	component_names = std::vector<std::string>(std::begin(components), std::end(components));
}

void CEntityLister::resetEventCount() {
	m_entity_event_count = 0;
}

void CEntityLister::update() {

	// if the entities has changed
	if ((m_entity_event_count != CEntityManager::get().getEntityEventCount()) || (prevSearch != searchIn)) {
		m_entity_event_count = CEntityManager::get().getEntityEventCount();
		// Get the entities currently alive
		std::vector< CHandle > entities = CEntityManager::get().getEntities();

		TwRemoveAllVars(lister_bar);

		TwAddButton(lister_bar, "Search", NULL, NULL, "");
		TwAddVarRW(lister_bar, "EntitySearchInput", TW_TYPE_STDSTRING, &searchIn, "label='Search entities'");
		TwAddSeparator(lister_bar, "", "");
		TwAddButton(lister_bar, "Entities", NULL, NULL, "");

		// Save the search script as std string
		std::string strSearch = std::string(searchIn);

		// If the first character is ':', search by type
		bool searchByTag = strSearch.size() > 0 && strSearch[0] == ':';

		for (int i = 0; i < entities.size(); ++i) {
			// Get the entity names
			TCompName* e_name = ((CEntity*)entities[i])->get<TCompName>();
			if (e_name) {
				if (searchByTag) {
					if (entityHasComponent(((CEntity*)entities[i]), strSearch)) {
						TwAddButton(lister_bar, e_name->name, CallbackInspectEntity, (CEntity*)entities[i], "");
					}
				}
				else {
					if (strSearch.empty() || searchCompare(strSearch, e_name->name)) {						
						TwAddButton(lister_bar, e_name->name, CallbackInspectEntity, (CEntity*)entities[i], "");
					}
				}
			}
		}

		prevSearch = searchIn;
	}	
}

// ----------------------------

TwBar *actioner_bar;

CEntityActioner::CEntityActioner() {}

CEntityActioner::~CEntityActioner() { }

static CEntityActioner entity_actioner;

CEntityActioner& CEntityActioner::get() {
	return entity_actioner;
}

// AntTweakBar button test
int entityCounter = 0;
void TW_CALL CallbackCreateEntity(void *clientData)
{

	// Create a new entity with some components
	CEntity* e = CEntityManager::get().createEmptyEntity();

	TCompName* n = getObjManager<TCompName>()->createObj();
	std::strcpy(n->name, ("Nueva entidad" + std::to_string(entityCounter)).c_str());
	e->add(n);

	entityCounter++;
}

void TW_CALL CallbackDestroyEntity(void *clientData)
{
	CEntityManager::get().remove(CHandle(entity_inspector.getInspectedEntity()));
	entity_inspector.inspectEntity(CHandle());
}

void TW_CALL CallbackLoadLevel(void *clientData)
{	
	// Remove all entities
	//CEntityManager::get().clear();
	
}

void CEntityActioner::init() {
	// Create a tewak bar
	actioner_bar = TwNewBar("Actioner");

	CApp &app = CApp::get();

	// AntTweakBar test
	int barSize[2] = { 224, 224 };
	int varPosition[2] = { 0, app.yres - 120 };
	TwSetParam(actioner_bar, NULL, "size", TW_PARAM_INT32, 2, barSize);
	TwSetParam(actioner_bar, NULL, "position", TW_PARAM_INT32, 2, varPosition);
	TwDefine(" Lister label='Entity actions' ");
	TwDefine(" Lister refresh='0.3' ");

	TwAddButton(actioner_bar, "New entity", CallbackCreateEntity, NULL, "");
	TwAddButton(actioner_bar, "Destroy entity", CallbackDestroyEntity, NULL, "");
}

void CEntityActioner::update() {
}


// ------------------------------------------------------
TwBar *debug_bar;

CDebugOptioner::CDebugOptioner()
{}

CDebugOptioner::~CDebugOptioner() { }

static CDebugOptioner debug_optioner;

CDebugOptioner& CDebugOptioner::get() {
	return debug_optioner;
}

void CDebugOptioner::init() {
	// Create a tewak bar
	debug_bar = TwNewBar("DebugOptioner");

	CApp &app = CApp::get();
	CIOStatus &io = CIOStatus::get();

	// AntTweakBar test
	int barSize[2] = { 224, 120 };
	int varPosition[2] = { 240, 20 };
	TwSetParam(debug_bar, NULL, "size", TW_PARAM_INT32, 2, barSize);
	TwSetParam(debug_bar, NULL, "position", TW_PARAM_INT32, 2, varPosition);
	TwDefine(" DebugOptioner label='Debug options' ");
	TwDefine(" DebugOptioner refresh='2' ");

	TwAddVarRW(debug_bar, "Draw grid", TW_TYPE_BOOL8, &app.renderGrid, "");
	TwAddVarRW(debug_bar, "Draw axis", TW_TYPE_BOOL8, &app.renderAxis, "");
	TwAddVarRW(debug_bar, "Draw names", TW_TYPE_BOOL8, &app.renderNames, "");
	TwAddVarRW(debug_bar, "Draw AABBs", TW_TYPE_BOOL8, &app.renderAABB, "");
	TwAddVarRW(debug_bar, "Mouse Sensibility", TW_TYPE_FLOAT, &io.mouse_sensibility, "");
}

// ------------------------------------------------------

void TW_CALL ExecuteConsoleAction(const void *value, void *clientData)
{
	CLogicManager::get().execute((*(const std::string *)value));
}

void TW_CALL GetNothing(void *value, void *clientData)
{
	std::string *destPtr = static_cast<std::string *>(value);
	TwCopyStdStringToLibrary(*destPtr, "");

}

TwBar *console_bar;
TwBar *console_input_bar;

CConsole::CConsole() : num_lines(100)
{}

CConsole::~CConsole() { }

static CConsole console;

CConsole& CConsole::get() {
	return console;
}

void CConsole::init() {
	num_lines = 100;

	// Create a tewak bar
	console_bar = TwNewBar("Console");
	console_input_bar = TwNewBar("ConsoleInput");

	CApp &app = CApp::get();

	// AntTweakBar test
	int barSize[2] = { 400, 220 };
	int varPosition[2] = { 240, app.yres - 520 };
	TwSetParam(console_bar, NULL, "size", TW_PARAM_INT32, 2, barSize);
	TwSetParam(console_bar, NULL, "position", TW_PARAM_INT32, 2, varPosition);
	TwDefine(" Console label='ConsoleOutput' ");
	TwDefine(" Console refresh='2' ");

	int barSize2[2] = { 400, 100 };
	int varPosition2[2] = { 240, app.yres - 620 };
	TwSetParam(console_input_bar, NULL, "size", TW_PARAM_INT32, 2, barSize2);
	TwSetParam(console_input_bar, NULL, "position", TW_PARAM_INT32, 2, varPosition2);
	TwDefine(" ConsoleInput label='ConsoleInput' ");
	TwDefine(" ConsoleInput refresh='2' ");

	TwAddButton(console_input_bar, "Commands", NULL, NULL, "");
	TwAddVarCB(console_input_bar, "Command", TW_TYPE_STDSTRING, ExecuteConsoleAction, GetNothing, NULL, " label='Command'");
}

void CConsole::print(std::string text) {
	std::vector<std::string> text_lines = split_string(text, "\n");

	for (std::string t : text_lines) {
		if (lines.size() >= num_lines)
			lines.pop_front();
		lines.push_back(t);
	}


	for (int i = 0; i < lines.size(); ++i) {
		std::string name = "Line" + std::to_string(i);
		std::string label = "label='" + lines[i] + "'";
		TwRemoveVar(console_bar, name.c_str());
		TwAddButton(console_bar, name.c_str(), NULL, NULL, label.c_str());
	}
}



// ------------------------------------------------------
TwBar *post_process_bar;

CPostProcessOptioner::CPostProcessOptioner()
{}

CPostProcessOptioner::~CPostProcessOptioner() { }

static CPostProcessOptioner post_process_optioner;

CPostProcessOptioner& CPostProcessOptioner::get() {
	return post_process_optioner;
}

void CPostProcessOptioner::init() {
	// Create a tewak bar
	post_process_bar = TwNewBar("PostProcessOptioner");

	CApp &app = CApp::get();

	// AntTweakBar test
	int barSize[2] = { 224, 120 };
	int varPosition[2] = { 500, 20 };
	TwSetParam(post_process_bar, NULL, "size", TW_PARAM_INT32, 2, barSize);
	TwSetParam(post_process_bar, NULL, "position", TW_PARAM_INT32, 2, varPosition);
	TwDefine(" PostProcessOptioner label='Post process options' ");
	TwDefine(" PostProcessOptioner refresh='2' ");

	TwAddVarRW(post_process_bar, "Sharpen", TW_TYPE_FLOAT, &sharpen->amount, "min=0 max=10 step=0.05 ");
	TwAddVarRW(post_process_bar, "Chromatic aberration", TW_TYPE_FLOAT, &chromatic_aberration->amount, "min=0 max=2 step=0.01 ");
	TwAddVarRW(post_process_bar, "Blur", TW_TYPE_FLOAT, &blur->amount, "min=0 max=8 step=0.01 ");
	TwAddVarRW(post_process_bar, "SSRR Amount", TW_TYPE_FLOAT, &ssrr->amount, "min=0 max=5 step=0.01 ");
	TwAddVarRW(post_process_bar, "SSRR Quality", TW_TYPE_FLOAT, &ssrr->quality, "min=0.2 max=10 step=0.01 ");
}