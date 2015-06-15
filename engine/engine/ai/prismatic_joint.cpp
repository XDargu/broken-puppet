#include "mcv_platform.h"
#include "prismatic_joint.h"
#include "components\comp_joint_prismatic.h"

CPrismaticJoint::CPrismaticJoint(CHandle the_entity) { entity = the_entity; }
CPrismaticJoint::~CPrismaticJoint() {}

// LUA

void CPrismaticJoint::setLinearLimit(float extent, float spring, float damping){
	if (!entity.isValid())
		return;
	TCompJointPrismatic* joint = ((CEntity*)entity)->get<TCompJointPrismatic>();
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

		joint->mJoint->setLinearLimit(PxJointLinearLimit(extent, PxSpring(spring, damping)));
	}
		
}