#ifndef _AIC_BASICPATROLLER
#define _AIC_BASICPATROLLER

#include "aicontroller.h"

class ai_basic_patroller : public aicontroller
{
public:

	CEntity* player;
	vector<CEntity*> waypoints;
	
	float speed;
	float in_waypoint_distance;
	float follow_rotation_velocity;
	float current_waypoint;

	void ResetWptState(float deltaTime);
	void FollowWpt(float deltaTime);
	void ChangeWpt(float deltaTime);

	void Init();

	void Follow(XMVECTOR target, float delta_time);
};

#endif