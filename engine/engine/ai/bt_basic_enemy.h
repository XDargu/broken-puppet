#ifndef _BT_BASIC_ENEMY_H_
#define _BT_BASIC_ENEMY_H_


#include "mcv_platform.h"
#include "behaviour_trees.h"

class bt_basic_enemy :public bt
{
	CHandle player;
	CHandle character_controller;
	CHandle comp_mesh;
	CHandle comp_life;

	float attack_distance;
	float view_distance;
	float forget_distance;
	float out_of_reach_distance;
	float angular_velocity;
	float velocity;

	XMVECTOR wander_target;

	bool already_animated; 
	bool initial_attack_done;
	bool already_attacked;
	bool player_viewed_sensor;

	unsigned int my_id;
	unsigned int lastNumNeedlesViewed;
	unsigned int currentNumNeedlesViewed;

public:
	void create(string);

	bool conditionWander();
	int actionSearchPoint();
	int actionChasePoint();

	bool conditionPatrol();
	int actionChasePlayer();
	int actionInitialAttack();
	int actionIdleWar();
	int actionAttack();

	void EvaluateHit(float damage);

	bool trueEverySecond();
	bool trueEveryXSeconds(float time);

	//---------------------------------------------------------------------------------------------------
	void update(float elapsed);
	void setTarget(CHandle target) { player = target; };
	string getCurrentNode();
	void setId(unsigned int id);
	unsigned int getInt();
	CHandle getTransform();
	//---------------------------------------------------------------------------------------------------

	//--------- SENSORES --------------------------------------------------------------------------------
	void playerViewedSensor();
	void needleViewedSensor();
	//---------------------------------------------------------------------------------------------------
};

#endif

