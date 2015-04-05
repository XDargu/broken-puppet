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

	// Update the position and rotation
	transform->position = XMVectorLerp(transform->position, XMLoadFloat3(&target_position), elapsed_time / time * elapsed);
	transform->rotation = XMQuaternionSlerp(transform->rotation, XMLoadFloat4(&target_rotation), elapsed_time / time * elapsed);
	elapsed_time += elapsed;

	// If the keyFrame has reached the destiny, then leave
	if (elapsed_time >= time)
		return true;

	return false;
}