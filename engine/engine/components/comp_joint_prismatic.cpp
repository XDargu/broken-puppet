#include "mcv_platform.h"
#include "comp_joint_prismatic.h"
#include "comp_rigid_body.h"
#include "comp_static_body.h"
#include "entity_manager.h"

TCompJointPrismatic::~TCompJointPrismatic() {

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

void TCompJointPrismatic::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	actor1 = atts.getString("actor1", "");
	actor2 = atts.getString("actor2", "");

	PxReal linearModeX = atts.getFloat("linearModeX", 1);
	PxReal linearModeY = atts.getFloat("linearModeY", 1);
	PxReal linearModeZ = atts.getFloat("linearModeZ", 1);

	linearPosition = atts.getFloat("linearPosition", 0);

	PxReal lower_limit = atts.getFloat("linearPosition", 0.1f) / 2.f;
	PxReal upper_limit = atts.getFloat("linearPosition", 0.1f) / 2.f;

	PxReal stiffness = atts.getFloat("posSpring", 0);
	PxReal damping = atts.getFloat("posDamping", 0);

	actor1 = atts.getString("actor1", "");
	actor2 = atts.getString("actor2", "");
	PxVec3 joint_position = Physics.XMVECTORToPxVec3(atts.getPoint("jointPosition"));
	PxQuat joint_rotation = Physics.XMVECTORToPxQuat(atts.getQuat("jointRotation"));

	bool breakable = atts.getBool("breakable", false);
	float breakForce = atts.getFloat("maxBreakForce", 1000);
	float breakTorque = atts.getFloat("maxTorqueForcemaxTorqueForce", 1000);

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
	mJoint = PxD6JointCreate(
		*Physics.gPhysicsSDK
		, m_ridig_Actor1
		, t_0
		, m_ridig_Actor2
		, t_1
		);


	if (breakable) {
		mJoint->setBreakForce(breakForce, breakTorque);
	}

	// Set the axis to locked, limited or free    mode 1 = Locked, 2 = Limited, 3 = Free

	switch ((int)linearModeX)
	{
	case 1:
		/*set de axis as locked*/
		break;
	case 2:
		/*set de axis as limited*/
		mJoint->setMotion(PxD6Axis::eX, PxD6Motion::eLIMITED);
		mJoint->setDrive(PxD6Drive::eX, PxD6JointDrive(stiffness, damping, 10000, false));
		break;
	case 3:
		/*set de axis as free*/
		mJoint->setMotion(PxD6Axis::eX, PxD6Motion::eFREE);
		break;
	default:
		/*set de axis as locked*/
		break;
	}

	switch ((int)linearModeY)
	{
	case 1:
		/*set de axis as locked*/
		break;
	case 2:
		/*set de axis as limited*/
		mJoint->setMotion(PxD6Axis::eY, PxD6Motion::eLIMITED);
		mJoint->setDrive(PxD6Drive::eY, PxD6JointDrive(stiffness, damping, 10000, false));
		break;
	case 3:
		/*set de axis as free*/
		mJoint->setMotion(PxD6Axis::eY, PxD6Motion::eFREE);
		break;
	default:
		/*set de axis as locked*/
		break;
	}

	switch ((int)linearModeZ)
	{
	case 1:
		/*set de axis as locked*/
		break;
	case 2:
		/*set de axis as limited*/
		mJoint->setMotion(PxD6Axis::eZ, PxD6Motion::eLIMITED);
		mJoint->setDrive(PxD6Drive::eZ, PxD6JointDrive(stiffness, damping, 10000, false));
		break;
	case 3:
		/*set de axis as free*/
		mJoint->setMotion(PxD6Axis::eZ, PxD6Motion::eFREE);
		break;
	default:
		/*set de axis as locked*/
		break;
	}
	mJoint->setLinearLimit(PxJointLinearLimit(linearPosition, PxSpring(0, 0)));
	//mJoint->setDrivePosition(drive_position);
}

void TCompJointPrismatic::init() {

}

PxD6Joint* TCompJointPrismatic::getJoint(){
	return mJoint;
}
CHandle TCompJointPrismatic::getActor1(){
	return e_a1;
}
CHandle TCompJointPrismatic::getActor2(){
	return e_a2;
}
PxReal TCompJointPrismatic::getLinealPosition(){
	return linearPosition;
}



// pasos para usar joints d6

/*
crear el objeto d6, relación no la tengo clara, hacer pruebas cambiando el trasnform del segundo actor y del primero
una vez creado se necesita añadir grados de libertad, movement para las translaciones, swing y twist para la rotación

Limites,
se ha de decir qué ejes se pueden trasladar con el límite, ya que el limite a dar será un radio.
los límites se añaden con set linear limit, el primer factor de un linear limit es el radio límite, el spring
añadido se utiliza para permitir que él joint se salga del límite bajo condiciones de spring, (como un soft limit)
poniendo en spring a 0,0 se deja el limite como Hard limit, es decir, no puede franquearse.

Fuerzas, (drive)
dentro del movimiento permitido en el joint, es decir dentre de los límites si los tiene, se pueden aplicar fuerzas.
Para aplicarlas hay que añadir un Drive o empujador al joint, en este estableceremos el eje al que afecta, la dureza
la resistencia al movimiento y la fuerza máxima que puede realizar (cuanto más alejado esté del punto más fuerza ejerce).
En caso de que se quiera aplicar fuerzas en 3 ejes, hay que declarar 3 drives con sus correspondiente valores.

Una vez creado el drive, deben aplicarse fuerzas o puntos a seguir.
con la funcion setDriveVelocity, indicamos a todos los drives del joint un vector direcctor que seguir.
con la función setDrivePoint indicamos a todos los drives del joint que punto deben intentar alcanzar.


los joints, las livertados son en local al objeto que tiene el joint, el drive position funciona de igual modo.
*/