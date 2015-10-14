#ifndef _FSM_SUBSTITUTE_H_
#define _FSM_SUBSTITUTE_H_

#include "aifsmcontroller.h"
#include "physics_manager.h"
#include "entity_manager.h"

class fsm_substitute : public aifsmcontroller
{
private:
	CHandle comp_skeleton;
	CHandle comp_ragdoll;

	void stopAllAnimations();
	void loopAnimation(int id, bool restart);
	void stopAnimation(int id);
	float getAnimationDuration(int id);

public:
	fsm_substitute();
	~fsm_substitute();

	void init();

	void IdleSit(float elapsed);
	void Idle(float elapsed);
	void LittleTalk(float elapsed);

};



#endif