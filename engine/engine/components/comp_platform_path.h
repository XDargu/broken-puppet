#ifndef INC_COMP_PLATFORM_PATH_H_
#define INC_COMP_PLATFORM_PATH_H_

#include "base_component.h"

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

	vector<PxVec3> mPath;
	PxVec3 next_pos;

	TCompPlatformPath() { }

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		if (elem == "platformPath") {
			movement_speed = atts.getFloat("speed", 2);
		}
		// Get the list of points
		if (elem == "point") {
			mPath.push_back(Physics.XMVECTORToPxVec3(atts.getPoint("position")));
		}
	}

	void init(){

		follow_line = true;

		next_target = 0;
		distance_enough = 0.5f;

		mRigidbody = assertRequiredComponent<TCompRigidBody>(this);

		// Check if there are a rigid body in the entity
		XASSERT(mRigidbody.isValid(), "The comp platform path requires a ridigbody in the entity");
	}

	void fixedUpdate(float elapsed) {

		if (follow_line != true) return;

		PxRigidDynamic* px_platform = ((TCompRigidBody*)mRigidbody)->rigidBody;		
		
		// Check if we have arrived
		float distance = (px_platform->getGlobalPose().p - mPath[next_target]).magnitude();
		
		if (distance <= distance_enough){
			// If the platform arrived, then change objetive
			next_target++;
			next_target = next_target % mPath.size();
		}

		next_pos = mPath[next_target];

		// Follow the point
		PxTransform my_trans = px_platform->getGlobalPose();

		PxVec3 dir = next_pos - my_trans.p;
		dir.normalize();

		float speed = lerp(distance, movement_speed, .8f);

		my_trans.p = my_trans.p + (dir * speed * elapsed);

		px_platform->setKinematicTarget(my_trans);
	}

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

	std::string toString() {
		return "Switch";
	}
	
};


#endif
