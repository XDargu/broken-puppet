#include "mcv_platform.h"
#include "entity_inspector.h"
#include <AntTweakBar.h>
#include "handle.h"

using namespace physx;

CEntityInspector::CEntityInspector() { }

CEntityInspector::~CEntityInspector() { }

TwBar *bar;

// Transform
XMVECTOR position;
XMVECTOR rotation;
XMVECTOR scale;

// Name
char name[32];

// AABB
XMVECTOR min;
XMVECTOR max;
XMVECTOR center;
XMVECTOR extents;
XMVECTOR size;

// Collider
float static_friction;
float dynamic_friction;
float restitution;

// Rigidbody
PxVec3 linearVelocity;
PxVec3 angularVelocity;


void CEntityInspector::init() {
	// Create a tewak bar
	bar = TwNewBar("Inspector");
	
	// AntTweakBar test
	int barSize[2] = { 224, 320 };
	int varPosition[2] = { 30, 80 };
	TwSetParam(bar, NULL, "size", TW_PARAM_INT32, 2, barSize);
	TwSetParam(bar, NULL, "position", TW_PARAM_INT32, 2, varPosition);
	TwDefine(" Inspector label='Entity inspector' ");
	TwDefine(" TW_HELP visible=false ");

}

// COLLIDER
void TW_CALL SetStaticFriction(const void *value, void *clientData)
{
	static_cast<TCollider *>(clientData)->getMaterial()->setStaticFriction(*static_cast<const float *>(value));
}
void TW_CALL GetStaticFriction(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TCollider *>(clientData)->getMaterial()->getStaticFriction();
}

void TW_CALL SetDynamicFriction(const void *value, void *clientData)
{
	static_cast<TCollider *>(clientData)->getMaterial()->setDynamicFriction(*static_cast<const float *>(value));
}
void TW_CALL GetDynamicFriction(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TCollider *>(clientData)->getMaterial()->getDynamicFriction();
}

void TW_CALL SetRestitution(const void *value, void *clientData)
{
	static_cast<TCollider *>(clientData)->getMaterial()->setRestitution(*static_cast<const float *>(value));
}
void TW_CALL GetRestitution(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TCollider *>(clientData)->getMaterial()->getRestitution();
}

// RIGIDBODY
void TW_CALL SetMass(const void *value, void *clientData)
{
	PxRigidBodyExt::setMassAndUpdateInertia(*static_cast<TRigidBody *>(clientData)->rigidBody, *static_cast<const float *>(value), NULL, false);
}
void TW_CALL GetMass(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TRigidBody *>(clientData)->rigidBody->getMass();
}

void TW_CALL SetGravity(const void *value, void *clientData)
{
	static_cast<TRigidBody *>(clientData)->setUseGravity(*static_cast<const bool *>(value));
}
void TW_CALL GetGravity(void *value, void *clientData)
{
	*static_cast<bool *>(value) = static_cast<TRigidBody *>(clientData)->isUsingGravity();
}

void TW_CALL SetKinematic(const void *value, void *clientData)
{
	static_cast<TRigidBody *>(clientData)->setKinematic(*static_cast<const bool *>(value));
}
void TW_CALL GetKinematic(void *value, void *clientData)
{
	*static_cast<bool *>(value) = static_cast<TRigidBody *>(clientData)->isKinematic();
}

void TW_CALL SetLinearDamping(const void *value, void *clientData)
{
	static_cast<TRigidBody *>(clientData)->rigidBody->setLinearDamping(*static_cast<const float *>(value));
}
void TW_CALL GetLinearDamping(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TRigidBody *>(clientData)->rigidBody->getLinearDamping();
}

void TW_CALL SetAngularDamping(const void *value, void *clientData)
{
	static_cast<TRigidBody *>(clientData)->rigidBody->setAngularDamping(*static_cast<const float *>(value));
}
void TW_CALL GetAngularDamping(void *value, void *clientData)
{
	*static_cast<float *>(value) = static_cast<TRigidBody *>(clientData)->rigidBody->getAngularDamping();
}

