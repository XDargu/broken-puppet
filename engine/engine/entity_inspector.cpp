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
	bar = TwNewBar("Entity Inspector");
	
	// AntTweakBar test
	int barSize[2] = { 224, 320 };
	TwSetParam(bar, NULL, "size", TW_PARAM_INT32, 2, barSize);

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
	TAABB* e_aabb = target_entity->get<TAABB>();
	TCollider* e_collider = target_entity->get<TCollider>();
	TRigidBody* e_rigidbody = target_entity->get<TRigidBody>();

	// AntTweakBar test
	int barSize[2] = { 224, 320 };
	TwSetParam(bar, NULL, "size", TW_PARAM_INT32, 2, barSize);

	if (e_name) {
		TwAddVarRW(bar, "Name", TW_TYPE_CSSTRING(sizeof(e_name->name)), e_name->name, " group=Name");
		TwAddSeparator(bar, "Name", "");
	}
	if (e_transform) {
		TwAddVarRW(bar, "Position", TW_TYPE_DIR3F, &position, " group=Transform");
		TwAddVarRW(bar, "Rotation", TW_TYPE_QUAT4F, &rotation, " group=Transform");
		TwAddVarRW(bar, "Scale", TW_TYPE_DIR3F, &scale, " group=Transform");
		TwAddSeparator(bar, "Transform", "");
	}
	if (e_aabb) {
		TwAddVarRW(bar, "Min", TW_TYPE_DIR3F, &min, " group=AABB");
		TwAddVarRW(bar, "Max", TW_TYPE_DIR3F, &max, " group=AABB");
		TwAddVarRO(bar, "Center", TW_TYPE_DIR3F, &center, " group=AABB");
		TwAddVarRO(bar, "Extents", TW_TYPE_DIR3F, &extents, " group=AABB");
		TwAddVarRO(bar, "Size", TW_TYPE_DIR3F, &size, " group=AABB");
	}
	if (e_collider) {
		TwAddVarRO(bar, "Static friction", TW_TYPE_FLOAT, &static_friction, " group=Collider");
		TwAddVarRO(bar, "Dynamic friction", TW_TYPE_FLOAT, &static_friction, " group=Collider");
		TwAddVarRO(bar, "Restitution", TW_TYPE_FLOAT, &static_friction, " group=Collider");
	}
	if (e_rigidbody) {
		TwAddVarRO(bar, "Linear velocity", TW_TYPE_DIR3F, &linearVelocity, " group=Rigidbody");
		TwAddVarRO(bar, "Angular velocity", TW_TYPE_DIR3F, &angularVelocity, " group=Rigidbody");
	}	
}