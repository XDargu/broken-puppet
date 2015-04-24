#ifndef INC_COMP_SWITCH_PULL_CONTROLLER_H_
#define INC_COMP_SWITCH_PULL_CONTROLLER_H_

#include "base_component.h"
#include "physics_manager.h"

/*

It needs a rigidbody and an static body

*/

struct TCompSwitchPullController : TBaseComponent{

	CHandle actor1;
	PxRigidActor* px_actor1;

	CHandle actor2;
	PxRigidActor* px_actor2;	

	float limit;
	bool pressed;

	TCompSwitchPullController() : px_actor1(nullptr), px_actor2(nullptr), pressed(false) { }

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);

	void onPress();

	void onLeave();
};

#endif