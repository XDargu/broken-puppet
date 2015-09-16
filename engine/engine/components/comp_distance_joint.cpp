#include "mcv_platform.h"
#include "comp_distance_joint.h"
#include "comp_rigid_body.h"
#include "entity_manager.h"
#include "comp_static_body.h"

TCompDistanceJoint::~TCompDistanceJoint() {

	// Awake the actors
	if (joint) {
		physx::PxRigidActor* a1 = nullptr;
		physx::PxRigidActor* a2 = nullptr;

		joint->getActors(a1, a2);
		// Call the addForce method to awake the bodies, if dynamic
		if (a1 && a1->isRigidDynamic()) {
			if (a1->getScene() != NULL) {
				if (!((physx::PxRigidDynamic*)a1)->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC)) {
					if (((PxRigidDynamic*)a1)->isSleeping())
						((PxRigidDynamic*)a1)->wakeUp();
				}
			}
		}
		if (a2 && a2->isRigidDynamic()) {
			if (a2->getScene() != NULL) {
				if (!((physx::PxRigidDynamic*)a2)->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC)) {
					if (((PxRigidDynamic*)a2)->isSleeping())
						((PxRigidDynamic*)a2)->wakeUp();
				}
			}
		}

		// Release the joint
		if ((a1 && a1->isRigidDynamic()) || (a2 && a2->isRigidDynamic()))
			joint->release();
	}
}

void TCompDistanceJoint::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	actor1 = atts.getString("actor1", "");
	actor2 = atts.getString("actor2", "");

	PxVec3 joint_position = Physics.XMVECTORToPxVec3(atts.getPoint("jointPosition"));
	PxQuat joint_rotation = Physics.XMVECTORToPxQuat(atts.getQuat("jointRotation"));

	bool breakable = atts.getBool("breakable", false);
	float breakForce = atts.getFloat("maxBreakForce", 1000);
	float breakTorque = atts.getFloat("maxTorqueForcemaxTorqueForce", 1000);


	CEntity* owner_entity = (CEntity*)CHandle(this).getOwner();
	const char* nombre = owner_entity->getName();
	// Sustituir los static por null y poner una posicion en la que deberían estar
	if (nombre == actor1){
		e_a1 = owner_entity;
	}
	else {
		e_a1 = CEntityManager::get().getByName(actor1.c_str());
	}
	XASSERT(e_a1.isValid(), "The prismatic joint requires an actor1");
	if (nombre == actor2){
		e_a2 = owner_entity;
	}
	else{
		e_a2 = CEntityManager::get().getByName(actor2.c_str());
	}

	CHandle r1;
	CHandle r2;

	CHandle s1;
	CHandle s2;

	if (e_a1.isValid())
	{
		r1 = ((CEntity*)e_a1)->get<TCompRigidBody>();
		s1 = ((CEntity*)e_a1)->get<TCompStaticBody>();
	}

	if (e_a2.isValid())
	{
		r2 = ((CEntity*)e_a2)->get<TCompRigidBody>();
		s2 = ((CEntity*)e_a2)->get<TCompStaticBody>();
	}

	PxRigidActor* m_ridig_Actor1 = NULL;
	PxRigidActor* m_ridig_Actor2 = NULL;

	if (r1.isValid()) {
		m_ridig_Actor1 = ((TCompRigidBody*)r1)->rigidBody;
	}
	else if (s1.isValid()){
		m_ridig_Actor1 = ((TCompStaticBody*)s1)->staticBody;
	}
	if (r2.isValid()) {
		m_ridig_Actor2 = ((TCompRigidBody*)r2)->rigidBody;
	}
	else if (s2.isValid()){
		m_ridig_Actor2 = ((TCompStaticBody*)s2)->staticBody;
	}

	XMVECTOR corrector = XMQuaternionRotationAxis(XMVectorSet(0, 0, 1, 0), deg2rad(-90));
	PxQuat pxCorrector = Physics.XMVECTORToPxQuat(corrector);
	PxTransform t_corrector = PxTransform(PxVec3(0, 0, 0), pxCorrector);

	// Get the transforms from the joint to the actors
	PxTransform joint_abs = PxTransform(joint_position, joint_rotation);
	joint_abs = joint_abs.transform(t_corrector);

	PxTransform r1_abs = m_ridig_Actor1->getGlobalPose();
	PxTransform r2_abs = m_ridig_Actor2->getGlobalPose();

	r1_abs = r1_abs.getInverse();
	r2_abs = r2_abs.getInverse();

	PxTransform t_0 = r1_abs.transform(joint_abs);
	PxTransform t_1 = r2_abs.transform(joint_abs);

	// Create a joint
	joint = PxDistanceJointCreate(
				  *Physics.gPhysicsSDK
				, m_ridig_Actor1
				, t_0
				, m_ridig_Actor2
				, t_1
				);
		
	if (breakable) {
		joint->setBreakForce(breakForce, breakTorque);
	}

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
	XASSERT(joint, "Joint no existe");
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

	if (joint) {
		joint->getActors(a1, a2);
		// Call the addForce method to awake the bodies, if dynamic
		if (a1 && a1->isRigidDynamic()) {
			if (a1->getScene() != NULL) {
				if (!((physx::PxRigidDynamic*)a1)->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC)) {
					if (((PxRigidDynamic*)a1)->isSleeping())
						((PxRigidDynamic*)a1)->wakeUp();
				}
			}
		}
		if (a2 && a2->isRigidDynamic()) {
			if (a2->getScene() != NULL) {
				if (!((physx::PxRigidDynamic*)a2)->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC)) {
					if (((PxRigidDynamic*)a2)->isSleeping())
						((PxRigidDynamic*)a2)->wakeUp();
				}
			}
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