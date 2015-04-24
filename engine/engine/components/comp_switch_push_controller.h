#ifndef INC_COMP_SWITCH_PUSH_CONTROLLER_H_
#define INC_COMP_SWITCH_PUSH_CONTROLLER_H_

#include "base_component.h"

/*****************************************************
	This Switch needs a joint with one RigidActor.
******************************************************/

struct TCompSwitchPushController : TBaseComponent{
	
	PxActor* px_actor1;

	float limit;
	PxReal init_up_distance;
	bool pressed;
	
	PxTransform init_pos;
	PxTransform actual_pos;

	TCompSwitchPushController() { }

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
	}

	void init(){

		// This needs a joint component in the owner entity
		TCompJointPrismatic* joint = assertRequiredComponent<TCompJointPrismatic>(this);

		// This switch needs an actor1
		CHandle actor1 = joint->getActor1();
		XASSERT((actor1.isValid()), "The comp push switch needs an actor1.");
		if (actor1.isValid() == false) assert("The comp push switch needs an actor1.");

		// This switch needs just one actor
		CHandle actor2 = joint->getActor2();
		if (actor2.isValid()) assert("The comp push switch just needs one actor, but have two.");

		// check if there are two actors
		CHandle r1 = ((CEntity*)actor1)->get<TCompRigidBody>();
		
		if (r1.isValid() == false) assert("The comp push switch needs a rigidbody actor");
		px_actor1 = ((TCompRigidBody*)(((CEntity*)actor1)->get<TCompRigidBody>()))->rigidBody;

		// If all it´s ok, init the limit, pressed and init_pos
		limit = joint->getLinealPosition();
		pressed = false;
		init_pos = ((PxRigidBody*)px_actor1)->getGlobalPose();

		PxVec3 aux_up = init_pos.q.getBasisVector1();
		PxVec3 aux_p = init_pos.p + aux_up;
		init_up_distance = (init_pos.p - aux_p).magnitude();
	}

	void update(float elapsed) {

		// Have to be a valid rigidbody
		actual_pos = ((PxRigidBody*)px_actor1)->getGlobalPose();

		// Check if pulling or pushing
		PxVec3 aux_up =  init_pos.q.getBasisVector1();
		PxVec3 aux_p = init_pos.p + aux_up;
		PxReal aux_distance = (actual_pos.p - aux_p).magnitude();
		bool pushing = init_up_distance < aux_distance;

		float distance_squared = (init_pos.p - actual_pos.p).magnitudeSquared();
		if (pushing && (pressed == false) && (distance_squared >= (limit*limit) * 3 / 5)){
			// Call onPress function
			onPress();
			pressed = true;
		}
		else if ((pressed == true) && (distance_squared < (limit*limit) * 3 / 5)){
			// Call onPress function
			onLeave();
			pressed = false;
		}

	}

	std::string toString() {
		return "Switch";
	}


	/**************************
		CALL LUA FUNTCIONS
	 **************************/
	void onPress(){
		int i = 0;
	}

	void onLeave(){
		int i = 0;
	}

};


#endif
