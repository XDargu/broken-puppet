#include "mcv_platform.h"
#include "hinge_joint.h"
#include "components\comp_joint_hinge.h"

CHingeJoint::CHingeJoint(CHandle the_entity) { entity = the_entity; }
CHingeJoint::~CHingeJoint() {}

// LUA

void CHingeJoint::setMotor(float velocity, float limit){
	if (!entity.isValid())
		return;
	TCompJointHinge* joint = ((CEntity*)entity)->get<TCompJointHinge>();
	if (joint)
	{
		// Awake the actors
		physx::PxRigidActor* a1 = nullptr;
		physx::PxRigidActor* a2 = nullptr;

		joint->mJoint->getActors(a1, a2);

		// Call the addForce method to awake the bodies, if dynamic
		if (a1 && a1->isRigidDynamic()) {
			((PxRigidDynamic*)a1)->wakeUp();
		}
		if (a2 && a2->isRigidDynamic()) {
			((PxRigidDynamic*)a2)->wakeUp();
		}

		joint->mJoint->setDriveVelocity(velocity);
		joint->mJoint->setDriveForceLimit(limit);
	}

}