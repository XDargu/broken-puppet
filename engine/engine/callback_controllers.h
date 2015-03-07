#pragma once

#ifndef INC_CALLBACKS_CONTROLLERS_H_
#define INC_CALLBACKS_CONTROLLERS_H_

#include <PxPhysicsAPI.h>
#include <foundation\PxFoundation.h>
#include <PxSimulationEventCallback.h>
#include "PxFiltering.h"
#include "mcv_platform.h"

using namespace physx;

class CCallbacks_controllers : public physx::PxControllerBehaviorCallback
{
public:

	CCallbacks_controllers();

	// Implements PxControllerBehaviorCallback
	virtual PxControllerBehaviorFlags getBehaviorFlags(const PxShape& shape, const PxActor& actor);
	virtual PxControllerBehaviorFlags getBehaviorFlags(const PxController& controller);
	virtual PxControllerBehaviorFlags getBehaviorFlags(const PxObstacle& obstacle);
};

/*PxFilterFlags FilterShader(
	PxFilterObjectAttributes attributes0,
	PxFilterData filterData0,
	PxFilterObjectAttributes attributes1,
	PxFilterData filterData1,
	PxPairFlags& pairFlags,
	const void* constantBlock,
	PxU32 constantBlockSize);*/

#endif





