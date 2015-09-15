#ifndef _RIGID_ANIMATION_H_
#define _RIGID_ANIMATION_H_

#include "key_frame.h"

class CRigidAnimation {
private:
	bool playing;
	bool reverted;
public:
	std::vector<TKeyFrame> keyframes;

	CHandle target_transform;
	CHandle target_kinematic;
	int current_keyframe;
	bool loop;

	CRigidAnimation();
	CRigidAnimation(CHandle the_target_transform);

	void update(float elapsed);

	void addKeyframe(TKeyFrame keyframe);
	void addKeyframe(XMVECTOR the_target_position, XMVECTOR the_target_rotation, float the_time);
	void addKeyframe(XMVECTOR the_target_position, float the_time); // Position only, mantain rotation
	void addRelativeKeyframe(XMVECTOR the_target_position, XMVECTOR the_target_rotation, float the_time);
	void addRelativeKeyframe(XMVECTOR the_target_position, float the_time); // Position only, mantain rotation

	void setTargetTransfrom(CHandle transform);

	void play();
	void pause();
	void stop();
	
	void reset();
	
};

#endif