#ifndef _FSM_BOSS_H_
#define _FSM_BOSS_H_

#include "aifsmcontroller.h"
#include "physics_manager.h"
#include "entity_manager.h"

class fsm_boss : public aifsmcontroller
{
private:
	PxVec3 point_offset;
	float distance_to_point;
	CEntityManager* m_entity_manager;
	float obj_distance;
	CHandle obj_selected;
	float force;
	bool need_reorientate;
	float reorientate_angle;
	float no_reorientate_angle;

	CHandle comp_skeleton;
	CHandle comp_skeleton_lookat;
	CHandle m_player;

	void Reorientate(float elapsed, bool just_look);
	void Release_def();

	// Animation
	int last_anim_id;

	void stopAllAnimations();
	void loopAnimationIfNotPlaying(int id, bool restart);
	void stopAnimation(int id);
	float getAnimationDuration(int id);
	
	int hurt_state;
	int arm_state;

	int shoots_amount;

public:
	fsm_boss();
	~fsm_boss();

	void Init();

	void Hidden();
	void RiseUp();

	void Idle1(float elapsed);
	void Hit1(float elapsed);
	void Recover(float elapsed);

	void Stunned1();

	void Rain1Prepare();
	void Rain1Loop();
	void Rain1Recover();

	
	void Ball1Initial(float elapsed);
	void Ball1Loop(float elapsed);
	void Ball1Launch(float elapsed);

	void Shoot1ReleaseDef();
	void Shoot1DownDef();
	void Shoot1Shoot();
	void Shoot1Reload();

	void WaveLeft();
	void WaveRight();

	void Proximity(float elapsed);

	void Damaged1Left(float elapsed);
	void Damaged1Right(float elapsed);

	void Damaged1LeftFinal();
	void Damaged1RightFinal();

	void Idle2();
	void Stunned2();

	void Wave();
	void Rain2();
	void Ball2();
	void Shoot2();

	void Damaged2();

	void FinalState();	

	void Death();

	bool EvaluateHit(int arm_damaged);

	CHandle Player;


	
};

#endif