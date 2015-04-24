#ifndef INC_PHYSICS_MANAGER_H_
#define INC_PHYSICS_MANAGER_H_

#include <PxPhysicsAPI.h>
#include <foundation\PxFoundation.h>
#include "mcv_platform.h"
#include "transform.h"
#include "collision_parser.h"

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

	physx::PxPhysics *gPhysicsSDK;
	physx::PxCooking *gCooking;
	physx::PxScene* gScene;
	physx::PxControllerManager* gManager;

	physx::PxReal timeStep;

	static CPhysicsManager& get();

	//typedef std::string(*collision_filter)();

	std::map< physx::PxU32, std::vector<physx::PxU32> >* m_collision;

	void init();

	CPhysicsManager();
	~CPhysicsManager();

	void addCollisionFilter(physx::PxU32 s, physx::PxU32 filter);
	void loadCollisions();

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

	PxTransform transformToPxTransform(TTransform the_transform);
	TTransform pxTransformToTransform(PxTransform the_transform);

};

void setupFiltering(physx::PxRigidActor* actor, physx::PxU32 filterGroup, physx::PxU32 filterMask);
void setupFiltering(physx::PxShape* shape, physx::PxU32 filterGroup, physx::PxU32 filterMask);

#define Physics CPhysicsManager::get()

#endif