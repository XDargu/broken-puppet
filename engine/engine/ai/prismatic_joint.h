#ifndef _PRISMATIC_JOINT_H_
#define _PRISMATIC_JOINT_H_

#include "handle\handle.h"

// Wrapper LUA Bot class
class CPrismaticJoint {
private:
	CHandle entity;
public:

	CPrismaticJoint(CHandle the_entity);
	~CPrismaticJoint();

	// LUA
	void setLinearLimit(float extent, float spring, float damping);
	
};

#endif