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
	bool fixed;

	bool pressed;

	int tipe;

	CHandle r1;
	CHandle s1;
	CHandle r2;
	CHandle s2;
	PxTransform Pos1;
	PxTransform Pos2;

	TCompSwitchPullController() { }

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
	}

	void init(){
		// Get the initial values
		// TODO meter assert
		TCompJointPrismatic* joint = ((CEntity*)((CHandle)this).getOwner())->get<TCompJointPrismatic>();
		actor1 = joint->getActor1();
		actor2 = joint->getActor2();
		limit = joint->getLinealPosition();

		bool is_static = false;
		bool is_rigid = false;

		pressed = false;
		tipe = 0;

		// check if there are two actors
		if (actor1.isValid())
		{
			r1 = ((CEntity*)actor1)->get<TCompRigidBody>();
			s1 = ((CEntity*)actor1)->get<TCompStaticBody>();
		}

		if (actor2.isValid())
		{
			r2 = ((CEntity*)actor2)->get<TCompRigidBody>();
			s2 = ((CEntity*)actor2)->get<TCompStaticBody>();
		}

		if (actor1.isValid()){
			if (r1.isValid()){
				Pos1 = ((TCompRigidBody*)(((CEntity*)actor1)->get<TCompRigidBody>()))->rigidBody->getGlobalPose();
				is_rigid = true;
			}
			else if (s1.isValid()){
				Pos1 = ((TCompStaticBody*)(((CEntity*)actor1)->get<TCompStaticBody>()))->staticBody->getGlobalPose();
				is_static = true;
			}
		}
		else {
			// The switch needs two valid bodies in a joint
			assert("The switch needs two valid bodies in a joint");
		}

		if (actor2.isValid()){
			if (r2.isValid()){
				Pos2 = ((TCompRigidBody*)(((CEntity*)actor2)->get<TCompRigidBody>()))->rigidBody->getGlobalPose();
				is_rigid = true;
			}
			if (s2.isValid()){
				Pos2 = ((TCompStaticBody*)(((CEntity*)actor2)->get<TCompStaticBody>()))->staticBody->getGlobalPose();
				is_static = true;
			}
		}
		else {
			// The switch needs two valid bodies in a joint
			assert("The switch needs two valid bodies in a joint");
		}

		if (!(is_static && is_rigid)) assert("the switch pull needs ans static body and an rigidbody");
	}

	void update(float elapsed) {

		PxActor* actor;
		if (r1.isValid()){
			Pos1 = ((TCompRigidBody*)(((CEntity*)actor1)->get<TCompRigidBody>()))->rigidBody->getGlobalPose();
			actor = ((TCompRigidBody*)(((CEntity*)actor1)->get<TCompRigidBody>()))->rigidBody;
			actor->getType();
			PxActorType::eRIGID_DYNAMIC;
			actor->isRigidBody();
			PxTransform pos_aux1 = ((PxRigidBody*)actor)->getGlobalPose();
		}
		else if (s1.isValid()){
			Pos1 = ((TCompStaticBody*)(((CEntity*)actor1)->get<TCompStaticBody>()))->staticBody->getGlobalPose();
		}

		if (r2.isValid()){
			Pos2 = ((TCompRigidBody*)(((CEntity*)actor2)->get<TCompRigidBody>()))->rigidBody->getGlobalPose();
			actor = ((TCompRigidBody*)(((CEntity*)actor2)->get<TCompRigidBody>()))->rigidBody;
			actor->getType();
			PxActorType::eRIGID_DYNAMIC;
			actor->isRigidBody();
			PxTransform pos_aux1 = ((PxRigidBody*)actor)->getGlobalPose();
		}
		if (s2.isValid()){
			Pos2 = ((TCompStaticBody*)(((CEntity*)actor2)->get<TCompStaticBody>()))->staticBody->getGlobalPose();
		}

		// there are a rigid and a static
		if ((pressed == false) && ((Pos1.p - Pos2.p).magnitudeSquared() >= (limit*limit) * 3 / 5)){
			// Llamar a la función de activar
			int i = 0;
			onPress();
			pressed = true;
		}
		else if ((pressed == true) && ((Pos1.p - Pos2.p).magnitudeSquared() < (limit*limit) * 3 / 5)){
			// free
			int i = 0;
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
