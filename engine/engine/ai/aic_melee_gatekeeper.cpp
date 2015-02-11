#include <windows.h>
#include "mcv_platform.h"
#include "aic_melee_gatekeeper.h"
#include "entity.h"

using namespace DirectX;

void aic_melee_gatekeeper::Init()
{
	// insert all states in the map
	AddState("idle", (statehandler)&aic_melee_gatekeeper::IdleState);
	AddState("chase", (statehandler)&aic_melee_gatekeeper::ChaseState);
	AddState("attack", (statehandler)&aic_melee_gatekeeper::AttackState);
	AddState("return_gate", (statehandler)&aic_melee_gatekeeper::ReturnGateState);

	// reset the state
	ChangeState("idle");

	// Values
	view_distance = 10.0f;
	forget_distance = 15.0f;
	in_gate_distance = 1.0f;
	forget_gate_distance = 30.0f;
	attack_distance = 2.0f;
	out_of_reach_distance = 3.0f;
	follow_rotation_velocity = 1.5f;
	chase_speed = 1.0f;
}


void aic_melee_gatekeeper::IdleState(float deltaTime)
{
	XMVECTOR a = player->getPosition();

	if (XMVectorGetX(XMVector3Length(player->getPosition() - entity->getPosition())) < view_distance)
	{
		ChangeState("chase");
	}
	//if (GetAsyncKeyState('V') != 0) { ChangeState("chase"); MessageBox(NULL, TEXT("view: idle->chase"), TEXT("change state"), MB_ICONASTERISK | MB_YESNOCANCEL); }
}

void aic_melee_gatekeeper::ChaseState(float deltaTime)
{
	// Follow
	XMVECTOR delta_vector = player->getPosition() - entity->getPosition();
	XMVECTOR efront = entity->getFront();

	float dot = XMVectorGetX(XMVector3Dot(entity->getLeft(), delta_vector));

	float angle = XMVectorGetX(XMVector3AngleBetweenVectors(delta_vector, efront));

	if (dot < 0)
		angle = -angle;

	float max_angle = follow_rotation_velocity * deltaTime;
	if (angle > max_angle)
		angle = max_angle;

	XMVECTOR q = XMQuaternionRotationAxis(entity->getUp(), angle);
	entity->setRotation(XMQuaternionMultiply(entity->getRotation(), q));

	entity->setPosition(entity->getPosition() + entity->getFront() * chase_speed * deltaTime);

	if (XMVectorGetX(XMVector3Length(player->getPosition() - entity->getPosition())) < attack_distance)
	{
		ChangeState("attack");
	}

	if (XMVectorGetX(XMVector3Length(player->getPosition() - entity->getPosition())) > forget_distance)
	{
		ChangeState("return_gate");
	}

	if (XMVectorGetX(XMVector3Length(gate->getPosition() - entity->getPosition())) > forget_gate_distance)
	{
		ChangeState("return_gate");
	}

	//if (GetAsyncKeyState('C') != 0) { ChangeState("attack"); MessageBox(NULL, TEXT("contact: chase->attack"), TEXT("change state"), MB_ICONASTERISK | MB_YESNOCANCEL); }
	//if (GetAsyncKeyState('F') != 0) { ChangeState("return_gate"); MessageBox(NULL, TEXT("forget: chase->return_gate"), TEXT("change state"), MB_ICONASTERISK | MB_YESNOCANCEL); }
}

void aic_melee_gatekeeper::AttackState(float deltaTime)
{
	if (XMVectorGetX(XMVector3Length(player->getPosition() - entity->getPosition())) > out_of_reach_distance)
	{
		ChangeState("chase");
	}

	//if (GetAsyncKeyState('O') != 0) { ChangeState("chase"); MessageBox(NULL, TEXT("out of reach: attack->chase"), TEXT("change state"), MB_ICONASTERISK | MB_YESNOCANCEL); }
	//if (GetAsyncKeyState('D') != 0) { ChangeState("idle"); MessageBox(NULL, TEXT("dead: attack->idle"), TEXT("change state"), MB_ICONASTERISK | MB_YESNOCANCEL); }
}

void aic_melee_gatekeeper::ReturnGateState(float deltaTime)
{
	// Follow
	XMVECTOR delta_vector = gate->getPosition() - entity->getPosition();
	XMVECTOR efront = entity->getFront();

	float dot = XMVectorGetX(XMVector3Dot(entity->getLeft(), delta_vector));

	float angle = XMVectorGetX(XMVector3AngleBetweenVectors(delta_vector, efront));

	if (dot < 0)
		angle = -angle;

	float max_angle = follow_rotation_velocity * deltaTime;
	if (angle > max_angle)
		angle = max_angle;
	if (angle < -max_angle)
		angle = -max_angle;

	XMVECTOR q = XMQuaternionRotationAxis(entity->getUp(), angle);
	entity->setRotation(XMQuaternionMultiply(entity->getRotation(), q));

	entity->setPosition(entity->getPosition() + entity->getFront() * chase_speed * deltaTime);

	if (XMVectorGetX(XMVector3Length(player->getPosition() - entity->getPosition())) < view_distance)
	{
		ChangeState("chase");
	}

	if (XMVectorGetX(XMVector3Length(gate->getPosition() - entity->getPosition())) < in_gate_distance)
	{
		ChangeState("idle");
	}

	//if (GetAsyncKeyState('V') != 0) { ChangeState("chase"); MessageBox(NULL, TEXT("view: return_gate->chase"), TEXT("change state"), MB_ICONASTERISK | MB_YESNOCANCEL); }
	//if (GetAsyncKeyState('A') != 0) { ChangeState("idle"); MessageBox(NULL, TEXT("at gate: return_gate->idle"), TEXT("change state"), MB_ICONASTERISK | MB_YESNOCANCEL); }
}