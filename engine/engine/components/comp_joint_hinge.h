#ifndef INC_COMP_JOINT_HINGE_H_
#define INC_COMP_JOINT_HINGE_H_

#include "base_component.h"
#include "physics_manager.h"

struct  TCompJointHinge : TBaseComponent {
private:
	std::string actor1;
	std::string actor2;

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
};

#endif