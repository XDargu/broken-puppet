#ifndef INC_COMP_SWITCH_PUSH_CONTROLLER_H_
#define INC_COMP_SWITCH_PUSH_CONTROLLER_H_

#include "base_component.h"

/*

En los interruptores asumimos:

dos elementos rígidos conforman un interruptor de presión móvil, cuando se acercan se activa

un elemento statico y un elemento rígido tienen que formar un interruptor de tensado

un elemento solo, puede formar un interruptor de presión y de tensado

*/

struct TCompSwitchPushController : TBaseComponent{

	CHandle actor1;
	CHandle actor2;
	float limit;
	float initial_distance_squared;
	bool fixed;

	bool pressed;

	int tipe;

	CHandle r1;
	CHandle s1;
	CHandle r2;
	CHandle s2;
	PxTransform Pos1;
	PxTransform Pos2;
	PxTransform init_pos;
	PxTransform actual_pos;

	TCompSwitchPushController() { }

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		// Get the initial values
		// TODO meter assert
		TCompJointPrismatic* joint = ((CEntity*)((CHandle)this).getOwner())->get<TCompJointPrismatic>();
		actor1 = joint->getActor1();
		actor2 = joint->getActor2();
		limit = joint->getLinealPosition();

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
			}
			else if (s1.isValid()){
				Pos1 = ((TCompStaticBody*)(((CEntity*)actor1)->get<TCompStaticBody>()))->staticBody->getGlobalPose();
				tipe = 1;
			}
		}
		else if (actor2.isValid()){
			if (s2.isValid()){
				Pos1 = ((TCompStaticBody*)(((CEntity*)actor2)->get<TCompStaticBody>()))->staticBody->getGlobalPose();
			}
			else if (r2.isValid()){
				Pos1 = ((TCompRigidBody*)(((CEntity*)actor2)->get<TCompRigidBody>()))->rigidBody->getGlobalPose();
			}
			tipe = 2;
			init_pos = Pos1;
		}

		if (actor2.isValid()){
			if (r2.isValid()){
				Pos2 = ((TCompRigidBody*)(((CEntity*)actor2)->get<TCompRigidBody>()))->rigidBody->getGlobalPose();
			}
			if (s2.isValid()){
				Pos2 = ((TCompStaticBody*)(((CEntity*)actor2)->get<TCompStaticBody>()))->staticBody->getGlobalPose();
				tipe = 1;
			}
		}
		else if (actor1.isValid()){

			if (s1.isValid()){
				Pos2 = ((TCompStaticBody*)(((CEntity*)actor1)->get<TCompStaticBody>()))->staticBody->getGlobalPose();
			}
			else if (r1.isValid()){
				Pos2 = ((TCompRigidBody*)(((CEntity*)actor1)->get<TCompRigidBody>()))->rigidBody->getGlobalPose();
			}
			tipe = 2;
			init_pos = Pos2;
		}


		initial_distance_squared = (Pos1.p - Pos2.p).magnitudeSquared();
		int i = 0;
	}

	void update(float elapsed) {
		if (actor1.isValid()){
			if (r1.isValid()){
				Pos1 = ((TCompRigidBody*)(((CEntity*)actor1)->get<TCompRigidBody>()))->rigidBody->getGlobalPose();
			}
			else if (s1.isValid()){
				Pos1 = ((TCompStaticBody*)(((CEntity*)actor1)->get<TCompStaticBody>()))->staticBody->getGlobalPose();
			}
		}
		else if (actor2.isValid()){
			if (s2.isValid()){
				Pos1 = ((TCompStaticBody*)(((CEntity*)actor2)->get<TCompStaticBody>()))->staticBody->getGlobalPose();
			}
			else if (r2.isValid()){
				Pos1 = ((TCompRigidBody*)(((CEntity*)actor2)->get<TCompRigidBody>()))->rigidBody->getGlobalPose();
			}
			actual_pos = Pos1;
		}

		if (actor2.isValid()){
			if (r2.isValid()){
				Pos2 = ((TCompRigidBody*)(((CEntity*)actor2)->get<TCompRigidBody>()))->rigidBody->getGlobalPose();
			}
			if (s2.isValid()){
				Pos2 = ((TCompStaticBody*)(((CEntity*)actor2)->get<TCompStaticBody>()))->staticBody->getGlobalPose();
			}
		}
		else if (actor1.isValid()){
			if (s1.isValid()){
				Pos2 = ((TCompStaticBody*)(((CEntity*)actor1)->get<TCompStaticBody>()))->staticBody->getGlobalPose();
			}
			else if (r1.isValid()){
				Pos2 = ((TCompRigidBody*)(((CEntity*)actor1)->get<TCompRigidBody>()))->rigidBody->getGlobalPose();
			}
			actual_pos = Pos2;
		}

		// if both actors are rigidbodies
		if (tipe == 0){
			if ((pressed == false) && ((Pos1.p - Pos2.p).magnitudeSquared() <= initial_distance_squared / 3)){
				// Llamar a la función de activar
				int i = 0;
				onPress();
				pressed = true;
			}
			else if ((pressed == true) && ((Pos1.p - Pos2.p).magnitudeSquared() > initial_distance_squared / 3)){
				// free
				int i = 0;
				onLeave();
				pressed = false;
			}
		}
		// there are a rigid and a static
		else if (tipe == 1)
		{
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
		else if (tipe == 2)
		{
			float distance = (init_pos.p - actual_pos.p).magnitudeSquared();
			if ((pressed == false) && ((init_pos.p - actual_pos.p).magnitudeSquared() >= (limit) / 2)){
				// Llamar a la función de activar
				onPress();
				pressed = true;
			}
			else if ((pressed == true) && ((init_pos.p - actual_pos.p).magnitudeSquared() < (limit) / 2)){
				// free
				onLeave();
				pressed = false;
			}
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
