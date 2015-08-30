#include "mcv_platform.h"
#include "comp_local_rotation.h"
#include "comp_transform.h"
#include "comp_static_body.h"

void TCompLocalRotation::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	h_transform = assertRequiredComponent<TCompTransform>(this);
	speed = atts.getFloat("speed", 1);
	axis = atts.getInt("axis", 0);
}

void TCompLocalRotation::update(float elapsed) {

	TCompTransform* c_transform = h_transform;

	XMVECTOR rot = XMVectorSet(0, 0, 0, 1);
	if (axis == 0) {
		rot = XMQuaternionRotationAxis(c_transform->getFront(), speed * elapsed);
	}
	if (axis == 1) {
		rot = XMQuaternionRotationAxis(-c_transform->getLeft(), speed * elapsed);
	}
	if (axis == 2) {
		rot = XMQuaternionRotationAxis(c_transform->getUp(), speed * elapsed);
	}

	c_transform->rotation = XMQuaternionMultiply(c_transform->rotation, rot);

}