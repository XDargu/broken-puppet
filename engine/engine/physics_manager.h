#ifndef INC_PHYSICS_MANAGER_H_
#define INC_PHYSICS_MANAGER_H_

#include <PxPhysicsAPI.h>
#include <foundation\PxFoundation.h>
#include "mcv_platform.h"

#define Physics CPhysicsManager::get()

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

	bool raycast(physx::PxVec3 origin, physx::PxVec3 unit_dir, physx::PxReal max_distance, physx::PxRaycastBuffer &hit);
	bool raycast(XMVECTOR origin, XMVECTOR unit_dir, physx::PxReal max_distance, physx::PxRaycastBuffer &hit);

	void raycastAll(physx::PxVec3 origin, physx::PxVec3 unit_dir, physx::PxReal max_distance, physx::PxRaycastBuffer &hit);
	void raycastAll(XMVECTOR origin, XMVECTOR unit_dir, physx::PxReal max_distance, physx::PxRaycastBuffer &hit);

};

void setupFiltering(physx::PxRigidActor* actor, physx::PxU32 filterGroup, physx::PxU32 filterMask);
void setupFiltering(physx::PxShape* shape, physx::PxU32 filterGroup, physx::PxU32 filterMask);

#endif