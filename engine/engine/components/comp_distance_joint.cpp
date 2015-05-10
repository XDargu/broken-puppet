#include "mcv_platform.h"
#include "comp_distance_joint.h"
#include "comp_rigid_body.h"
#include "entity_manager.h"
#include "comp_static_body.h"

TCompDistanceJoint::~TCompDistanceJoint() {

	// Awake the actors
	physx::PxRigidActor* a1 = nullptr;
	physx::PxRigidActor* a2 = nullptr;

	joint->getActors(a1, a2);
	// Call the addForce method to awake the bodies, if dynamic
	if (a1 && a1->isRigidDynamic()) {
		((physx::PxRigidDynamic*)a1)->wakeUp();
	}
	if (a2 && a2->isRigidDynamic()) {
		((physx::PxRigidDynamic*)a2)->wakeUp();
	}

	// Release the joint
	joint->release();
}

void TCompDistanceJoint::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	actor1 = atts.getString("actor1", "");
	actor2 = atts.getString("actor2", "");
}

void TCompDistanceJoint::create(physx::PxRigidActor* actor1, physx::PxRigidActor* actor2, float damping, physx::PxVec3 pos1, physx::PxVec3 pos2, physx::PxTransform offset1, physx::PxTransform offset2) {

	SET_ERROR_CONTEXT("Creating a distance joint", "");
	// If any actor is static, then attack the joint to he air instead of the actor
	if (actor1 && actor1->isRigidDynamic()) {
		if (actor2 && actor2->isRigidDynamic()) {
			// Dynamic dynamic
			joint = physx::PxDistanceJointCreate(*Physics.gPhysicsSDK, actor1, offset1, actor2, offset2);
		}
		else {
			// Dynamic static
			joint = physx::PxDistanceJointCreate(*Physics.gPhysicsSDK, actor1, offset1, NULL, physx::PxTransform(pos2));
		}
	}
	else {
		if (actor2 && actor2->isRigidDynamic()) {
			// Static dynamic
			joint = physx::PxDistanceJointCreate(*Physics.gPhysicsSDK, NULL, physx::PxTransform(pos1), actor2, offset2);
		}
		else {
			// Static static -- ¡No se puede!
			joint = physx::PxDistanceJointCreate(*Physics.gPhysicsSDK, NULL, physx::PxTransform(pos1), NULL, physx::PxTransform(pos2));
		}

	}
	if (Physics.gPhysicsSDK->getPvdConnectionManager())
		Physics.gPhysicsSDK->getVisualDebugger()->setVisualDebuggerFlags(physx::PxVisualDebuggerFlag::eTRANSMIT_CONTACTS | physx::PxVisualDebuggerFlag::eTRANSMIT_CONSTRAINTS);
	XASSERT((actor1 != actor2), "Joint actors must be different");

	float dist_between_positions = (pos1 - pos2).magnitude();
	joint->setDamping(damping);
	joint->setMaxDistance(dist_between_positions);
	joint->setStiffness(500);
	joint->setConstraintFlag(physx::PxConstraintFlag::eCOLLISION_ENABLED, true);
	joint->setDistanceJointFlag(physx::PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);
	joint->setDistanceJointFlag(physx::PxDistanceJointFlag::eSPRING_ENABLED, true);

	awakeActors();
}

void TCompDistanceJoint::awakeActors() {
	// Awake the actors
	physx::PxRigidActor* a1 = nullptr;
	physx::PxRigidActor* a2 = nullptr;

	joint->getActors(a1, a2);
	// Call the addForce method to awake the bodies, if dynamic
	if (a1) {
		if (a1->isRigidDynamic()) {
			((physx::PxRigidDynamic*)a1)->wakeUp();
		}
	}
	if (a2)
	{
		if (a2->isRigidDynamic()) {
			((physx::PxRigidDynamic*)a2)->wakeUp();
		}
	}
}

void TCompDistanceJoint::init() {
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