#ifndef INC_COMP_JOINT_PRISMATIC_H_
#define INC_COMP_JOINT_PRISMATIC_H_

#include "base_component.h"

struct  TCompJointPrismatic : TBaseComponent {
private:
	std::string actor1;
	std::string actor2;

	

public:

	CHandle e_a1;
	CHandle e_a2; 
	physx::PxD6Joint* mJoint;
	PxReal linearPosition;

	TCompJointPrismatic() : mJoint(nullptr) {}
	~TCompJointPrismatic() {

		// Awake the actors
		physx::PxRigidActor* a1 = nullptr;
		physx::PxRigidActor* a2 = nullptr;

		mJoint->getActors(a1, a2);
		// Call the addForce method to awake the bodies, if dynamic
		if (a1 && a1->isRigidDynamic()) {
			((physx::PxRigidDynamic*)a1)->wakeUp();
		}
		if (a2 && a2->isRigidDynamic()) {
			((physx::PxRigidDynamic*)a2)->wakeUp();
		}

		// Release the joint
		mJoint->release();
	}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {

		if (mJoint)
			mJoint->release();

		Physics.gPhysicsSDK->getVisualDebugger()->setVisualDebuggerFlags(physx::PxVisualDebuggerFlag::eTRANSMIT_CONTACTS | physx::PxVisualDebuggerFlag::eTRANSMIT_CONSTRAINTS);

		//Physics->getVisualDebugger()->setVisualDebuggerFlags(PxVisualDebuggerFlags::eTRANSMIT_CONTACTS | PxVisualDebuggerFlag::eTRANSMIT_CONSTRAINTS);

		// Physics var
		PxReal lower_limit;
		PxReal upper_limit;
		PxReal stiffness;
		PxReal damping;

		PxReal linearModeX;
		PxReal linearModeY;
		PxReal linearModeZ;

		PxReal linearRestitution;

		actor1 = atts.getString("actor1", "");
		actor2 = atts.getString("actor2", "");

		linearModeX = atts.getFloat("linearModeX", 1);
		linearModeY = atts.getFloat("linearModeY", 1);
		linearModeZ = atts.getFloat("linearModeZ", 1);

		linearPosition = atts.getFloat("linearPosition", 0);

		lower_limit = atts.getFloat("linearPosition", 0.1) / 2;
		upper_limit = atts.getFloat("linearPosition", 0.1) / 2;


		stiffness = atts.getFloat("posSpring", 0);
		damping = atts.getFloat("posDamping", 0);


		CEntity* owner_entity = (CEntity*)CHandle(this).getOwner();
		const char* nombre = owner_entity->getName();

		e_a1;
		e_a2;

		// Sustituir los static por null y poner una posicion en la que deberían estar

		if (nombre == actor1){
			e_a1 = owner_entity;
		}
		else{
			e_a1 = CEntityManager::get().getByName(actor1.c_str());
		}

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

		PxTransform Pos1 = PxTransform(0.0f, 0.0f, 0.0f);
		PxTransform Pos2 = PxTransform(0.0f, 0.0f, 0.0f);
		PxTransform drive_position = PxTransform(0.f, 0.f, 0.f);
		
		// Tenemos que sacar la distancia entre pivotes

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
		
		// Take the positions in world
		if (e_a1.isValid()){
			if (r1.isValid()){
				Pos1 = ((TCompRigidBody*)(((CEntity*)e_a1)->get<TCompRigidBody>()))->rigidBody->getGlobalPose();
			}
			if (s1.isValid()){
				Pos1 = ((TCompStaticBody*)(((CEntity*)e_a1)->get<TCompStaticBody>()))->staticBody->getGlobalPose();
			}
		}
		else if (e_a2.isValid()){
			if (s2.isValid()){
				Pos1 = ((TCompStaticBody*)(((CEntity*)e_a2)->get<TCompStaticBody>()))->staticBody->getGlobalPose();
			}
			else if (r2.isValid()){
				Pos1 = ((TCompRigidBody*)(((CEntity*)e_a2)->get<TCompRigidBody>()))->rigidBody->getGlobalPose();
			}
		}

		if (e_a2.isValid()){
			if (r2.isValid()){
				Pos2 = ((TCompRigidBody*)(((CEntity*)e_a2)->get<TCompRigidBody>()))->rigidBody->getGlobalPose();
			}
			if (s2.isValid()){
				((TCompStaticBody*)(((CEntity*)e_a2)->get<TCompStaticBody>()));
				((TCompStaticBody*)(((CEntity*)e_a2)->get<TCompStaticBody>()))->staticBody->getGlobalPose();
				Pos2 = ((TCompStaticBody*)(((CEntity*)e_a2)->get<TCompStaticBody>()))->staticBody->getGlobalPose();
			}
		}
		else if (e_a1.isValid()){
			if (s1.isValid()){
				Pos2 = ((TCompStaticBody*)(((CEntity*)e_a1)->get<TCompStaticBody>()))->staticBody->getGlobalPose();
			}
			else if (r1.isValid()){
				Pos2 = ((TCompRigidBody*)(((CEntity*)e_a1)->get<TCompRigidBody>()))->rigidBody->getGlobalPose();
			}
		}

		//create a joint
		
		if (r1.isValid()) {
			if (r2.isValid())
			{
				mJoint = physx::PxD6JointCreate(*Physics.gPhysicsSDK, ((TCompRigidBody*)r1)->rigidBody, physx::PxTransform(0.f, 0.f, 0.f), ((TCompRigidBody*)r2)->rigidBody, physx::PxTransform(0.f, 0.f, 0.f));
				drive_position.p = Pos2.p - Pos1.p;
				linearPosition = (drive_position.p).magnitude();
			}
			else
				mJoint = physx::PxD6JointCreate(*Physics.gPhysicsSDK, ((TCompRigidBody*)r1)->rigidBody, physx::PxTransform(0.f, 0.f, 0.f), NULL, Pos2);
		}
		else {
			if (s1.isValid())
				mJoint = physx::PxD6JointCreate(*Physics.gPhysicsSDK, NULL, Pos1, ((TCompRigidBody*)r2)->rigidBody, physx::PxTransform(0.f, 0.f, 0.f));
			else
				mJoint = physx::PxD6JointCreate(*Physics.gPhysicsSDK, NULL, Pos2, NULL, Pos2);
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

		// Set the axis to locked, limited or free    mode 1 = Locked, 2 = Limited, 3 = Free

		switch ((int)linearModeX)
		{
		case 1: 
			/*set de axis as locked*/
			break;
		case 2: 
			/*set de axis as limited*/
			mJoint->setMotion(physx::PxD6Axis::eX, physx::PxD6Motion::eLIMITED);
			mJoint->setDrive(physx::PxD6Drive::eX, physx::PxD6JointDrive(stiffness, damping, 10000, false));
			break;
		case 3: 
			/*set de axis as free*/
			mJoint->setMotion(physx::PxD6Axis::eX, physx::PxD6Motion::eFREE);
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
			mJoint->setMotion(physx::PxD6Axis::eY, physx::PxD6Motion::eLIMITED);
			mJoint->setDrive(physx::PxD6Drive::eY, physx::PxD6JointDrive(stiffness, damping, 10000, false));
			break;
		case 3:
			/*set de axis as free*/
			mJoint->setMotion(physx::PxD6Axis::eY, physx::PxD6Motion::eFREE);
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
			mJoint->setMotion(physx::PxD6Axis::eZ, physx::PxD6Motion::eLIMITED);
			mJoint->setDrive(physx::PxD6Drive::eZ, physx::PxD6JointDrive(stiffness, damping, 10000, false));
			break;
		case 3:
			/*set de axis as free*/
			mJoint->setMotion(physx::PxD6Axis::eZ, physx::PxD6Motion::eFREE);
			break;
		default:
			/*set de axis as locked*/
			break;
		}
		mJoint->setLinearLimit(physx::PxJointLinearLimit(linearPosition, physx::PxSpring(0, 0)));
		mJoint->setDrivePosition(drive_position);
	}

	void init() {

	}

	PxD6Joint* getJoint(){
		return mJoint;
	}
	CHandle getActor1(){
		return e_a1;
	}
	CHandle getActor2(){
		return e_a2;
	}
	PxReal getLinealPosition(){
		return linearPosition;
	}
};

#endif