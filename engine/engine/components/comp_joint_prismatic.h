#ifndef INC_COMP_JOINT_PRISMATIC_H_
#define INC_COMP_JOINT_PRISMATIC_H_

#include "base_component.h"
#include "physics_manager.h"

struct  TCompJointPrismatic : TBaseComponent {
public:

	CHandle e_a1;
	CHandle e_a2; 
	PxD6Joint* mJoint;
	PxReal linear_position;

	TCompJointPrismatic() : mJoint(nullptr) {}
	~TCompJointPrismatic();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	PxD6Joint* getJoint();

	CHandle getActor1();

	CHandle getActor2();

	PxReal getLinealPosition();
};

#endif