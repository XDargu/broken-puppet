#include "mcv_platform.h"
#include "prismatic_joint.h"
#include "components\comp_joint_prismatic.h"

CPrismaticJoint::CPrismaticJoint(CHandle the_entity) { entity = the_entity; }
CPrismaticJoint::~CPrismaticJoint() {}

// LUA

void CPrismaticJoint::setLinearLimit(float extent){
	if (!entity.isValid())
		return;
	TCompJointPrismatic* joint = ((CEntity*)entity)->get<TCompJointPrismatic>();
	if (joint)
		joint->mJoint->setLinearLimit(PxJointLinearLimit(extent, PxSpring(0, 0)));
}