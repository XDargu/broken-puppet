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
	int current_keyframe;
	bool loop;

	CRigidAnimation(CHandle the_target_transform);

	void update(float elapsed);

	void addKeyframe(TKeyFrame keyframe);
	void addKeyframe(XMVECTOR the_target_position, XMVECTOR the_target_rotation, float the_time);
	void addRelativeKeyframe(XMVECTOR the_target_position, XMVECTOR the_target_rotation, float the_time);

	void play();
	void pause();
	void stop();
	
	void reset();
	
};

#endif