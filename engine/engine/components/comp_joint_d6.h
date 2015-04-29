#ifndef INC_COMP_JOINT_D6_H_
#define INC_COMP_JOINT_D6_H_

#include "base_component.h"
#include "physics_manager.h"

struct  TCompJointD6 : TBaseComponent {
private:
	std::string actor1;
	std::string actor2;

	PxTransform getAnchorConfiguration(PxTransform body_transform, PxVec3 joint_pos, PxQuat joint_rot);
	PxTransform getAxisConfiguration(PxTransform body_transform, PxVec3 joint_pos, PxQuat joint_rot);
public:

	CHandle e_a1;
	CHandle e_a2;
	PxD6Joint* mJoint;
	PxReal linearPosition;

	TCompJointD6() : mJoint(nullptr) {}
	~TCompJointD6();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();
};

#endif