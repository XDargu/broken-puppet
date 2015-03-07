#include "mcv_platform.h"
#include "callback_controllers.h"


CCallbacks_controllers::CCallbacks_controllers()
{
}

PxControllerBehaviorFlags CCallbacks_controllers::getBehaviorFlags(const PxShape& shape, const PxActor& actor){
	return physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT | physx::PxControllerBehaviorFlag::eCCT_SLIDE;
}

PxControllerBehaviorFlags CCallbacks_controllers::getBehaviorFlags(const PxController& controller){
	return physx::PxControllerBehaviorFlags(0);
}

PxControllerBehaviorFlags CCallbacks_controllers::getBehaviorFlags(const PxObstacle& obstacle){
	return physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
}
