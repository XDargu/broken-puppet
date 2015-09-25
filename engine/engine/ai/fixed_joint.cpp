#include "mcv_platform.h"
#include "fixed_joint.h"
#include "components\comp_joint_fixed.h"

CFixedJoint::CFixedJoint(CHandle the_entity) { entity = the_entity; }
CFixedJoint::~CFixedJoint() {}

// LUA

void CFixedJoint::breakJoint() {
	if (!entity.isValid())
		return;
	TCompJointFixed* joint = ((CEntity*)entity)->get<TCompJointFixed>();
	if (joint)
	{
		joint->mJoint->setConstraintFlag(PxConstraintFlag::eBROKEN, true);
	}
}