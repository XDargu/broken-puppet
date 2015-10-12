#ifndef _FSM_SUBSTITUTE_H_
#define _FSM_SUBSTITUTE_H_

#include "aifsmcontroller.h"
#include "physics_manager.h"
#include "entity_manager.h"

class fsm_substitute : public aifsmcontroller
{
private:
	CHandle comp_skeleton;
	int last_anim_id;

	void stopAllAnimations();
	void loopAnimationIfNotPlaying(int id, bool restart);
	void stopAnimation(int id);
	float getAnimationDuration(int id);

public:
	fsm_substitute();
	~fsm_substitute();

	void init();

	void Idle(float elapsed);
	void LittleTalk(float elapsed);

};



#endif