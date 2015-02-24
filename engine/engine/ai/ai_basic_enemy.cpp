#include "mcv_platform.h"
#include "ai_basic_enemy.h"
#include "../entity_manager.h"
#include "../components/all_components.h"

using namespace DirectX;

#define V3DISTANCE(x, y) XMVectorGetX(XMVector3Length(x - y))

ai_basic_enemy::ai_basic_enemy()
{
}


ai_basic_enemy::~ai_basic_enemy()
{
}

void ai_basic_enemy::Init()
{
	// insert all states in the map
	AddState("aibe_Idle", (statehandler)&ai_basic_enemy::Idle);
	AddState("aibe_Wander", (statehandler)&ai_basic_enemy::Wander);
	AddState("aibe_View", (statehandler)&ai_basic_enemy::View);
	AddState("aibe_Lost", (statehandler)&ai_basic_enemy::Lost);
	AddState("aibe_Chase", (statehandler)&ai_basic_enemy::Chase);
	AddState("aibe_InitialAttack", (statehandler)&ai_basic_enemy::InitialAttack);
	AddState("aibe_IdleWar", (statehandler)&ai_basic_enemy::IdleWar);
	AddState("aibe_AttackSelect", (statehandler)&ai_basic_enemy::AttackSelect);
	AddState("aibe_SelectAttack", (statehandler)&ai_basic_enemy::SelectAttack);
	AddState("aibe_Attacking1", (statehandler)&ai_basic_enemy::Attacking1);
	AddState("aibe_Attacking2", (statehandler)&ai_basic_enemy::Attacking2);
	AddState("aibe_SelectSide", (statehandler)&ai_basic_enemy::SelectSide);
	AddState("aibe_OrbitRight", (statehandler)&ai_basic_enemy::OrbitRight);
	AddState("aibe_OrbitLeft", (statehandler)&ai_basic_enemy::OrbitLeft);

	// reset the state
	ChangeState("aibe_Idle");

	// Values
	player = CEntityManager::get().getByName("Player");
	enemy_controller = ((CEntity*)entity)->get<TCompEnemyController>();

	probability_wander = 40;
	probability_idle = 20;

	probability_aggresive = 30;
	probability_mobile = 50;

	probability_attack1 = 30;
	probability_attack2 = 70;

	assert(probability_attack1 + probability_attack2 == 100);

	view_distance = 10;
	forget_distance = 13;

	attack_distance = 3;
	out_of_reach_distance = 5;

	angular_velocity = 4;
	velocity = 20;
	orbit_velocity = 10;

	orbit_angle = deg2rad(120);

	assert(orbit_angle < 180);

	wander_target = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->position;

	// For orbit check purpose
	initial_yaw = 1000;
}

void ai_basic_enemy::Idle() {

	XMVECTOR mPosition = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->position;
	XMVECTOR pPosition = ((TCompTransform*)((CEntity*)player)->get<TCompTransform>())->position;

	// Check probability to wander
	if (trueEverySecond())
	{
		int prob = getRandomNumber(1, 100);
		if (prob < probability_wander) {
			ChangeState("aibe_Wander");
		}
	}

	// Check if player viewed

	if (V3DISTANCE(pPosition, mPosition) < view_distance)
	{
		ChangeState("aibe_View");
	}
}

void ai_basic_enemy::Wander() {

	XMVECTOR mPosition = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->position;

	// Go to the wander_target
	LookAt(wander_target);
	Advance();

	// If wander target reached, set other target
	if (V3DISTANCE(mPosition, wander_target) < 1)
	{
		int x = getRandomNumber(-10, 10);
		int z = getRandomNumber(-10, 10);
		wander_target = XMVectorSet(x, 0, z, 1);
	}

	// Check probability to wander
	if (trueEverySecond())
	{
		int prob = getRandomNumber(1, 100);
		if (prob < probability_idle) {
			ChangeState("aibe_Idle");
		}
	}

	// Check if player viewed
	//if (V3DISTANCE(entity->getPosition(), player->getPosition()) < view_distance)
	
	if (true)
	{
		ChangeState("aibe_View");
	}
}

void ai_basic_enemy::View() {

	XMVECTOR pPosition = ((TCompTransform*)((CEntity*)player)->get<TCompTransform>())->position;

	LookAt(pPosition);

	// Stay until next Second
	if (trueEveryXSeconds(2))
	{
		ChangeState("aibe_Chase");
	}
}

void ai_basic_enemy::Lost() {

	// Stay until next Second
	if (trueEveryXSeconds(3))
	{
		ChangeState("aibe_Idle");
	}
}

void ai_basic_enemy::Chase() {

	XMVECTOR mPosition = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->position;
	XMVECTOR pPosition = ((TCompTransform*)((CEntity*)player)->get<TCompTransform>())->position;

	// Go to the player
	LookAt(pPosition);
	Advance();

	// Check if player reached
	if (V3DISTANCE(mPosition, pPosition) < attack_distance)
	{
		ChangeState("aibe_InitialAttack");
	}

	// Check if player forgeted
	if (V3DISTANCE(mPosition, pPosition) > forget_distance)
	{
		ChangeState("aibe_Lost");
	}
}

