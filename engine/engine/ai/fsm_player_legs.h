#ifndef _FSM_PLAYER_LEGS_H_
#define _FSM_PLAYER_LEGS_H_

#include "aifsmcontroller.h"
#include "fsm_player_torso.h"

class FSMPlayerLegs : public aifsmcontroller
{
private:
	int current_animation_id;	
	float idle_var_count;
	float airTime;
public:

	physx::PxVec3 movement_dir;

	CHandle comp_character_controller;
	CHandle comp_rigidbody;
	CHandle comp_collider;
	CHandle comp_skeleton;
	CHandle comp_ragdoll;
	CHandle comp_skeleton_ik;
	CHandle comp_player_controller;
	CHandle comp_player_pivot_transform;
	CHandle entity_camera;
	FSMPlayerTorso* torso;

	CHandle life;

	bool canThrow;
	bool falling;
	float last_hit;
	float walk_speed;
	float run_speed;
	float dead_counter;

	FSMPlayerLegs();
	~FSMPlayerLegs();

	void Init();

	void Idle(float elapsed);
	void Walk(float elapsed);
	void Jump(float elapsed);
	void Run(float elapsed);
	void ThrowString(float elapsed);
	void ThrowStringPartial(float elapsed);
	void ThrowStringGolden(float elapsed);
	void PullString(float elapsed);
	void Fall(float elapsed);
	void Land(float elapsed);
	void WrongFall(float elapsed);
	void WrongLand(float elapsed);
	void ProcessHit(float elapsed);
	void Hurt(float elapsed);
	void Ragdoll(float elapsed);
	void Dead(float elapsed);
	void ReevaluatePriorities();
	void WakeUp(float elapsed);
	void WakeUpTeleport(float elapsed);

	void Victory(float elapsed);

	void EvaluateHit(float damage);
	void EvaluateLiveToLose(float damage);

	void localCameraFront();
	bool EvaluateMovement(bool lookAtCamera, float elapsed);
	bool EvaluateFall(float elapsed);

	bool trueEveryXSeconds(float time);

	void stopAllAnimations();
	void stopAnimation(int id);
	float getAnimationDuration(int id);
	bool canPlayerThrow();
};

#endif