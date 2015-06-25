#include "mcv_platform.h"
#include "comp_joint_d6.h"
#include "comp_rigid_body.h"
#include "comp_static_body.h"
#include "entity_manager.h"

TCompJointD6::~TCompJointD6() {

	if (mJoint) {
		// Awake the actors
		PxRigidActor* a1 = nullptr;
		PxRigidActor* a2 = nullptr;


		mJoint->getActors(a1, a2);
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
			mJoint->release();
	}
}

void TCompJointD6::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	// Release the joint if exists before the loading
	if (mJoint)
		mJoint->release();


	// Physics var
	PxReal lower_limit;
	PxReal upper_limit;
	PxReal stiffness;
	PxReal damping;

	PxRigidActor* actor;

	PxReal linearModeX;
	PxReal linearModeY;
	PxReal linearModeZ;

	PxVec3 joint_position = Physics.XMVECTORToPxVec3(atts.getPoint("jointPosition"));
	PxQuat joint_rotation = Physics.XMVECTORToPxQuat(atts.getQuat("jointRotation"));

	bool breakable = atts.getBool("breakable", false);
	float breakForce = atts.getFloat("maxBreakForce", 1000);
	float breakTorque = atts.getFloat("maxTorqueForcemaxTorqueForce", 1000);

	actor1 = atts.getString("actor1", "");
	actor2 = atts.getString("actor2", "");

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

	// Take the rigidbodies
	r1 = ((CEntity*)e_a1)->get<TCompRigidBody>();
	r2 = ((CEntity*)e_a2)->get<TCompRigidBody>();

	s1 = ((CEntity*)e_a1)->get<TCompStaticBody>();
	s2 = ((CEntity*)e_a2)->get<TCompStaticBody>();

	PxRigidActor* m_ridig_dynamic1 = nullptr;
	PxRigidActor* m_ridig_dynamic2 = nullptr;

	if (r1.isValid()) {
		m_ridig_dynamic1 = ((TCompRigidBody*)r1)->rigidBody;

		if (r2.isValid()) {
			m_ridig_dynamic2 = ((TCompRigidBody*)r2)->rigidBody;
		}
		else if (s2.isValid()) {
			m_ridig_dynamic2 = ((TCompStaticBody*)s2)->staticBody;
		}
	}
	else if (s1.isValid()) {
		m_ridig_dynamic1 = ((TCompStaticBody*)s1)->staticBody;

		if (r2.isValid()) {
			m_ridig_dynamic2 = ((TCompRigidBody*)r2)->rigidBody;
		}
		else if (s2.isValid()) {
			m_ridig_dynamic2 = ((TCompStaticBody*)s2)->staticBody;
		}
	}

	PxTransform t_1 = PxTransform(
		PxVec3(0, 0, 0),
		PxQuat(0, 0, 0, 1)
		);
	PxTransform t_2 = PxTransform(
		PxVec3(0, 0, 0),
		PxQuat(0, 0, 0, 1)
		);

	t_1 = getAnchorConfiguration(m_ridig_dynamic1->getGlobalPose(), joint_position, joint_rotation);
	t_2 = getAnchorConfiguration(m_ridig_dynamic2->getGlobalPose(), joint_position, joint_rotation);
	

	// Create a joint
	mJoint = PxD6JointCreate(
		  *Physics.gPhysicsSDK
		  , m_ridig_dynamic1
		  , t_1
		  , m_ridig_dynamic2
		  , t_2
	);

	//mJoint->set

	mJoint->setMotion(PxD6Axis::eSWING1, PxD6Motion::eLOCKED);
	mJoint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLOCKED);
	mJoint->setMotion(PxD6Axis::eSWING2, PxD6Motion::eLOCKED);

	mJoint->setMotion(PxD6Axis::eX, PxD6Motion::eLOCKED);
	mJoint->setMotion(PxD6Axis::eY, PxD6Motion::eLOCKED);
	mJoint->setMotion(PxD6Axis::eZ, PxD6Motion::eLOCKED);

	PxD6Motion::Enum swing1_motion = (PxD6Motion::Enum)(atts.getInt("swing1Mode", 0) - 1);
	PxD6Motion::Enum swing2_motion = (PxD6Motion::Enum)(atts.getInt("swing2Mode", 0) - 1);
	PxD6Motion::Enum twist_motion = (PxD6Motion::Enum)(atts.getInt("twistMode", 0) - 1);

	float swing1Angle = atts.getFloat("swing1Angle", 45);
	float swing2Angle = atts.getFloat("swing2Angle", 45);
	float twistAngle = atts.getFloat("twistAngle", 45);

	// SWING 1: Rot Y
	// SWING 2: Rot X
	// TWIST: Rot Z
	mJoint->setMotion(PxD6Axis::eSWING1, swing1_motion);
	mJoint->setMotion(PxD6Axis::eSWING2, swing2_motion);
	mJoint->setMotion(PxD6Axis::eTWIST, twist_motion);

	float swing1_spring_val = atts.getFloat("swing1Spring", 0);
	float swing2_spring_val = atts.getFloat("swing2Spring", 0);
	float twist_spring_val = atts.getFloat("twistSpring", 0);

	float swing1_damping_val = atts.getFloat("swing1Damping", 0);
	float swing2_damping_val = atts.getFloat("swing2Damping", 0);
	float twist_damping_val = atts.getFloat("twistDamping", 0);

	PxSpring swing1_spring = PxSpring(swing1_spring_val, swing1_damping_val);
	PxSpring swing2_spring = PxSpring(swing2_spring_val, swing2_damping_val);
	PxSpring twist_spring = PxSpring(twist_spring_val, twist_damping_val);

	// Primer valor: Swing 1 angle / 2, segundo valor: Swing 2 angle / 2
	if (swing1_spring_val == 0)
		mJoint->setSwingLimit(PxJointLimitCone(deg2rad(swing1Angle), deg2rad(swing2Angle), -1));
	else
		mJoint->setSwingLimit(PxJointLimitCone(deg2rad(swing1Angle), deg2rad(swing2Angle), swing1_spring));
	
	// Twist limit
	if (twist_spring_val == 0)
		mJoint->setTwistLimit(PxJointAngularLimitPair(deg2rad(-twistAngle), deg2rad(twistAngle), -1));
	else
		mJoint->setTwistLimit(PxJointAngularLimitPair(deg2rad(-twistAngle), deg2rad(twistAngle), twist_spring));
	
	//mJoint->setMotion(PxD6Axis::eX, PxD6Motion::eLIMITED);
	//mJoint->setDrive(PxD6Drive::eX, PxD6JointDrive(stiffness, damping, 10000, false));	

	// Set the axis to locked, limited or free  mode 1 = Locked, 2 = Limited, 3 = Free

	if (breakable) {
		mJoint->setBreakForce(breakForce, breakTorque);
	}
}