void ai_basic_enemy::InitialAttack() {


	// Stay until next Second
	if (trueEveryXSeconds(2))
	{
		ChangeState("aibe_IdleWar");
	}
}

void ai_basic_enemy::IdleWar() {

	XMVECTOR mPosition = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->position;
	XMVECTOR pPosition = ((TCompTransform*)((CEntity*)player)->get<TCompTransform>())->position;
	LookAt(pPosition);

	// Stay until next Second
	if (trueEveryXSeconds(2))
	{
		ChangeState("aibe_AttackSelect");
	}

	// Check if player out of reach
	if (V3DISTANCE(mPosition, pPosition) > out_of_reach_distance)
	{
		ChangeState("aibe_Chase");
	}
}

void ai_basic_enemy::AttackSelect() {

	// Check if attack, orbit or wait
	int prob = getRandomNumber(1, 100);

	// Attack
	if (prob < probability_aggresive)
	{
		ChangeState("aibe_SelectAttack");
	}

	// Orbit
	else if (prob < probability_mobile + probability_aggresive)
	{
		ChangeState("aibe_SelectSide");
	}

	// Wait
	else
	{
		ChangeState("aibe_IdleWar");
	}
}

void ai_basic_enemy::SelectAttack() {

	// Select attack
	int prob = getRandomNumber(1, 100);

	if (prob < probability_attack1)
	{
		ChangeState("aibe_Attacking1");
	}
	else
	{
		ChangeState("aibe_Attacking2");
	}
}

void ai_basic_enemy::Attacking1() {


	// Stay until next Second
	if (trueEveryXSeconds(2))
	{
		ChangeState("aibe_IdleWar");
	}
}

void ai_basic_enemy::Attacking2() {


	// Stay until next Second
	if (trueEveryXSeconds(1))
	{
		ChangeState("aibe_IdleWar");
	}
}

void ai_basic_enemy::SelectSide() {

	// Select side
	int prob = getRandomNumber(1, 100);

	if (prob < 50)
	{
		ChangeState("aibe_OrbitRight");
	}
	else
	{
		ChangeState("aibe_OrbitLeft");
	}
}

void ai_basic_enemy::OrbitRight() {

	XMVECTOR mPosition = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->position;
	XMVECTOR pPosition = ((TCompTransform*)((CEntity*)player)->get<TCompTransform>())->position;

	Orbit(false);
	LookAt(pPosition);

	if (initial_yaw == 1000)
		initial_yaw = getYawFromVector(pPosition - mPosition);
	float currentYaw = getYawFromVector(pPosition - mPosition);

	float angle = XMVectorGetX(XMVector3AngleBetweenVectors(getVectorFromYaw(currentYaw), getVectorFromYaw(initial_yaw)));
	if (angle >= orbit_angle)
	{
		initial_yaw = 1000;
		ChangeState("aibe_IdleWar");
	}
}

void ai_basic_enemy::OrbitLeft() {

	XMVECTOR mPosition = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->position;
	XMVECTOR pPosition = ((TCompTransform*)((CEntity*)player)->get<TCompTransform>())->position;

	Orbit(true);
	LookAt(pPosition);

	if (initial_yaw == 1000)
		initial_yaw = getYawFromVector(pPosition - mPosition);
	float currentYaw = getYawFromVector(pPosition - mPosition);

	float angle = XMVectorGetX(XMVector3AngleBetweenVectors(getVectorFromYaw(currentYaw), getVectorFromYaw(initial_yaw)));
	if (angle >= orbit_angle)
	{
		initial_yaw = 1000;
		ChangeState("aibe_IdleWar");
	}
}

bool ai_basic_enemy::trueEverySecond()
{
	static float counter = 0;
	counter += CApp::get().delta_time;
	if (counter >= 1) {
		counter = 0;
		return true;
	}
	return false;
}

bool ai_basic_enemy::trueEveryXSeconds(float time)
{
	static float counter = 0;
	counter += CApp::get().delta_time;
	if (counter >= time) {
		counter = 0;
		return true;
	}
	return false;
}

void ai_basic_enemy::Advance() {

	XMVECTOR m_front = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->getFront();
	float delta_time = CApp::get().delta_time;

	((TCompEnemyController*)enemy_controller)->addDeltaPos(m_front *velocity * delta_time);
}

void ai_basic_enemy::LookAt(XMVECTOR target) {

	XMVECTOR m_up = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->getUp();
	((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->lookAt(target, m_up);
}

void ai_basic_enemy::Orbit(bool left)
{
	XMVECTOR m_position = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->position;
	XMVECTOR p_position = ((TCompTransform*)((CEntity*)player)->get<TCompTransform>())->position;

	float delta_time = CApp::get().delta_time;

	float radius = 4;
	XMVECTOR dif = p_position - m_position;
	float yaw = getYawFromVector(dif);
	if (left)
		yaw -= orbit_velocity * delta_time;
	else
		yaw += orbit_velocity * delta_time;
	XMVECTOR desiredPos = -getVectorFromYaw(yaw) * attack_distance;
	desiredPos = p_position + desiredPos;

	XMVECTOR speed = desiredPos - m_position;
	speed = XMVector3Normalize(speed);

	((TCompEnemyController*)enemy_controller)->addDeltaPos(speed * orbit_velocity * delta_time);
}