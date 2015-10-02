#ifndef _FSM_BOSS_H_
#define _FSM_BOSS_H_

#include "aifsmcontroller.h"
#include "physics_manager.h"
#include "entity_manager.h"

// ----------------------------------------
/**/
struct TPattern{
	int attack;
	float wait_time;
};

struct TAttackPattern {
	vector<TPattern> pattern;
};
/**/


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
	CHandle comp_ragdoll;
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

	int shoots_amount;
	
	float last_attack;
	int CalculateAttack();

	bool RainDebris(float elapsed);
	void rainJustBombs();
	void FistParticles(float elapsed);
	int debris_created;
	float debris_creation_delay;
	float bomb_creation_delay;
	XMVECTOR last_random_pos;
	XMVECTOR last_created_pos;
	

	float ball_size;
	vector<CHandle> ball_list;
	
	int times_stunned;

	void SelectObjToShoot();
	float distance_to_hand;
	float max_shoot_angle;

	CHandle obj_to_shoot;

	float r_hand_pos_y;
	bool r_hand_change;

	float l_hand_pos_y;
	bool l_hand_change;

	//bool extrem_need_bombs; // When we need bombs but there are too much objs in the scene. Not implemented
	bool need_bombs;

	bool bombs_destroyed;
	void destroyBombs();
	
	bool appear;

public:
	fsm_boss();
	~fsm_boss();

	bool has_left;
	bool has_right;

	void Init();

	void Hidden(float elapsed);
	void RiseUp();

	void Idle1(float elapsed);
	void Hit1(float elapsed);
	void Recover(float elapsed);

	void Stunned1();

	void Rain1Prepare();
	void Rain1Loop(float elapsed);
	void Rain1Recover();

	
	void Ball1Initial(float elapsed);
	void Ball1Loop(float elapsed);
	void Ball1Launch(float elapsed);

	void Shoot1ReleaseDef();
	void Shoot1DownDef();
	void Shoot1Shoot(float elapsed);
	void Shoot1Reload(float elapsed);

	void WaveLeft(float elapsed);
	void WaveRight();

	void Proximity(float elapsed);

	void Damaged1Left(float elapsed);
	void Damaged1Right(float elapsed);

	void Damaged1LeftFinal();
	void Damaged1RightFinal();

	void FinalState(float elapsed);

	void Death();

	bool EvaluateHit(int arm_damaged);
	bool HeartHit();

	CHandle Player;

	bool HeadHit();

	bool can_proximity;
	bool can_proximity_hit;

	XMVECTOR original_pos;

	int last_attack_it;
	int pattern_it;
	int pattern_current;

	bool lua_boss_init;

	/**/
	std::vector<TAttackPattern> attack_pattern_list;
	/**/
};



#endif