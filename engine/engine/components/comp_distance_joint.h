#ifndef INC_COMP_DISTANCE_JOINT_H_
#define INC_COMP_DISTANCE_JOINT_H_

#include "base_component.h"

struct  TCompDistanceJoint : TBaseComponent {

	physx::PxDistanceJoint* joint;

	std::string actor1;
	std::string actor2;

	TCompDistanceJoint() {}

	void loadFromAtts(MKeyValue &atts) {
		actor1 = atts.getString("actor1", "");
		actor2 = atts.getString("actor2", "");
	}

	void create(physx::PxRigidActor* actor1, physx::PxRigidActor* actor2, float damping, float max_distance) {
		joint = physx::PxDistanceJointCreate(*Physics.gPhysicsSDK, actor1, physx::PxTransform(0.5f, 0.5f, 0.5f), actor2, physx::PxTransform(0.0f, 0.0f, 0.0f));

		assert((actor1 != actor2) || fatal("Joint actors must be different"));

		joint->setDamping(damping);
		joint->setMaxDistance(max_distance);
		joint->setStiffness(500);
		joint->setConstraintFlag(physx::PxConstraintFlag::eCOLLISION_ENABLED, true);
		joint->setDistanceJointFlag(physx::PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);
		joint->setDistanceJointFlag(physx::PxDistanceJointFlag::eSPRING_ENABLED, true);
	}

	void init() {
		CEntity* e_a1 = CEntityManager::get().getByName(actor1.c_str());
		CEntity* e_a2 = CEntityManager::get().getByName(actor2.c_str());

		TCompRigidBody* r1 = e_a1->get<TCompRigidBody>();
		TCompRigidBody* r2 = e_a2->get<TCompRigidBody>();

		TCompStaticBody* s1 = e_a1->get<TCompStaticBody>();
		TCompStaticBody* s2 = e_a2->get<TCompStaticBody>();

		//create a joint
		if (r1) {
			if (r2)
				joint = physx::PxDistanceJointCreate(*Physics.gPhysicsSDK, r1->rigidBody, physx::PxTransform(0.5f, 0.5f, 0.5f), r2->rigidBody, physx::PxTransform(0.0f, -0.5f, 0.0f));
			else
				joint = physx::PxDistanceJointCreate(*Physics.gPhysicsSDK, r1->rigidBody, physx::PxTransform(0.5f, 0.5f, 0.5f), s2->staticBody, physx::PxTransform(0.0f, -0.5f, 0.0f));
		}
		else {
			if (s1)
				joint = physx::PxDistanceJointCreate(*Physics.gPhysicsSDK, s1->staticBody, physx::PxTransform(0.5f, 0.5f, 0.5f), r2->rigidBody, physx::PxTransform(0.0f, -0.5f, 0.0f));
			else
				joint = physx::PxDistanceJointCreate(*Physics.gPhysicsSDK, s1->staticBody, physx::PxTransform(0.5f, 0.5f, 0.5f), s2->staticBody, physx::PxTransform(0.0f, -0.5f, 0.0f));
		}

		joint->setDamping(5);
		joint->setStiffness(200);
		joint->setMaxDistance(2.5f);
		joint->setConstraintFlag(physx::PxConstraintFlag::eCOLLISION_ENABLED, true);
		joint->setDistanceJointFlag(physx::PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);
		joint->setDistanceJointFlag(physx::PxDistanceJointFlag::eSPRING_ENABLED, true);
	}
};

#endif