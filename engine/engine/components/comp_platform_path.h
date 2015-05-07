#ifndef INC_COMP_PLATFORM_PATH_H_
#define INC_COMP_PLATFORM_PATH_H_

#include "base_component.h"
#include "physics_manager.h"

/*

	This component makes a rigidbody kinematic follow an array of postition points

*/

struct TCompPlatformPath : TBaseComponent{

	/*
		Primero obtenemos el rigid body,
		tiene que ser kinematic.
		*/

	CHandle mRigidbody;		

	bool follow_line;
	int next_target;
	float distance_enough;
	float movement_speed;

	std::vector<PxVec3> mPath;
	PxVec3 next_pos;

	TCompPlatformPath() { }

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void fixedUpdate(float elapsed);

	float lerp(float a, float b, float f)
	{
		return a + f * (b - a);
	}

	void startMoving(){
		follow_line = true;
	}

	void stopMovement(){
		follow_line = false;
	}
	
};


#endif
