#ifndef INC_COMP_JOINT_PRISMATIC_H_
#define INC_COMP_JOINT_PRISMATIC_H_

#include "base_component.h"

struct  TCompJointPrismatic : TBaseComponent {
private:
	std::string actor1;
	std::string actor2;
	physx::PxD6Joint* mJoint;

public:

	TCompJointPrismatic() {}
	~TCompJointPrismatic() {

		// Awake the actors
		physx::PxRigidActor* a1 = nullptr;
		physx::PxRigidActor* a2 = nullptr;

		mJoint->getActors(a1, a2);
		// Call the addForce method to awake the bodies, if dynamic
		if (a1->isRigidDynamic()) {
			((physx::PxRigidDynamic*)a1)->wakeUp();
		}
		if (a2->isRigidDynamic()) {
			((physx::PxRigidDynamic*)a2)->wakeUp();
		}

		// Release the joint
		mJoint->release();
	}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {

		Physics.gPhysicsSDK->getVisualDebugger()->setVisualDebuggerFlags(physx::PxVisualDebuggerFlag::eTRANSMIT_CONTACTS | physx::PxVisualDebuggerFlag::eTRANSMIT_CONSTRAINTS);

		//Physics->getVisualDebugger()->setVisualDebuggerFlags(PxVisualDebuggerFlags::eTRANSMIT_CONTACTS | PxVisualDebuggerFlag::eTRANSMIT_CONSTRAINTS);

		// Physics var
		physx::PxReal lower_limit;
		physx::PxReal upper_limit;
		physx::PxReal stiffness;
		physx::PxReal damping;

		actor1 = atts.getString("actor1", "");
		actor2 = atts.getString("actor2", "");

		lower_limit = atts.getFloat("linearPosition", 0.1) / 2;
		upper_limit = atts.getFloat("linearPosition", 0.1) / 2;

		stiffness = atts.getFloat("posSpring", 0.1);
		damping = atts.getFloat("posDamping", 0.1);


		CEntity* owner_entity = (CEntity*)CHandle(this).getOwner();
		const char* nombre = owner_entity->getName();

		CEntity* e_a1;
		CEntity* e_a2;

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

		TCompRigidBody* r1 = NULL;
		TCompRigidBody* r2 = NULL;

		TCompStaticBody* s1 = NULL;
		TCompStaticBody* s2 = NULL;

		if (e_a1)
		{
			r1 = e_a1->get<TCompRigidBody>();
			s1 = e_a1->get<TCompStaticBody>();
		}
		

		if (e_a2)
		{
			r2 = e_a2->get<TCompRigidBody>();
			s2 = e_a2->get<TCompStaticBody>();
		}
		

		//create a joint
		/*
		if (r1) {
			if (r2)
				joint = physx::PxPrismaticJointCreate(*Physics.gPhysicsSDK, r1->rigidBody, physx::PxTransform(0.5f, 0.5f, 0.5f), r2->rigidBody, physx::PxTransform(0.0f, -0.5f, 0.0f));

			else
				joint = physx::PxPrismaticJointCreate(*Physics.gPhysicsSDK, r1->rigidBody, physx::PxTransform(0.5f, 0.5f, 0.5f), s2->staticBody, physx::PxTransform(0.0f, -0.5f, 0.0f));
		}
		else {
			if (s1)
				joint = physx::PxPrismaticJointCreate(*Physics.gPhysicsSDK, s1->staticBody, physx::PxTransform(0.5f, 0.5f, 0.5f), r2->rigidBody, physx::PxTransform(0.0f, -0.5f, 0.0f));
			else
				joint = physx::PxPrismaticJointCreate(*Physics.gPhysicsSDK, s1->staticBody, physx::PxTransform(0.5f, 0.5f, 0.5f), s2->staticBody, physx::PxTransform(0.0f, -0.5f, 0.0f));
		}
		*/
		TCompRigidBody* trans = e_a1->get<TCompRigidBody>();
			
		mJoint = physx::PxD6JointCreate(*Physics.gPhysicsSDK, r1->rigidBody, physx::PxTransform(0.f, 0.f, 0.f), NULL, trans->rigidBody->getGlobalPose());


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
			

		
		*/


		mJoint->setMotion(physx::PxD6Axis::eY, physx::PxD6Motion::eLIMITED);
		mJoint->setLinearLimit(physx::PxJointLinearLimit(0.8f, physx::PxSpring(0, 0)));
		mJoint->setDrive(physx::PxD6Drive::eY, physx::PxD6JointDrive(200, 10, 10000, false));
		mJoint->setDrivePosition(physx::PxTransform(0.f, 0.f, 0.f));


		//mJoint->setMotion(physx::PxD6Axis::eY, physx::PxD6Motion::eLIMITED);
		//mJoint->setLinearLimit(physx::PxJointLinearLimit(0.5f, physx::PxSpring(0, 0)));
		//mJoint->setMotion(physx::PxD6Axis::eX, physx::PxD6Motion::eFREE);
		
		//mJoint->setMotion(physx::PxD6Axis::eZ, physx::PxD6Motion::eFREE);



		//mJoint->setDriveVelocity(physx::PxVec3(10, 10, 0), physx::PxVec3(0, 0, 0));
		//mJoint->setConstraintFlag(physx::PxConstraintFlag::ePROJECTION, true);		
		//mJoint->setConstraintFlag(physx::PxConstraintFlag::eDRIVE_LIMITS_ARE_FORCES, true);
		//physx::PxD6JointDrive cosa = physx::PxD6JointDrive();
		//cosa.damping = 100000000000000;
		//cosa.forceLimit = true;
		//cosa.stiffness = 10000000000000000;
		//physx::PxConstraintFlag::eBROKEN;		

		//mJoint->setDrivePosition(physx::PxTransform(0.f, 0.f, 0.f));

	}

	void init() {

	}
};

#endif