#ifndef _HINGE_JOINT_H_
#define _HINGE_JOINT_H_

#include "handle\handle.h"

// Wrapper LUA Bot class
class CHingeJoint {
private:
	CHandle entity;
public:

	CHingeJoint(CHandle the_entity);
	~CHingeJoint();

	// LUA
	void setMotor(float velocity, float force_limit);
	void setLimit(float limit);
};

#endif