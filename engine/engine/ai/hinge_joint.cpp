#include "mcv_platform.h"
#include "hinge_joint.h"
#include "components\comp_joint_hinge.h"

CHingeJoint::CHingeJoint(CHandle the_entity) { entity = the_entity; }
CHingeJoint::~CHingeJoint() {}

// LUA

void CHingeJoint::setMotor(float velocity, float force_limit){
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
			if (!((physx::PxRigidDynamic*)a1)->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
				((PxRigidDynamic*)a1)->wakeUp();
		}
		if (a2 && a2->isRigidDynamic()) {
			if (!((physx::PxRigidDynamic*)a2)->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
				((PxRigidDynamic*)a2)->wakeUp();
		}

		joint->mJoint->setDriveForceLimit(force_limit);
		joint->mJoint->setDriveVelocity(velocity);
		
		if (velocity == 0 && force_limit == 0){
			joint->mJoint->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, false);
		}
		else{
			joint->mJoint->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);
		}
	}


}

void CHingeJoint::setLimit(float angle_limit){
	if (!entity.isValid())
		return;
	TCompJointHinge* joint = ((CEntity*)entity)->get<TCompJointHinge>();
	if (joint)
	{
		// Awake the actors
		physx::PxRigidActor* a1 = nullptr;
		physx::PxRigidActor* a2 = nullptr;
		
		PxJoint* mjoint = joint->getJoint();
		joint->getJoint()->getActors(a1, a2);

		// Call the addForce method to awake the bodies, if dynamic
		if (a1 && a1->isRigidDynamic()) {
			if (!((physx::PxRigidDynamic*)a1)->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
				((PxRigidDynamic*)a1)->wakeUp();
		}
		if (a2 && a2->isRigidDynamic()) {
			if (!((physx::PxRigidDynamic*)a2)->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
				((PxRigidDynamic*)a2)->wakeUp();
		}

		if (angle_limit != 0)
		{
			PxJointAngularLimitPair limit = PxJointAngularLimitPair(deg2rad(-angle_limit), deg2rad(angle_limit));
			joint->getJoint()->setLimit(limit);
			joint->getJoint()->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);

		}
		else{
			joint->getJoint()->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, false);
		}
	}
}

void CHingeJoint::setLimit(float angle_lower_limit, float angle_upper_limit) {
	if (!entity.isValid())
		return;
	TCompJointHinge* joint = ((CEntity*)entity)->get<TCompJointHinge>();
	if (joint)
	{
		// Awake the actors
		physx::PxRigidActor* a1 = nullptr;
		physx::PxRigidActor* a2 = nullptr;

		PxJoint* mjoint = joint->getJoint();
		joint->getJoint()->getActors(a1, a2);

		// Call the addForce method to awake the bodies, if dynamic
		if (a1 && a1->isRigidDynamic()) {
			if (!((physx::PxRigidDynamic*)a1)->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
				((PxRigidDynamic*)a1)->wakeUp();
		}
		if (a2 && a2->isRigidDynamic()) {
			if (!((physx::PxRigidDynamic*)a2)->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
				((PxRigidDynamic*)a2)->wakeUp();
		}

		if ((angle_lower_limit == 0) && (angle_upper_limit == 0))
		{
			joint->getJoint()->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, false);
		}
		else{
			PxJointAngularLimitPair limit = PxJointAngularLimitPair(deg2rad(-angle_lower_limit), deg2rad(angle_upper_limit));
			joint->getJoint()->setLimit(limit);
			joint->getJoint()->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, true);			
		}
	}
}