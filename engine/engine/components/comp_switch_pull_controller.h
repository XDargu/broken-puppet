#ifndef INC_COMP_SWITCH_PULL_CONTROLLER_H_
#define INC_COMP_SWITCH_PULL_CONTROLLER_H_

#include "base_component.h"

/*

It needs a rigidbody and an static body

*/

struct TCompSwitchPullController : TBaseComponent{

	CHandle actor1;
	PxActor* px_actor1;

	CHandle actor2;
	PxActor* px_actor2;

	float limit;

	bool pressed;

	CHandle r1;
	CHandle s1;
	CHandle r2;
	CHandle s2;

	TCompSwitchPullController() { }

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
	}

	void init(){		
		TCompJointPrismatic* joint = assertRequiredComponent<TCompJointPrismatic>(this);

		actor1 = joint->getActor1();
		px_actor1 = nullptr;

		actor2 = joint->getActor2();
		px_actor2 = nullptr;

		limit = joint->getLinealPosition();

		bool is_static = false;
		bool is_rigid = false;

		pressed = false;

		// check if there are two actors
		if (actor1.isValid())
		{
			r1 = ((CEntity*)actor1)->get<TCompRigidBody>();
			s1 = ((CEntity*)actor1)->get<TCompStaticBody>();
		}
		else {
			// The switch needs two valid bodies in a joint
			assert("The switch needs two valid bodies in a joint");
		}

		if (actor2.isValid())
		{
			r2 = ((CEntity*)actor2)->get<TCompRigidBody>();
			s2 = ((CEntity*)actor2)->get<TCompStaticBody>();
		}
		else {
			// The switch needs two valid bodies in a joint
			assert("The switch needs two valid bodies in a joint");
		}

		if (actor1.isValid()){
			if (r1.isValid()){				
				px_actor1 = ((TCompRigidBody*)(((CEntity*)actor1)->get<TCompRigidBody>()))->rigidBody;
				is_rigid = true;
			}
			else if (s1.isValid()){			
				px_actor1 = ((TCompStaticBody*)(((CEntity*)actor1)->get<TCompStaticBody>()))->staticBody;
				is_static = true;
			}
		}

		if (actor2.isValid()){
			if (r2.isValid()){
				px_actor2 = ((TCompRigidBody*)(((CEntity*)actor2)->get<TCompRigidBody>()))->rigidBody;
				is_rigid = true;
			}
			if (s2.isValid()){
				px_actor2 = ((TCompStaticBody*)(((CEntity*)actor2)->get<TCompStaticBody>()))->staticBody;
				is_static = true;
			}
		}

		if (!(is_static && is_rigid)) assert("the switch pull needs ans static body and an rigidbody");
	}

	void update(float elapsed) {
		PxTransform Pos1;
		PxTransform Pos2;

		if (px_actor1->isRigidBody()){
			Pos1 = ((PxRigidBody*)px_actor1)->getGlobalPose();
		}
		else {
			Pos1 = ((PxRigidStatic*)px_actor1)->getGlobalPose();
		}

		if (px_actor2->isRigidBody()){
			Pos2 = ((PxRigidBody*)px_actor2)->getGlobalPose();
		}
		else {
			Pos2 = ((PxRigidStatic*)px_actor2)->getGlobalPose();
		}

		if ((pressed == false) && ((Pos1.p - Pos2.p).magnitudeSquared() >= (limit*limit) * 3 / 5)){
			// Call on press function
			onPress();
			pressed = true;
		}
		else if ((pressed == true) && ((Pos1.p - Pos2.p).magnitudeSquared() < (limit*limit) * 3 / 5)){
			// Call on press function
			onLeave();
			pressed = false;
		}
	}


	void onPress(){
		int i = 0;
	}

	void onLeave(){
		int i = 0;
	}

	std::string toString() {
		return "Switch";
	}

};


#endif
