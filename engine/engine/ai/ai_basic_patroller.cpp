#include <windows.h>
#include "mcv_platform.h"
#include "ai_basic_patroller.h"
#include "../entity_manager.h"

using namespace DirectX;

float V3DISTANCE(XMVECTOR x, XMVECTOR y)
{
	return XMVectorGetX(XMVector3Length(x - y));
}

void ai_basic_patroller::Init()
{
	// insert all states in the map
	AddState("reset_wpt", (statehandler)&ai_basic_patroller::ResetWptState);
	AddState("follow_wpt", (statehandler)&ai_basic_patroller::FollowWpt);
	AddState("change_wpt", (statehandler)&ai_basic_patroller::ChangeWpt);

	// reset the state
	ChangeState("reset_wpt");

	// Values
	speed = 5.f;
	in_waypoint_distance = 1.f;
	current_waypoint = 0.0f;
	follow_rotation_velocity = 3.f;
}


void ai_basic_patroller::ResetWptState(float deltaTime)
{
	// Find nearest waypoint
	current_waypoint = 0;
	ChangeState("follow_wpt");

	/*
	// Find nearest waypoint
	float minDist = D3D10_FLOAT32_MAX;
	float minIndex = 0;
	float dist = 0;
	for (int i = 0; i < waypoints.size; ++i)
	{
		dist = V3DISTANCE(entity->position, waypoints[i]->position);
		if (dist < minDist)
		{
			minIndex = i;
			minDist = dist;
		}
	}*/
}

void ai_basic_patroller::FollowWpt(float deltaTime)
{
	float delta_time = 1.0f / 60.0f;

	// Follow
	Follow(waypoints[current_waypoint]->getPosition(), deltaTime);

	// Waypoint reached
	if (V3DISTANCE(entity->getPosition(), waypoints[current_waypoint]->getPosition()) < in_waypoint_distance)
	{
		ChangeState("change_wpt");
	}
}

void ai_basic_patroller::ChangeWpt(float deltaTime)
{
	current_waypoint = (int)(current_waypoint + 1) % waypoints.size();
	ChangeState("follow_wpt");
}

void ai_basic_patroller::Follow(XMVECTOR target, float delta_time) {
	XMVECTOR delta = target - entity->getPosition();
	delta = XMVectorSetY(delta, 0.f);
	XMVECTOR my_front = entity->getFront();
	my_front = XMVectorSetY(my_front, 0.f);

	// Angle returned is always positive!
	float angle = XMVectorGetX(XMVector3AngleBetweenVectors(my_front, delta));

	// If the target is on my right, rotate to the right 
	if (!entity->isInLeft(target))
		angle = -angle;

	// The max angle I can rotate with my current speed
	float max_angle = follow_rotation_velocity * delta_time;
	if (angle > max_angle)
		angle = max_angle;
	else if (angle < -max_angle)
		angle = -max_angle;

	// Delta rotation around world axis Y
	XMVECTOR q = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), angle);
	entity->setRotation(XMQuaternionMultiply(entity->getRotation(), q));

	entity->setPosition(entity->getPosition() + entity->getFront() * speed * delta_time);
}