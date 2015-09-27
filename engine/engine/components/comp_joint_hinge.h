#ifndef INC_COMP_JOINT_HINGE_H_
#define INC_COMP_JOINT_HINGE_H_

#include "base_component.h"
#include "physics_manager.h"

struct  TCompJointHinge : TBaseComponent {
public:

	CHandle e_a1;
	CHandle e_a2;
	PxRevoluteJoint* mJoint;
	PxReal linearPosition;

	TCompJointHinge() : mJoint(nullptr) {}
	~TCompJointHinge();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	PxRevoluteJoint* getJoint();

	CHandle getActor1();

	CHandle getActor2();

	PxReal getLinealPosition();

	PxTransform getAnchorConfiguration(PxTransform body_transform, PxVec3 joint_pos, PxQuat joint_rot);
	PxTransform getAxisConfiguration(PxTransform body_transform, PxVec3 joint_pos, PxQuat joint_rot);

};

#endif