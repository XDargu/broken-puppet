#include "mcv_platform.h"
#include "entity_inspector.h"
#include <AntTweakBar.h>
#include "handle\handle.h"
#include "components\all_components.h"
#include "importer_parser.h"
#include <locale>
#include <algorithm>

using namespace physx;

CEntityInspector::CEntityInspector() { }

CEntityInspector::~CEntityInspector() { }

TwBar *bar;

XMVECTOR center;
XMVECTOR extents;
XMVECTOR size;

// Rigidbody
PxVec3 linearVelocity;
PxVec3 angularVelocity;

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


	// Logic
	TCompTrigger* e_trigger = inspectedEntity->get<TCompTrigger>();

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
		TwAddVarRW(bar, "Min", TW_TYPE_DIR3F, &e_aabb->max, " group=AABB axisx=-x axisz=-z");
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
		TwAddVarRW(bar, "PointLightIntensity", TW_TYPE_FLOAT, &e_point_light->intensity, " min=0 max=10 group='Point Light' label='Intensity' ");
		TwAddVarRW(bar, "PointLightColor", TW_TYPE_COLOR3F, &e_point_light->color, " group='Point Light' label='Color' ");
		TwAddVarRW(bar, "PointLightRadius", TW_TYPE_FLOAT, &e_point_light->radius, " group='Point Light' label='Radius' min=0.1");
	}
	if (e_comp_ai_fsm_basic) {
		TwAddVarCB(bar, "AIFSMBasicState", TW_TYPE_STDSTRING, NULL, GetAIFSMState, e_comp_ai_fsm_basic, " group='AI FSM Basic' label='State'");
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
}