void CEntityInspector::update() {
	TTransform* e_transform = target_entity->get<TTransform>();
	TCompName* e_name = target_entity->get<TCompName>();
	TAABB* e_aabb = target_entity->get<TAABB>();
	TCollider* e_collider = target_entity->get<TCollider>();
	TRigidBody* e_rigidbody = target_entity->get<TRigidBody>();

	if (e_transform) {
		position = e_transform->position;
		rotation = e_transform->rotation;		
		scale = e_transform->scale;
	}

	if (e_name) {
		std::strcpy(name, e_name->name);
	}

	if (e_aabb) {
		min = e_aabb->min;
		max = e_aabb->max;
		center = e_aabb->getCenter();
		extents = e_aabb->getExtents();
		size = e_aabb->getSize();
	}

	if (e_collider) {
		static_friction = e_collider->getMaterial()->getStaticFriction();
		dynamic_friction = e_collider->getMaterial()->getDynamicFriction();
		restitution = e_collider->getMaterial()->getRestitution();
	}

	if (e_rigidbody) {
		linearVelocity = e_rigidbody->rigidBody->getLinearVelocity();
		angularVelocity = e_rigidbody->rigidBody->getAngularVelocity();
	}
}

void CEntityInspector::inspectEntity(CEntity* the_entity) {
	target_entity = the_entity;

	TTransform* e_transform = target_entity->get<TTransform>();
	TCompName* e_name = target_entity->get<TCompName>();
	TMesh* e_mesh = target_entity->get<TMesh>();
	TAABB* e_aabb = target_entity->get<TAABB>();
	TCollider* e_collider = target_entity->get<TCollider>();
	TRigidBody* e_rigidbody = target_entity->get<TRigidBody>();


	TwRemoveAllVars(bar);
	if (e_name) {
		TwAddVarRW(bar, "Name", TW_TYPE_CSSTRING(sizeof(e_name->name)), e_name->name, " group=Name");
		TwAddSeparator(bar, "Name", "");
	}
	if (e_transform) {
		TwAddVarRW(bar, "Position", TW_TYPE_DIR3F, &e_transform->position, " group=Transform");
		TwAddVarRW(bar, "Rotation", TW_TYPE_QUAT4F, &e_transform->rotation, " group=Transform");
		TwAddVarRW(bar, "Scale", TW_TYPE_DIR3F, &e_transform->scale, " group=Transform");
		TwAddSeparator(bar, "Transform", "");
	}
	if (e_mesh) {
		TwAddVarRW(bar, "Color", TW_TYPE_COLOR4F, &e_mesh->color, " group=Mesh");
	}
	if (e_collider) {
		TwAddVarCB(bar, "Static friction", TW_TYPE_FLOAT, SetStaticFriction, GetStaticFriction, e_collider, " min=0 max=1 step=0.1 group=Collider");
		TwAddVarCB(bar, "Dynamic friction", TW_TYPE_FLOAT, SetDynamicFriction, GetDynamicFriction, e_collider, " min=0 max=1 step=0.1 group=Collider");
		TwAddVarCB(bar, "Restitution", TW_TYPE_FLOAT, SetRestitution, GetRestitution, e_collider, " min=0 max=1 step=0.1 group=Collider");
	}
	if (e_rigidbody) {
		TwAddVarCB(bar, "Mass", TW_TYPE_FLOAT, SetMass, GetMass, e_rigidbody, " min=0.1 group=Rigidbody");
		TwAddVarCB(bar, "Damping", TW_TYPE_FLOAT, SetLinearDamping, GetLinearDamping, e_rigidbody, " min=0 group=Rigidbody");
		TwAddVarCB(bar, "Angular Damping", TW_TYPE_FLOAT, SetAngularDamping, GetAngularDamping, e_rigidbody, " min=0 group=Rigidbody");
		TwAddVarCB(bar, "Use Gravity", TW_TYPE_BOOL32, SetGravity, GetGravity, e_rigidbody, " group=Rigidbody");
		TwAddVarCB(bar, "Is Kinematic", TW_TYPE_BOOL32, SetKinematic, GetKinematic, e_rigidbody, " group=Rigidbody");		
		TwAddVarRO(bar, "Linear velocity", TW_TYPE_DIR3F, &linearVelocity, " group=Rigidbody");
		TwAddVarRO(bar, "Angular velocity", TW_TYPE_DIR3F, &angularVelocity, " group=Rigidbody");
	}	
	if (e_aabb) {
		TwAddVarRW(bar, "Min", TW_TYPE_DIR3F, &e_aabb->max, " group=AABB");
		TwAddVarRW(bar, "Max", TW_TYPE_DIR3F, &e_aabb->max, " group=AABB");
		TwAddVarRO(bar, "Center", TW_TYPE_DIR3F, &center, " group=AABB");
		TwAddVarRO(bar, "Extents", TW_TYPE_DIR3F, &extents, " group=AABB");
		TwAddVarRO(bar, "Size", TW_TYPE_DIR3F, &size, " group=AABB");
	}
}