/*
Cuando el joint cuenta con un solo actor, la primera posición (pos3) determina donde está el joint de forma relativa al actor
y la segunda posición determina donde está situado el joint y mueve el actor a esa posición.


A tener en cuenta al modificar la posición relativa del joint es necesario quitar la diferencia de posición y rotación a la posicion
del joint

en el exportador tendríamos que pasar la posición del joint como segundo actor para poder situar
*/
void TCompJointD6::init() {

}

PxTransform TCompJointD6::getAnchorConfiguration(PxTransform body_transform, PxVec3 joint_pos, PxQuat joint_rot) {
	PxVec3 diff_pos;

	PxTransform anchor = PxTransform(0, 0, 0);
	diff_pos = joint_pos - body_transform.p;
	diff_pos = body_transform.q.rotateInv(diff_pos);
	anchor.p = anchor.p + diff_pos;

	anchor.q = PxQuat(deg2rad(90), PxVec3(0, 1, 0));

	return anchor;
}

PxTransform TCompJointD6::getAxisConfiguration(PxTransform body_transform, PxVec3 joint_pos, PxQuat joint_rot) {

	TTransform j_trans = TTransform(Physics.PxVec3ToXMVECTOR(joint_pos), Physics.PxQuatToXMVECTOR(joint_rot), XMVectorSet(1, 1, 1, 0));

	XMVECTOR corrector = XMQuaternionRotationAxis(j_trans.getFront(), deg2rad(-90));

	PxQuat joint_final_rot = Physics.XMVECTORToPxQuat(
		XMQuaternionMultiply(
			Physics.PxQuatToXMVECTOR(joint_rot)
			, corrector
		)
	);

	return PxTransform(joint_pos, joint_final_rot);
}