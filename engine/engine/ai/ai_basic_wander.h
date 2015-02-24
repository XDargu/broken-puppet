#ifndef _AI_BASIC_WANDER
#define _AI_BASIC_WANDER

#include "aicontroller.h"

class ai_basic_wander : public aicontroller
{
public:

	CHandle enemyController;
	CHandle trans;

	float movement_speed;
	float wait_time;
	float count;

	void IdleState(float deltaTime);
	void WanderState(float deltaTime);

	void Init();
};

#endif