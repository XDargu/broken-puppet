#ifndef _FSM_BASIC_PLAYER_H_
#define _FSM_BASIC_PLAYER_H_

#include "aicontroller.h"

class fsm_basic_player : public aicontroller
{

public:
	fsm_basic_player();
	~fsm_basic_player();

	void Init();

	void Idle();
	void Walk();
	void Jump();
	void Run();
	void ThrowString();
	void Fall();
	void Land();
	void WrongFall();
	void WrongLand();
	void ProcessHit();
	void Hurt();
	void Ragdoll(float elapsed);
	void Dead();
	void ReevaluatePriorities();
	void WakeUp();

	CHandle comp_mesh;
	CHandle comp_unity_controller;
	CHandle comp_basic_player_controller;
	CHandle comp_player_pivot_transform;
	CHandle entity_camera;
	
	float state_time;
	float last_hit;
	void EvaluateHit();
	void EvaluateLiveToLose();

	void localCameraFront();
	bool EvaluateMovement();
	bool EvaluateFall();

	bool trueEverySecond();
	bool trueEveryXSeconds(float time);


	void Rerotate();
	void Advance();
	void Orbit(bool right);
	void LookAt(XMVECTOR target);
	void Twist(float freq, float power);
};

#endif