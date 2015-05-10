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
		eUNDEFINED = (1 << 1),
		eENEMY =     (1 << 2),
		ePLAYER =    (1 << 3),
		eACTOR =     (1 << 4),
		eLEVEL =     (1 << 5),
		ePLAYER_RG = (1 << 6),
		eENEMY_RG  = (1 << 7),
		eACTOR_NON_COLLISION = (1 << 8),
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

	//typedef std::string(*collision_filter)();

	std::map< physx::PxU32, std::vector<physx::PxU32> >* m_collision;

	void init();

	CPhysicsManager();
	~CPhysicsManager();

	void addCollisionFilter(physx::PxU32 s, physx::PxU32 filter);
	void loadCollisions();

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

PxU32 convertStrInCollisionFilter(std::string name);

#define Physics CPhysicsManager::get()

#endif