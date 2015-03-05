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
	character_controller = ((CEntity*)entity)->get<TCompUnityCharacterController>();

	probability_wander = 40;
	probability_idle = 20;

	probability_aggresive = 30;
	probability_mobile = 50;

	probability_attack1 = 30;
	probability_attack2 = 70;

	assert(probability_attack1 + probability_attack2 == 100);

	view_distance = 10;
	forget_distance = 13;

	attack_distance = 1.5f;
	out_of_reach_distance = 3;

	angular_velocity = 0.15f;
	velocity = 3;
	orbit_velocity = 20;

	orbit_angle = deg2rad(120);

	assert(orbit_angle < 180);

	wander_target = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->position;

	// For orbit check purpose
	initial_yaw = 1000;
}

void ai_basic_enemy::Idle() {

	XMVECTOR mPosition = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->position;
	XMVECTOR pPosition = ((TCompTransform*)((CEntity*)player)->get<TCompTransform>())->position;
	XMVECTOR mFront = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->getFront();

	((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, Physics.XMVECTORToPxVec3(mFront));

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

	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();
	TCompTransform* p_transform = ((CEntity*)player)->get<TCompTransform>();

	// Go to the wander_target
	physx::PxVec3 front = Physics.XMVECTORToPxVec3(m_transform->getFront());
	((TCompUnityCharacterController*)character_controller)->Move(front, false, false, Physics.XMVECTORToPxVec3(wander_target - m_transform->position));

	// If wander target reached, set other target
	if (V3DISTANCE(m_transform->position, wander_target) < 1)
	{
		bool can_go = false;
		int count_exit = 0;
		while (!can_go)
		{
			int x = getRandomNumber(-10, 10);
			int z = getRandomNumber(-10, 10);
			wander_target = m_transform->position + XMVectorSet(x, 1, z, 0);
			physx::PxRaycastBuffer hit;
			Physics.raycast(m_transform->position + m_transform->getFront(), m_transform->getFront(), V3DISTANCE(m_transform->position, wander_target), hit);
			if (!hit.hasBlock) {
				can_go = true;
			}
			count_exit++;

			if (count_exit > 100)
			{
				can_go = true;
				ChangeState("aibe_Idle");
			}
		}
		
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
	if (V3DISTANCE(m_transform->position, p_transform->position) < view_distance)
	{
		ChangeState("aibe_View");
	}
}

void ai_basic_enemy::View() {

	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();
	TCompTransform* p_transform = ((CEntity*)player)->get<TCompTransform>();
		
	((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, Physics.XMVECTORToPxVec3(p_transform->position - m_transform->position));

	// Stay until next Second
	if (trueEveryXSeconds(2))
	{
		ChangeState("aibe_Chase");
	}
}

void ai_basic_enemy::Lost() {
	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();

	physx::PxVec3 front = Physics.XMVECTORToPxVec3(m_transform->getFront());
	((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, front);

	// Stay until next Second
	if (trueEveryXSeconds(3))
	{
		ChangeState("aibe_Idle");
	}
}

void ai_basic_enemy::Chase() {

	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();
	TCompTransform* p_transform = ((CEntity*)player)->get<TCompTransform>();

	// Go to the wander_target
	physx::PxVec3 front = Physics.XMVECTORToPxVec3(m_transform->getFront());
	((TCompUnityCharacterController*)character_controller)->Move(front, false, false, Physics.XMVECTORToPxVec3(p_transform->position - m_transform->position));

	// Check if player reached
	if (V3DISTANCE(m_transform->position, p_transform->position) < attack_distance)
	{
		ChangeState("aibe_InitialAttack");
	}

	// Check if player forgeted
	if (V3DISTANCE(m_transform->position, p_transform->position) > forget_distance)
	{
		ChangeState("aibe_Lost");
	}
}

void ai_basic_enemy::InitialAttack() {
	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();

	// Go to the wander_target
	physx::PxVec3 front = Physics.XMVECTORToPxVec3(m_transform->getFront());
	((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, front);

	// Stay until next Second
	if (trueEveryXSeconds(2))
	{
		ChangeState("aibe_IdleWar");
	}
}

void ai_basic_enemy::IdleWar() {

	XMVECTOR mPosition = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->position;
	XMVECTOR pPosition = ((TCompTransform*)((CEntity*)player)->get<TCompTransform>())->position;

	((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, Physics.XMVECTORToPxVec3(pPosition - mPosition));

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
	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();

	physx::PxVec3 front = Physics.XMVECTORToPxVec3(m_transform->getFront());
	((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, front);

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
	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();

	physx::PxVec3 front = Physics.XMVECTORToPxVec3(m_transform->getFront());
	((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, front);

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
	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();

	physx::PxVec3 front = Physics.XMVECTORToPxVec3(m_transform->getFront());
	((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, front);

	// Stay until next Second
	if (trueEveryXSeconds(2))
	{
		ChangeState("aibe_IdleWar");
	}
}

void ai_basic_enemy::Attacking2() {
	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();

	physx::PxVec3 front = Physics.XMVECTORToPxVec3(m_transform->getFront());
	((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, front);

	// Stay until next Second
	if (trueEveryXSeconds(1))
	{
		ChangeState("aibe_IdleWar");
	}
}

void ai_basic_enemy::SelectSide() {
	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();

	physx::PxVec3 front = Physics.XMVECTORToPxVec3(m_transform->getFront());
	((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, front);

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

void ai_basic_enemy::Orbit(bool left)
{
	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();
	TCompTransform* p_transform = ((CEntity*)player)->get<TCompTransform>();

	float delta_time = CApp::get().delta_time;

	float radius = 4;
	XMVECTOR dif = p_transform->position - m_transform->position;
	float yaw = getYawFromVector(dif);
	if (left)
		yaw -= orbit_velocity * delta_time;
	else
		yaw += orbit_velocity * delta_time;
	XMVECTOR desiredPos = -getVectorFromYaw(yaw) * attack_distance;
	desiredPos = p_transform->position + desiredPos;

	XMVECTOR speed = desiredPos - m_transform->position;
	speed = XMVector3Normalize(speed);

	physx::PxVec3 player_pos = Physics.XMVECTORToPxVec3(p_transform->position - m_transform->position);
	((TCompUnityCharacterController*)character_controller)->Move(Physics.XMVECTORToPxVec3(speed), false, false, player_pos);
}