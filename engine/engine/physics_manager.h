#ifndef INC_PHYSICS_MANAGER_H_
#define INC_PHYSICS_MANAGER_H_

#include <PxPhysicsAPI.h>
#include <foundation\PxFoundation.h>
#include "mcv_platform.h"

class CPhysicsManager
{
public:

	physx::PxPhysics *gPhysicsSDK;
	physx::PxCooking *gCooking;
	physx::PxScene* gScene;
	physx::PxControllerManager* gManager;

	physx::PxReal timeStep;

	static CPhysicsManager& get();

	void init();

	CPhysicsManager();
	~CPhysicsManager();

	physx::PxVec3 XMVECTORToPxVec3(XMVECTOR vector);
	XMVECTOR PxVec3ToXMVECTOR(physx::PxVec3 vector);

	physx::PxQuat XMVECTORToPxQuat(XMVECTOR quat);
	XMVECTOR PxQuatToXMVECTOR(physx::PxQuat quat);

	std::string toString(physx::PxVec3 vector);
	std::string toString(physx::PxQuat quat);

	bool CPhysicsManager::raycast(physx::PxVec3 origin, physx::PxVec3 unit_dir, physx::PxReal max_distance, physx::PxRaycastBuffer &hit);
	bool CPhysicsManager::raycast(XMVECTOR origin, XMVECTOR unit_dir, physx::PxReal max_distance, physx::PxRaycastBuffer &hit);

};

#endif