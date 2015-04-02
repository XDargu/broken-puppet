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
}