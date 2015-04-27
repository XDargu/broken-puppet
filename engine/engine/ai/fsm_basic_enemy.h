#ifndef _AIC_BASICENEMY
#define _AIC_BASICENEMY

#include "aifsmcontroller.h"

class fsm_basic_enemy : public aifsmcontroller
{
	CHandle player;
	CHandle character_controller;
	CHandle comp_life;

	int probability_wander;
	int probability_idle;

	float view_distance;
	float forget_distance;

	float attack_distance;
	float out_of_reach_distance;

	float probability_aggresive;
	float probability_mobile;

	float probability_attack1;
	float probability_attack2;

	XMVECTOR wander_target;
	float angular_velocity;
	float velocity;
	float orbit_velocity;
	float orbit_angle;
	float initial_yaw;

	unsigned int my_id;

public:
	fsm_basic_enemy();
	~fsm_basic_enemy();

	void create(string);

	void Idle();
	void Wander();
	void View();
	void Lost();
	void Chase();
	void InitialAttack();
	void IdleWar();
	void AttackSelect();
	void SelectAttack();
	void Attacking1();
	void Attacking2();
	void SelectSide();
	void OrbitRight();
	void OrbitLeft();
	void Ragdoll(float elapsed);
	void Dead(float elapsed);
	void EvaluateHit(float damage);

	bool trueEverySecond();
	bool trueEveryXSeconds(float time);

	void Orbit(bool right);

	void setTarget(CHandle target) { player = target; };

	void setId(unsigned int id);
	unsigned int getInt();
	CHandle getTransform();
};

#endif