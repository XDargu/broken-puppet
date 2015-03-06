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

bool already_attacked;
bool already_animated;
bool initial_attack_done;

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

	probability_aggresive = 50;
	probability_mobile = 30;

	probability_attack1 = 30;
	probability_attack2 = 70;

	assert(probability_attack1 + probability_attack2 == 100);

	view_distance = 5;
	forget_distance = 8;

	attack_distance = 1.5f;
	out_of_reach_distance = 3;

	angular_velocity = 0.15f;
	velocity = 3;
	orbit_velocity = 20;

	orbit_angle = deg2rad(120);

	assert(orbit_angle < 180);

	wander_target = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->position;
	comp_mesh = ((CEntity*)entity)->get<TCompMesh>();

	// For orbit check purpose
	initial_yaw = 1000;

	already_attacked = false;
	already_animated = false;
	initial_attack_done = false;
}

void ai_basic_enemy::Idle() {

	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_Idle");
	if (!already_animated) {
		((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_Idle");
		already_animated = true;
	}

	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();
	TCompTransform* p_transform = ((CEntity*)player)->get<TCompTransform>();
	XMVECTOR mFront = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->getFront();

	((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, Physics.XMVECTORToPxVec3(mFront));

	// Check probability to wander
	if (trueEverySecond())
	{
		int prob = getRandomNumber(1, 100);
		if (prob < probability_wander) {
			already_animated = false;
			ChangeState("aibe_Wander");
		}
	}

	// Check if player viewed

	if (V3DISTANCE(m_transform->position, p_transform->position) < view_distance)
	{

		// Check if player is visible
		physx::PxRaycastBuffer buf;
		Physics.raycastAll(m_transform->position + XMVectorSet(0, 1.5f, 0, 0), XMVector3Normalize(p_transform->position - m_transform->position), view_distance, buf);

		bool player_visible = false;
		if (buf.nbTouches > 1)
			if (std::strcmp(buf.touches[1].actor->getName(), "Player") == 0) {
				player_visible = true;
			}

		if (player_visible) {
			already_animated = false;
			ChangeState("aibe_View");
		}
	}
}

void ai_basic_enemy::Wander() {

	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_Walk");
	if (!already_animated) {
		((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_Walk");
		already_animated = true;

	}
	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();
	TCompTransform* p_transform = ((CEntity*)player)->get<TCompTransform>();

	// Go to the wander_target
	physx::PxVec3 front = Physics.XMVECTORToPxVec3(m_transform->getFront());
	((TCompUnityCharacterController*)character_controller)->Move(front, false, false, Physics.XMVECTORToPxVec3(wander_target - m_transform->position));

	// Check if wall in front of me, then set other target
	physx::PxRaycastBuffer buf;
	Physics.raycastAll(m_transform->position + XMVectorSet(0, 0.1f, 0, 0), XMVector3Normalize(wander_target - m_transform->position), 1.f, buf);

	bool can_go = true;

	for (int i = 0; i < buf.nbTouches; i++)
	{		
		if (buf.touches[i].actor != ((TCompUnityCharacterController*)character_controller)->enemy_rigidbody) {
			can_go = false;
		}
	}

	if (!can_go) {
		int x = getRandomNumber(0, 20) - 10;
		int z = getRandomNumber(0, 20) - 10;
		wander_target = m_transform->position + XMVectorSet(x * 2, 0, z * 2, 0);
	}

	// If wander target reached, set other target
	if (V3DISTANCE(m_transform->position, wander_target) < 3)
	{
		int x = getRandomNumber(0, 20) - 10;
		int z = getRandomNumber(0, 20) - 10;
		wander_target = m_transform->position + XMVectorSet(x * 2, 0, z * 2, 0);		
	}

	// Check probability to wander
	if (trueEverySecond())
	{
		int prob = getRandomNumber(1, 100);
		if (prob < probability_idle) {
			already_animated = false;
			ChangeState("aibe_Idle");
		}
	}

	// Check if player viewed
	if (V3DISTANCE(m_transform->position, p_transform->position) < view_distance)
	{
		// Check if player is visible
		physx::PxRaycastBuffer buf;
		Physics.raycastAll(m_transform->position + XMVectorSet(0, 1.5f, 0, 0), XMVector3Normalize(p_transform->position - m_transform->position), view_distance, buf);

		bool player_visible = false;
		if (buf.nbTouches > 1)
		if (std::strcmp(buf.touches[1].actor->getName(), "Player") == 0) {
			player_visible = true;
		}
		/*for (int i = 0; i < buf.nbTouches; i++) {
			if (std::strcmp(buf.touches[i].actor->getName(), "Player") == 0) {
				player_visible = true;
				break;
			}
		}*/

		if (player_visible) {
			already_animated = false;
			ChangeState("aibe_View");
		}
	}
}

void ai_basic_enemy::View() {

	if (!already_animated) {
		((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_IdleWar");
		already_animated = true;
	}

	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();
	TCompTransform* p_transform = ((CEntity*)player)->get<TCompTransform>();
		
	((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, Physics.XMVECTORToPxVec3(p_transform->position - m_transform->position));

	// Stay until next Second
	if (trueEveryXSeconds(2))
	{
		ChangeState("aibe_Chase");
		already_animated = false;
	}
}

void ai_basic_enemy::Lost() {

	initial_attack_done = false;

	if (!already_animated) {
		((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_IdleWar");
		already_animated = true;
	}

	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();

	physx::PxVec3 front = Physics.XMVECTORToPxVec3(m_transform->getFront());
	((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, front);

	// Stay until next Second
	if (trueEveryXSeconds(3))
	{
		ChangeState("aibe_Idle");
		already_animated = false;
	}
}

void ai_basic_enemy::Chase() {

	if (!already_animated) {
		((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_Run");
		already_animated = true;
	}

	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();
	TCompTransform* p_transform = ((CEntity*)player)->get<TCompTransform>();

	// Go to the wander_target
	physx::PxVec3 front = Physics.XMVECTORToPxVec3(m_transform->getFront());
	((TCompUnityCharacterController*)character_controller)->Move(front, false, false, Physics.XMVECTORToPxVec3(p_transform->position - m_transform->position));

	// Check if player reached
	if (V3DISTANCE(m_transform->position, p_transform->position) < attack_distance)
	{
		already_animated = false;
		if (initial_attack_done) {
			ChangeState("aibe_IdleWar");
		}
		else {
			ChangeState("aibe_InitialAttack");
		}
	}

	// Check if player forgeted
	if (V3DISTANCE(m_transform->position, p_transform->position) > forget_distance)
	{
		already_animated = false;
		ChangeState("aibe_Lost");
	}
}

void ai_basic_enemy::InitialAttack() {

	initial_attack_done = true;

	if (!already_animated) {
		((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_Attack");
		already_animated = true;
	}

	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();

	// Go to the wander_target
	physx::PxVec3 front = Physics.XMVECTORToPxVec3(m_transform->getFront());
	((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, front);

	if (!already_attacked) {
		((CEntity*)player)->sendMsg(TMsgAttackDamage(entity, 10));
		already_attacked = true;
	}

	// Stay until next Second
	if (trueEveryXSeconds(2))
	{
		already_attacked = false;
		already_animated = false;
		ChangeState("aibe_IdleWar");
	}
}

void ai_basic_enemy::IdleWar() {

	if (!already_animated) {
		((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_IdleWar");
		already_animated = true;
	}

	XMVECTOR mPosition = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->position;
	XMVECTOR pPosition = ((TCompTransform*)((CEntity*)player)->get<TCompTransform>())->position;

	((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, Physics.XMVECTORToPxVec3(pPosition - mPosition));

	// Stay until next Second
	if (trueEveryXSeconds(2))
	{
		already_animated = false;
		ChangeState("aibe_AttackSelect");
	}

	// Check if player out of reach
	if (V3DISTANCE(mPosition, pPosition) > out_of_reach_distance)
	{
		already_animated = false;
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

	if (!already_animated) {
		((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_Attack");
		already_animated = true;
	}

	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();

	physx::PxVec3 front = Physics.XMVECTORToPxVec3(m_transform->getFront());
	((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, front);

	if (!already_attacked) {
		((CEntity*)player)->sendMsg(TMsgAttackDamage(entity, 10));
		already_attacked = true;
	}

	// Stay until next Second
	if (trueEveryXSeconds(2))
	{
		already_attacked = false;
		already_animated = false;
		ChangeState("aibe_IdleWar");
	}
}

void ai_basic_enemy::Attacking2() {

	if (!already_animated) {
		((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_Attack");
		already_animated = true;
	}

	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();

	physx::PxVec3 front = Physics.XMVECTORToPxVec3(m_transform->getFront());
	((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, front);

	if (!already_attacked) {
		((CEntity*)player)->sendMsg(TMsgAttackDamage(entity, 10));
		already_attacked = true;
	}

	// Stay until next Second
	if (trueEveryXSeconds(1))
	{
		already_attacked = false;
		already_animated = false;
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

	if (!already_animated) {
		((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_Walk");
		already_animated = true;
	}

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
		already_animated = false;
		ChangeState("aibe_IdleWar");
	}
}

void ai_basic_enemy::OrbitLeft() {

	if (!already_animated) {
		((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_Walk");
		already_animated = true;
	}

	XMVECTOR mPosition = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->position;
	XMVECTOR pPosition = ((TCompTransform*)((CEntity*)player)->get<TCompTransform>())->position;

	Orbit(true);

	if (initial_yaw == 1000)
		initial_yaw = getYawFromVector(pPosition - mPosition);
	float currentYaw = getYawFromVector(pPosition - mPosition);

	float angle = XMVectorGetX(XMVector3AngleBetweenVectors(getVectorFromYaw(currentYaw), getVectorFromYaw(initial_yaw)));
	if (angle >= orbit_angle)
	{
		already_animated = false;
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