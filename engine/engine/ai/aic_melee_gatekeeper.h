#ifndef _AIC_GATEKEEPER
#define _AIC_GATEKEEPER

#include "aicontroller.h"

class aic_melee_gatekeeper : public aicontroller
{
public:

	CEntity* player;
	CEntity* gate;

	float view_distance;
	float forget_distance;
	float in_gate_distance;
	float forget_gate_distance;
	float attack_distance;
	float out_of_reach_distance;
	float follow_rotation_velocity;
	float chase_speed;

	void IdleState();
	void ChaseState();
	void AttackState();
	void ReturnGateState();

	void Init();
};

#endif