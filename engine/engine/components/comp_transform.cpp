#include "mcv_platform.h"
#include "comp_transform.h"
#include "comp_rigid_body.h"
#include "comp_unity_character_controller.h"

void TCompTransform::loadFromAtts(const std::string& elem, MKeyValue& atts) {
	position = atts.getPoint("position");
	rotation = atts.getQuat("rotation");
	scale = atts.getPoint("scale");

	TCompRigidBody* r = getSibling<TCompRigidBody>(this);
	TCompUnityCharacterController* u = getSibling<TCompUnityCharacterController>(this);
	if (r) {
		r->rigidBody->setGlobalPose(Physics.transformToPxTransform(*this));
	}
	if (u) {
		u->enemy_rigidbody->setGlobalPose(Physics.transformToPxTransform(*this));
	}

	prev_transform = TTransform(position, rotation, scale);
}

void TCompTransform::update(float elapsed) {
	prev_transform.position = position;
	prev_transform.rotation = rotation;
	prev_transform.scale = scale;
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