#ifndef _AIC_BASICENEMY
#define _AIC_BASICENEMY

#include "aicontroller.h"

class ai_basic_enemy : public aicontroller
{
	CHandle player;
	CHandle character_controller;

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

public:
	ai_basic_enemy();
	~ai_basic_enemy();

	void Init();

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

	bool trueEverySecond();
	bool trueEveryXSeconds(float time);

	void Orbit(bool right);

	void setTarget(CHandle target) { player = target; };
};

#endif