#ifndef INC_COMP_SWITCH_PUSH_CONTROLLER_H_
#define INC_COMP_SWITCH_PUSH_CONTROLLER_H_

#include "base_component.h"
#include "ai\logic_manager.h"

/*****************************************************
	This Switch needs a joint with one RigidActor.
******************************************************/

struct TCompSwitchPushController : TBaseComponent{
	
	PxActor* px_actor1;
	PxActor* px_actor2;

	float limit;
	PxReal init_up_distance;
	bool pressed;
	
	PxVec3 init_offset;

	TCompSwitchPushController() { }

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
	}

	void init(){

		// This needs a joint component in the owner entity
		TCompJointPrismatic* joint = assertRequiredComponent<TCompJointPrismatic>(this);

		PxD6Joint* mPrismaticJoint = joint->getJoint();

		// This switch needs an actor1
		CHandle actor1 = joint->getActor1();
		CHandle actor2 = joint->getActor2();			

		// In case the switch is in the actor1
		if ((CHandle(this).getOwner() == actor1) == false) {
			CHandle actor_aux = actor1;
			actor1 = actor2;
			actor2 = actor_aux;
		}

		CHandle r2 = ((CEntity*)actor2)->get<TCompRigidBody>();
		CHandle s2 = ((CEntity*)actor2)->get<TCompStaticBody>();

		px_actor1 = ((TCompRigidBody*)(((CEntity*)actor1)->get<TCompRigidBody>()))->rigidBody;

		if (r2.isValid()){
			px_actor2 = ((TCompRigidBody*)(((CEntity*)actor2)->get<TCompRigidBody>()))->rigidBody;
		}
		else{
			px_actor2 = ((TCompStaticBody*)(((CEntity*)actor2)->get<TCompStaticBody>()))->staticBody;
		}
		

		// If all it´s ok, init the limit, pressed and init_pos
		limit = joint->getLinealPosition();
		pressed = false;
		
		init_offset = (((PxRigidBody*)px_actor1)->getGlobalPose().p - ((PxRigidBody*)px_actor2)->getGlobalPose().p);

	}

	void update(float elapsed) {

		// Have to be a valid rigidbody
		PxVec3 actual_offset = (((PxRigidBody*)px_actor1)->getGlobalPose().p - ((PxRigidBody*)px_actor2)->getGlobalPose().p);

		float dist_actual = (init_offset - actual_offset).magnitude();

		if ((pressed == false) && (dist_actual >= limit / 3 * 2)){
			// Call onPress function
			onPress();
			pressed = true;
		}
		else if ((pressed == true) && (dist_actual < limit / 3 * 2)){
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
		CLogicManager::get().onSwitchPressed(CHandle(this).getOwner());
	}

	void onLeave(){
		CLogicManager::get().onSwitchReleased(CHandle(this).getOwner());
	}

};


#endif
