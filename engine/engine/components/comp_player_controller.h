#ifndef INC_COMP_PLAYER_CONTROLLER_H_
#define INC_COMP_PLAYER_CONTROLLER_H_

#include "base_component.h"

struct TCompPlayerController : TBaseComponent {

	TCompPlayerController()
	{}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		
	}

	void init() {

	}

	void update(float elapsed) {

		TCompTransform* trans = getSibling<TCompTransform>(this);
		TCompCharacterController* controller = getSibling<TCompCharacterController>(this);

		if (isKeyPressed('R')) {
			controller->Move(physx::PxVec3(0, 0, 1), false, false, Physics.XMVECTORToPxVec3(trans->getFront()));
		}
		else {
			controller->Move(physx::PxVec3(0, 0, 0), false, false, Physics.XMVECTORToPxVec3(trans->getFront()));
		}

	}

	void fixedUpdate(float elapsed) {

	}
};

#endif
