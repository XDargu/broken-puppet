#ifndef INC_COMP_JOINT_FIXED_H_
#define INC_COMP_JOINT_FIXED_H_

#include "base_component.h"
#include "physics_manager.h"

	
struct  TCompJointFixed : TBaseComponent {
private:
	std::string actor1;
	std::string actor2;

public:

	CHandle e_a1;
	CHandle e_a2;
	PxFixedJoint* mJoint;
	PxReal linearPosition;

	TCompJointFixed() : mJoint(nullptr) {}
	~TCompJointFixed();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	PxFixedJoint* getJoint();

	CHandle getActor1();

	CHandle getActor2();

	PxReal getLinealPosition();

	PxTransform getAnchorConfiguration(PxTransform body_transform, PxVec3 joint_pos, PxQuat joint_rot);
	PxTransform getAxisConfiguration(PxTransform body_transform, PxVec3 joint_pos, PxQuat joint_rot);

};

#endif