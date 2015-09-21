#include "mcv_platform.h"
#include "comp_transform.h"
#include "comp_rigid_body.h"
#include "comp_unity_character_controller.h"
#include "entity_manager.h"
#include "physics_manager.h"
#include "ai\logic_manager.h"

void TCompTransform::loadFromAtts(const std::string& elem, MKeyValue& atts) {
	position = atts.getPoint("position");
	rotation = atts.getQuat("rotation");
	scale = atts.getPoint("scale");

	// Parent to follow
	if (atts.has("parent")) {
		strcpy(parent_name, atts["parent"].c_str());
	}

	type = 0;
	room_id = 0;

	TCompRigidBody* r = getSibling<TCompRigidBody>(this);
	TCompUnityCharacterController* u = getSibling<TCompUnityCharacterController>(this);
	if (r) {
		r->rigidBody->setGlobalPose(Physics.transformToPxTransform(*this));
		type = 1;
	}
	if (u) {
		u->enemy_rigidbody->setGlobalPose(Physics.transformToPxTransform(*this));
	}

	prev_transform = TTransform(position, rotation, scale);
}

void TCompTransform::init() {
	if (parent_name[0] != 0x00) {
		CEntity* e_parent = CEntityManager::get().getByName(parent_name);
		parent = e_parent->get<TCompTransform>();
		XASSERT(parent.isValid(), "Parent %s must have a transform", parent_name);
		TCompTransform* c_parent = parent;

		PxTransform my_transform = PxTransform(Physics.XMVECTORToPxVec3(position), Physics.XMVECTORToPxQuat(rotation));
		PxTransform parent_transform = PxTransform(Physics.XMVECTORToPxVec3(c_parent->position), Physics.XMVECTORToPxQuat(c_parent->rotation));

		PxTransform parent_inverse = parent_transform.getInverse();
		PxTransform local = parent_inverse.transform(my_transform);
		parent_offset.position = Physics.PxVec3ToXMVECTOR(local.p);
		parent_offset.rotation = Physics.PxQuatToXMVECTOR(local.q);

		//parent_offset.rotation = XMQuaternionMultiply(rotation, XMQuaternionInverse(c_parent->rotation));
		//parent_offset.position = c_parent->position - XMVector3Rotate(position, XMQuaternionInverse(c_parent->rotation));
		//parent_offset.position = c_parent->inverseTransformPoint(position);		
	}

	TCompRigidBody* r = getSibling<TCompRigidBody>(this);
	if (r && !r->isKinematic())
		type = 1;

	// Room id
	room_id = CLogicManager::get().getPointZoneID(position);
}

void TCompTransform::update(float elapsed) {
	prev_transform.position = position;
	prev_transform.rotation = rotation;
	prev_transform.scale = scale;

	if (parent_name[0] != 0x00) {
		TCompTransform* c_parent = parent;

		/*position = c_parent->transformPoint(parent_offset.position);
		rotation = c_parent->transformDirection(parent_offset.rotation);*/

		/*PxTransform me_to_parent = PxTransform(Physics.XMVECTORToPxVec3(parent_offset.position), Physics.XMVECTORToPxQuat(parent_offset.rotation));
		PxTransform parent_transform = PxTransform(Physics.XMVECTORToPxVec3(c_parent->position), Physics.XMVECTORToPxQuat(c_parent->rotation));

		PxTransform my_transform = parent_transform.transform(me_to_parent);

		position = Physics.PxVec3ToXMVECTOR(my_transform.p);
		rotation = Physics.PxQuatToXMVECTOR(my_transform.q);*/

		TTransform t = c_parent->transform(parent_offset);
		t = t.transform(local_transform);
		position = t.position;
		rotation = t.rotation;
		
		/*position = c_parent->position + XMVector3Rotate(local_transform.position, c_parent->rotation);
		rotation = XMQuaternionMultiply(local_transform.rotation, c_parent->rotation);

		position = position + XMVector3Rotate(parent_offset.position, rotation);
		rotation = XMQuaternionMultiply(parent_offset.rotation, rotation);*/


		/*
		position = c_parent->position + XMVector3Rotate(parent_offset.position, c_parent->rotation);
		rotation = XMQuaternionMultiply(parent_offset.rotation, c_parent->rotation);

		position = position + XMVector3Rotate(local_transform.position, rotation);
		rotation = XMQuaternionMultiply(local_transform.rotation, rotation);*/

		/*position = parent_offset_pos + c_parent->position;
		rotation = XMQuaternionMultiply(c_parent->rotation, parent_offset_rot);*/

	}

	
}

void TCompTransform::teleport(XMVECTOR the_position) {

	position = the_position;

	TCompRigidBody* r = getSibling<TCompRigidBody>(this);
	TCompUnityCharacterController* u = getSibling<TCompUnityCharacterController>(this);
	if (r) {
		r->rigidBody->setGlobalPose(Physics.transformToPxTransform(*this));
	}
	if (u) {
		u->enemy_rigidbody->setGlobalPose(Physics.transformToPxTransform(*this));
	}
}

bool TCompTransform::transformChanged() {
	int equal = memcmp(&prev_transform, &*this, sizeof(*this));

	return equal != 0;
}

int TCompTransform::getType() {
	if (parent_name[0] != 0x00) {
		TCompTransform* c_parent = parent;
		return c_parent->getType();
	}
	else {
		return (int)(type * 100);
	}
}