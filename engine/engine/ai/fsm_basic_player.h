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
	void ThrowString(float elapsed);
	void Fall(float elapsed);
	void Land(float elapsed);
	void WrongFall();
	void WrongLand(float elapsed);
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

	CHandle life;


	bool falling;
	float state_time;
	float last_hit;
	void EvaluateHit();
	void EvaluateLiveToLose(float damage);

	void localCameraFront();
	bool EvaluateMovement();
	bool EvaluateFall();

	bool trueEveryXSeconds(float time);
};

#endif