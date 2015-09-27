#ifndef INC_COMP_DISTANCE_JOINT_H_
#define INC_COMP_DISTANCE_JOINT_H_

#include "base_component.h"
#include "physics_manager.h"

struct  TCompDistanceJoint : TBaseComponent {
public:
	physx::PxDistanceJoint* joint;
	CHandle e_a1;
	CHandle e_a2;

	TCompDistanceJoint() {}

	~TCompDistanceJoint();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void create(physx::PxRigidActor* actor1, physx::PxRigidActor* actor2, float damping, physx::PxVec3 pos1, physx::PxVec3 pos2, physx::PxTransform offset1, physx::PxTransform offset2);

	void awakeActors();

	void init();
};

#endif