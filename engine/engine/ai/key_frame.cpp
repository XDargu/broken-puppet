#include "mcv_platform.h"
#include "key_frame.h"
#include "components\all_components.h"

TKeyFrame::TKeyFrame(CHandle the_target_transform, XMVECTOR the_target_position, XMVECTOR the_target_rotation, float the_time) {
	target_transform = the_target_transform;
	XMStoreFloat3(&target_position, the_target_position);
	XMStoreFloat4(&target_rotation, the_target_rotation);
	time = the_time;
	elapsed_time = 0.f;
}

bool TKeyFrame::update(float elapsed) {
	TCompTransform* transform = target_transform;

	// In the first frame we store the initial position and rotation
	if (elapsed_time == 0) {
		XMStoreFloat3(&initial_position, transform->position);
		XMStoreFloat4(&initial_rotation, transform->rotation);
	}

	// Update the position and rotation
	XMVECTOR desired_pos = XMVectorLerp(XMLoadFloat3(&initial_position), XMLoadFloat3(&target_position), elapsed_time / time);
	XMVECTOR desired_rot = XMQuaternionSlerp(XMLoadFloat4(&initial_rotation), XMLoadFloat4(&target_rotation), elapsed_time / time);
	elapsed_time += elapsed;

	// Update the rigid position, if needed
	if (target_kinematic.isValid()) {
		TCompRigidBody* rigid = target_kinematic;

		if (rigid->isKinematic()) {
			rigid->rigidBody->setKinematicTarget(PxTransform(Physics.XMVECTORToPxVec3(desired_pos), Physics.XMVECTORToPxQuat(desired_rot)));
		}
	}
	else {
		// Update position and rotation manually only if there are no kinematic targets
		transform->position = desired_pos;
		transform->rotation = desired_rot;
	}

	// If the keyFrame has reached the destiny, then leave
	if (elapsed_time >= time) {
		elapsed_time = 0;
		return true;
	}

	return false;
}