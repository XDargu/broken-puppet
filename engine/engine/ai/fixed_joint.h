#ifndef _FIXED_JOINT_H_
#define _FIXED_JOINT_H_

#include "handle\handle.h"

// Wrapper LUA Bot class
class CFixedJoint {
private:
	CHandle entity;
public:

	CFixedJoint(CHandle the_entity);
	~CFixedJoint();

	// LUA
	void breakJoint();
};

#endif