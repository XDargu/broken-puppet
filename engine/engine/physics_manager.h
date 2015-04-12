#ifndef INC_PHYSICS_MANAGER_H_
#define INC_PHYSICS_MANAGER_H_

#include <PxPhysicsAPI.h>
#include <foundation\PxFoundation.h>
#include "mcv_platform.h"
#include "transform.h"

using namespace physx;

struct FilterGroup
{
	enum Enum
	{
		ePROTA = (1 << 1),
		eENEMY = (1 << 2),
		eACTOR = (1 << 3),
		eLEVEL = (1 << 4),
	};
};

class CPhysicsManager
{
public:

	PxPhysics *gPhysicsSDK;
	PxCooking *gCooking;
	PxScene* gScene;
	PxControllerManager* gManager;

	PxReal timeStep;

	static CPhysicsManager& get();

	void init();

	CPhysicsManager();
	~CPhysicsManager();

	PxVec3 XMVECTORToPxVec3(XMVECTOR vector);
	XMVECTOR PxVec3ToXMVECTOR(PxVec3 vector);

	PxQuat XMVECTORToPxQuat(XMVECTOR quat);
	XMVECTOR PxQuatToXMVECTOR(PxQuat quat);

	std::string toString(PxVec3 vector);
	std::string toString(PxQuat quat);

	bool raycast(PxVec3 origin, PxVec3 unit_dir, PxReal max_distance, PxRaycastBuffer &hit);
	bool raycast(XMVECTOR origin, XMVECTOR unit_dir, PxReal max_distance, PxRaycastBuffer &hit);

	void raycastAll(PxVec3 origin, PxVec3 unit_dir, PxReal max_distance, PxRaycastBuffer &hit);
	void raycastAll(XMVECTOR origin, XMVECTOR unit_dir, PxReal max_distance, PxRaycastBuffer &hit);

	bool sweepTest(PxGeometry sweepShape, PxTransform initialPose, PxVec3 unit_dir, PxReal max_distance, PxSweepBuffer &hit);
	bool sweepTest(PxGeometry sweepShape, TTransform initialPose, XMVECTOR unit_dir, PxReal max_distance, PxSweepBuffer &hit);

	void sweepTestAll(PxGeometry sweepShape, PxTransform initialPose, PxVec3 unit_dir, PxReal max_distance, PxSweepBuffer &hit);
	void sweepTestAll(PxGeometry sweepShape, TTransform initialPose, XMVECTOR unit_dir, PxReal max_distance, PxSweepBuffer &hit);

	PxTransform transformToPxTransform(TTransform the_transform);
	TTransform pxTransformToTransform(PxTransform the_transform);

};

void setupFiltering(PxRigidActor* actor, PxU32 filterGroup, PxU32 filterMask);
void setupFiltering(PxShape* shape, PxU32 filterGroup, PxU32 filterMask);

#define Physics CPhysicsManager::get()

#endif