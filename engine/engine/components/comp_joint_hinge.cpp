#include "mcv_platform.h"
#include "comp_joint_hinge.h"
#include "comp_rigid_body.h"
#include "comp_static_body.h"
#include "entity_manager.h"

TCompJointHinge::~TCompJointHinge() {

	// Awake the actors
	PxRigidActor* a1 = nullptr;
	PxRigidActor* a2 = nullptr;
	

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

void TCompJointHinge::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	if (mJoint)
		mJoint->release();

	//Physics.gPhysicsSDK->getVisualDebugger()->setVisualDebuggerFlags(PxVisualDebuggerFlag::eTRANSMIT_CONTACTS | PxVisualDebuggerFlag::eTRANSMIT_CONSTRAINTS);

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

	actor1 = atts.getString("actor1", "");

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

	// Tenemos que sacar la distancia entre pivotes
	r1 = ((CEntity*)e_a1)->get<TCompRigidBody>();

	// Anchor configuration
	PxVec3 diff_pos;

	// Take the positions in world
	PxTransform c_rigidbody_trans = ((TCompRigidBody*)(((CEntity*)e_a1)->get<TCompRigidBody>()))->rigidBody->getGlobalPose();

	PxTransform anchor = PxTransform(0, 0, 0);
	diff_pos = joint_position - c_rigidbody_trans.p;
	diff_pos = c_rigidbody_trans.q.rotateInv(diff_pos);
	anchor.p = anchor.p + diff_pos;
	
	anchor.q = PxQuat(deg2rad(90), PxVec3(0, 1, 0));
	PxRigidDynamic* m_ridig_dynamic = ((TCompRigidBody*)r1)->rigidBody;
	PxVec3 aux_pos = m_ridig_dynamic->getGlobalPose().p;

	// Axis configuration
	TTransform j_trans = TTransform(Physics.PxVec3ToXMVECTOR(joint_position), Physics.PxQuatToXMVECTOR(joint_rotation), XMVectorSet(1, 1, 1, 0));

	XMVECTOR corrector = XMQuaternionRotationAxis(j_trans.getFront(), deg2rad(-90));

	PxQuat joint_final_rot = Physics.XMVECTORToPxQuat(
			XMQuaternionMultiply(
				  Physics.PxQuatToXMVECTOR(joint_rotation)
				, corrector
			)
		);

	// Create a joint
	mJoint = PxRevoluteJointCreate(*Physics.gPhysicsSDK, m_ridig_dynamic, anchor, NULL, PxTransform(joint_position, joint_final_rot));

	// Set the axis to locked, limited or free  mode 1 = Locked, 2 = Limited, 3 = Free
}

/*
	Cuando el joint cuenta con un solo actor, la primera posición (pos3) determina donde está el joint de forma relativa al actor
	y la segunda posición determina donde está situado el joint y mueve el actor a esa posición.


	A tener en cuenta al modificar la posición relativa del joint es necesario quitar la diferencia de posición y rotación a la posicion 
	del joint

	en el exportador tendríamos que pasar la posición del joint como segundo actor para poder situar
*/
void TCompJointHinge::init() {

}

PxRevoluteJoint* TCompJointHinge::getJoint(){
	return mJoint;
}
CHandle TCompJointHinge::getActor1(){
	return e_a1;
}
CHandle TCompJointHinge::getActor2(){
	return e_a2;
}
PxReal TCompJointHinge::getLinealPosition(){
	return linearPosition;
}