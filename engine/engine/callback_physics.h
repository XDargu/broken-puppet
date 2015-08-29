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
	virtual void							onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count){}
	virtual void							onConstraintBreak(physx::PxConstraintInfo*, physx::PxU32) {}
	virtual void							onWake(physx::PxActor**, physx::PxU32) {}
	virtual void							onSleep(physx::PxActor**, physx::PxU32){}
private:
	physx::PxReal getForce(physx::PxReal mass, const physx::PxContactPair* pairs, PxU32 index);
	physx::PxReal getForceAndPosition(PxReal mass, const PxContactPair* pairs, PxU32 index, XMVECTOR& position, XMVECTOR& normal);
	const physx::PxReal forceLargeImpact;
	const physx::PxReal forceMediumImpact;
	const float impact_threshold_time;
};

class CFilterCallback:public physx::PxSimulationFilterCallback
{
public:
	CFilterCallback();

	//Filter method to specify how a pair of potentially colliding objects should be processed.
	virtual PxFilterFlags 	pairFound(PxU32 pairID, PxFilterObjectAttributes attributes0, PxFilterData filterData0, const PxActor *a0, const PxShape *s0, PxFilterObjectAttributes attributes1, PxFilterData filterData1, const PxActor *a1, const PxShape *s1, PxPairFlags &pairFlags);
	
	//Callback to inform that a tracked collision pair is gone.
	virtual void 	        pairLost(PxU32 pairID, PxFilterObjectAttributes attributes0, PxFilterData filterData0, PxFilterObjectAttributes attributes1, PxFilterData filterData1, bool objectDeleted){};
	
	//Callback to give the opportunity to change the filter state of a tracked collision pair.
	virtual bool 	        statusChange(PxU32 &pairID, PxPairFlags &pairFlags, PxFilterFlags &filterFlags){ return false; };
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



