#ifndef INC_COMP_DISTANCE_JOINT_H_
#define INC_COMP_DISTANCE_JOINT_H_

#include "base_component.h"
#include "physics_manager.h"

struct  TCompDistanceJoint : TBaseComponent {
private:
	std::string actor1;
	std::string actor2;

public:
	physx::PxDistanceJoint* joint;

	TCompDistanceJoint() {}

	~TCompDistanceJoint();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void create(physx::PxRigidActor* actor1, physx::PxRigidActor* actor2, float damping, physx::PxVec3 pos1, physx::PxVec3 pos2, physx::PxTransform offset1, physx::PxTransform offset2);

	void awakeActors();

	void init();
};

#endif