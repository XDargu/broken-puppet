#include "mcv_platform.h"
#include "comp_joint_fixed.h"
#include "comp_rigid_body.h"
#include "comp_static_body.h"
#include "entity_manager.h"

	
TCompJointFixed::~TCompJointFixed() {

	// Awake the actors
	PxRigidActor* a1 = nullptr;
	PxRigidActor* a2 = nullptr;

	if (mJoint) {
		mJoint->getActors(a1, a2);
		// Call the addForce method to awake the bodies, if dynamic
		if (a1 && a1->isRigidDynamic()) {
			((PxRigidDynamic*)a1)->wakeUp();
		}
		if (a2 && a2->isRigidDynamic()) {
			((PxRigidDynamic*)a2)->wakeUp();
		}

		// Release the joint
		mJoint->release();
	}
}

void TCompJointFixed::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	actor1 = atts.getString("actor1", "");
	actor2 = atts.getString("actor2", "");
	PxVec3 joint_position = Physics.XMVECTORToPxVec3(atts.getPoint("jointPosition"));
	PxQuat joint_rotation = Physics.XMVECTORToPxQuat(atts.getQuat("jointRotation"));

	bool breakable = atts.getBool("breakable", false);
	float breakForce = atts.getFloat("maxBreakForce", 1000);
	float breakTorque = atts.getFloat("maxTorqueForcemaxTorqueForce", 1000);

	float angle_limit = atts.getFloat("swingAngle", 0);

	/*PxVec3 joint_rel_pos_0 = Physics.XMVECTORToPxVec3(atts.getPoint("jointRelativePosition0"));
	PxQuat joint_rel_rot_0 = Physics.XMVECTORToPxQuat(atts.getQuat("jointRelativeRotation0"));

	PxVec3 joint_rel_pos_1 = Physics.XMVECTORToPxVec3(atts.getPoint("jointRelativePosition1"));
	PxQuat joint_rel_rot_1 = Physics.XMVECTORToPxQuat(atts.getQuat("jointRelativeRotation1"));

	PxTransform joint_rel_0 = PxTransform(joint_rel_pos_0, joint_rel_rot_0);
	PxTransform joint_rel_1 = PxTransform(joint_rel_pos_1, joint_rel_rot_1);*/

	CEntity* owner_entity = (CEntity*)CHandle(this).getOwner();
	const char* nombre = owner_entity->getName();
	// Sustituir los static por null y poner una posicion en la que deber?an estar
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
	mJoint = PxFixedJointCreate(
		*Physics.gPhysicsSDK
		, m_ridig_Actor1
		, t_0
		, m_ridig_Actor2
		, t_1
		);


	if (breakable) {
		mJoint->setBreakForce(breakForce, breakTorque);
	}

	//mJoint->setDriveForceLimit(100000);
	//mJoint->setDriveVelocity(-1);
	//mJoint->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, true);

}

/*
Cuando el joint cuenta con un solo actor, la primera posici?n (pos3) determina donde est? el joint de forma relativa al actor
y la segunda posici?n determina donde est? situado el joint y mueve el actor a esa posici?n.


A tener en cuenta al modificar la posici?n relativa del joint es necesario quitar la diferencia de posici?n y rotaci?n a la posicion
del joint

en el exportador tendr?amos que pasar la posici?n del joint como segundo actor para poder situar
*/
void TCompJointFixed::init() {

}

PxFixedJoint* TCompJointFixed::getJoint(){
	return mJoint;
}
CHandle TCompJointFixed::getActor1(){
	return e_a1;
}
CHandle TCompJointFixed::getActor2(){
	return e_a2;
}
PxReal TCompJointFixed::getLinealPosition(){
	return linearPosition;
}

PxTransform TCompJointFixed::getAnchorConfiguration(PxTransform body_transform, PxVec3 joint_pos, PxQuat joint_rot) {
	PxVec3 diff_pos;

	PxTransform anchor = PxTransform(0, 0, 0);
	diff_pos = joint_pos - body_transform.p;
	diff_pos = body_transform.q.rotateInv(diff_pos);
	anchor.p = anchor.p + diff_pos;

	anchor.q = PxQuat(deg2rad(90), PxVec3(0, 1, 0));

	return anchor;
}

PxTransform TCompJointFixed::getAxisConfiguration(PxTransform body_transform, PxVec3 joint_pos, PxQuat joint_rot) {

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