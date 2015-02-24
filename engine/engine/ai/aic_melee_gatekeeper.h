/*
#ifndef _AIC_GATEKEEPER
#define _AIC_GATEKEEPER

#include "aicontroller.h"

class aic_melee_gatekeeper : public aicontroller
{
public:

	CEntityOld* player;
	CEntityOld* gate;

	float view_distance;
	float forget_distance;
	float in_gate_distance;
	float forget_gate_distance;
	float attack_distance;
	float out_of_reach_distance;
	float follow_rotation_velocity;
	float chase_speed;

	void IdleState(float deltaTime);
	void ChaseState(float deltaTime);
	void AttackState(float deltaTime);
	void ReturnGateState(float deltaTime);

	void Init();
};

#endif

*/