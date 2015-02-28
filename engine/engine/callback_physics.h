#pragma once

#ifndef INC_CALLBACKS_PHYSX_H_
#define INC_CALLBACKS_PHYSX_H_

#include <PxPhysicsAPI.h>
#include <foundation\PxFoundation.h>
#include <PxSimulationEventCallback.h>
#include "PxFiltering.h"
#include "mcv_platform.h"

using namespace physx;

class CCallbacks_physx : public physx::PxSimulationEventCallback
{
public:

	CCallbacks_physx();

	// Implements PxSimulationEventCallback
	virtual void							onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs);
	virtual void							onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count);
	virtual void							onConstraintBreak(physx::PxConstraintInfo*, physx::PxU32) {}
	virtual void							onWake(physx::PxActor**, physx::PxU32) {}
	virtual void							onSleep(physx::PxActor**, physx::PxU32){}
};

PxFilterFlags FilterShader(
	PxFilterObjectAttributes attributes0,
	PxFilterData filterData0,
	PxFilterObjectAttributes attributes1,
	PxFilterData filterData1,
	PxPairFlags& pairFlags,
	const void* constantBlock,
	PxU32 constantBlockSize);

#endif



