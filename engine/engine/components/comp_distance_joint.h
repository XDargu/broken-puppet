#ifndef INC_COMP_DISTANCE_JOINT_H_
#define INC_COMP_DISTANCE_JOINT_H_

#include "base_component.h"

struct  TCompDistanceJoint : TBaseComponent {

	physx::PxDistanceJoint* joint;
	physx::PxRigidActor* a1;
	physx::PxRigidActor* a2;

	std::string actor1;
	std::string actor2;

	TCompDistanceJoint() {}

	void loadFromAtts(MKeyValue &atts) {
		actor1 = atts.getString("actor1", "");
		actor2 = atts.getString("actor2", "");
	}

	void create(physx::PxRigidActor* actor1, physx::PxRigidActor* actor2, float damping, float max_distance) {
		joint = physx::PxDistanceJointCreate(*Physics.gPhysicsSDK, actor1, physx::PxTransform(0.5f, 0.5f, 0.5f), actor2, physx::PxTransform(0.0f, -0.5f, 0.0f));

		joint->setDamping(damping);
		joint->setMaxDistance(max_distance);
		joint->setConstraintFlag(physx::PxConstraintFlag::eCOLLISION_ENABLED, true);
		joint->setDistanceJointFlag(physx::PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);
	}

	void init() {
		CEntity* a1 = CEntityManager::get().getByName(actor1.c_str());
		CEntity* a2 = CEntityManager::get().getByName(actor2.c_str());

		TCompRigidBody* r1 = a1->get<TCompRigidBody>();
		TCompRigidBody* r2 = a2->get<TCompRigidBody>();

		TCompStaticBody* s1 = a1->get<TCompStaticBody>();
		TCompStaticBody* s2 = a2->get<TCompStaticBody>();

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
		joint->setMaxDistance(2.5f);
		joint->setConstraintFlag(physx::PxConstraintFlag::eCOLLISION_ENABLED, true);
		joint->setDistanceJointFlag(physx::PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);
	}
};

#endif