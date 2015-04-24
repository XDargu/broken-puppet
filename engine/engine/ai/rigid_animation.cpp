#include "mcv_platform.h"
#include "rigid_animation.h"
#include "components\comp_transform.h"

CRigidAnimation::CRigidAnimation(CHandle the_target_transform) {
	SET_ERROR_CONTEXT("Creating an animation", "")

	TCompTransform* t = the_target_transform;
	XASSERT(t, "Animation must have a valid Transform");

	target_transform = the_target_transform;
	current_keyframe = 0;
	playing = false;
	reverted = false;
	loop = false;
}

void CRigidAnimation::update(float elapsed) {

	// Play the animation backwards
	if (reverted) {
		if (current_keyframe > 0) {
			if (keyframes[current_keyframe].update(elapsed) == true) {
				current_keyframe--;
			}
		}
	}
	// Play the animation normally
	else {
		if (current_keyframe < keyframes.size()) {
			if (keyframes[current_keyframe].update(elapsed) == true) {
				current_keyframe++;
			}
		}
		else {
			// End of animation
			// TODO: Hacer que funcione
			if (loop) {
				current_keyframe = 0;
			}
		}
	}
}

void CRigidAnimation::addKeyframe(TKeyFrame keyframe) {
	SET_ERROR_CONTEXT("Adding a keyframe to an animation", "")

	XASSERT(keyframe.target_transform == target_transform, "Animation and keyframe must have the same target transform");

	keyframes.push_back(keyframe);
}

void CRigidAnimation::addKeyframe(XMVECTOR the_target_position, XMVECTOR the_target_rotation, float the_time) {	
	TKeyFrame keyframe(target_transform, the_target_position, the_target_rotation, the_time);
	keyframes.push_back(keyframe);
}

void CRigidAnimation::addRelativeKeyframe(XMVECTOR the_target_position, XMVECTOR the_target_rotation, float the_time) {
	TCompTransform* t = target_transform;

	TKeyFrame keyframe(target_transform, t->position + the_target_position, XMQuaternionMultiply(t->rotation, the_target_rotation), the_time);
	keyframes.push_back(keyframe);
}

void CRigidAnimation::play() {
	playing = true;
}

void CRigidAnimation::pause() {
	playing = false;
}

void CRigidAnimation::stop() {
	current_keyframe = 0;
	playing = false;
}

void CRigidAnimation::reset() {
	current_keyframe = 0;
}