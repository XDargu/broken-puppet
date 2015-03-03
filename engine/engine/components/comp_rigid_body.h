#ifndef INC_COMP_RIGID_BODY_H_
#define INC_COMP_RIGID_BODY_H_

#include "base_component.h"

struct TCompRigidBody : TBaseComponent {
private:
	CHandle transform;
public:
	physx::PxRigidDynamic* rigidBody;

	TCompRigidBody() :
		rigidBody(nullptr)
	{}

	~TCompRigidBody() { Physics.gScene->removeActor(*rigidBody); }

	void create(float density, float is_kinematic, float use_gravity) {

		CEntity* e = CHandle(this).getOwner();
		transform = e->get<TCompTransform>();
		TCompCollider* c = e->get<TCompCollider>();
		TCompColliderMesh* mesh_c = e->get<TCompColliderMesh>();

		TCompTransform* trans = (TCompTransform*)transform;

		assert(transform.isValid() || fatal("TRigidBody requieres a TTransform component"));
		assert((c || mesh_c) || fatal("TRigidBody requieres a TCollider or TMeshCollider component"));

		if (c) {
			rigidBody = physx::PxCreateDynamic(
				*Physics.gPhysicsSDK
				, physx::PxTransform(
				Physics.XMVECTORToPxVec3(trans->position),
				Physics.XMVECTORToPxQuat(trans->rotation))
				, *c->collider
				, density);
		}
		if (mesh_c) {
			rigidBody = physx::PxCreateDynamic(
				*Physics.gPhysicsSDK
				, physx::PxTransform(
				Physics.XMVECTORToPxVec3(trans->position),
				Physics.XMVECTORToPxQuat(trans->rotation))
				, *mesh_c->collider
				, density);
		}

		//Asignación de mascara al actor para el filtrado de colisiones
		setupFiltering(rigidBody, FilterGroup::eACTOR, FilterGroup::eACTOR);
		//Asignación de la fuerza minima para hacer hacer saltar el callback de collisiones
		physx::PxReal threshold = 15000.f;
		rigidBody->setContactReportThreshold(threshold);

		Physics.gScene->addActor(*rigidBody);
		setKinematic(is_kinematic);
		setUseGravity(use_gravity);

		// Set the owner entity as the rigidbody user data
		rigidBody->setName(e->getName());
	}

	void loadFromAtts(MKeyValue &atts) {
		float temp_density = atts.getFloat("density", 1);
		bool temp_is_kinematic = atts.getBool("kinematic", false);
		bool temp_use_gravity = atts.getBool("gravity", true);

		CEntity* e = CHandle(this).getOwner();
		transform = e->get<TCompTransform>();
		TCompCollider* c = e->get<TCompCollider>();
		TCompColliderMesh* mesh_c = e->get<TCompColliderMesh>();

		TCompTransform* trans = (TCompTransform*)transform;

		assert(trans || fatal("TRigidBody requieres a TTransform component"));
		assert((c || mesh_c) || fatal("TRigidBody requieres a TCollider or TMeshCollider component"));

		if (c) {
			rigidBody = physx::PxCreateDynamic(
				*Physics.gPhysicsSDK
				, physx::PxTransform(
				Physics.XMVECTORToPxVec3(trans->position),
				Physics.XMVECTORToPxQuat(trans->rotation))
				, *c->collider
				, temp_density);
		}
		if (mesh_c) {
			rigidBody = physx::PxCreateDynamic(
				*Physics.gPhysicsSDK
				, physx::PxTransform(
				Physics.XMVECTORToPxVec3(trans->position),
				Physics.XMVECTORToPxQuat(trans->rotation))
				, *mesh_c->collider
				, temp_density);
		}

		//Asignación de mascara al actor para el filtrado de colisiones
		setupFiltering(rigidBody, FilterGroup::eACTOR, FilterGroup::eACTOR);
		//Asignación de la fuerza minima para hacer hacer saltar el callback de collisiones
		physx::PxReal threshold = 15000.f;
		rigidBody->setContactReportThreshold(threshold);


		Physics.gScene->addActor(*rigidBody);
		setKinematic(temp_is_kinematic);
		setUseGravity(temp_use_gravity);

		// Set the owner entity as the rigidbody user data
		rigidBody->setName(e->getName());
	}

	void init() {
	}

	void fixedUpdate(float elapsed) {
		TCompTransform* trans = (TCompTransform*)transform;

		trans->position = Physics.PxVec3ToXMVECTOR(rigidBody->getGlobalPose().p);
		trans->rotation = Physics.PxQuatToXMVECTOR(rigidBody->getGlobalPose().q);
	}

	XMVECTOR getPosition() {
		return Physics.PxVec3ToXMVECTOR(rigidBody->getGlobalPose().p);
	}

	XMVECTOR getRotation() {
		return Physics.PxQuatToXMVECTOR(rigidBody->getGlobalPose().q);
	}

	void setKinematic(bool is_kinematic) {
		rigidBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, is_kinematic);
	}

	bool isKinematic() {
		return rigidBody->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC);
	}

	void setUseGravity(bool use_gravity) {
		rigidBody->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !use_gravity);
	}

	bool isUsingGravity() {
		return !rigidBody->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_GRAVITY);
	}

	std::string toString() {
		return "Mass: " + std::to_string(rigidBody->getMass()) +
			"\nLinear velocity: " + Physics.toString(rigidBody->getLinearVelocity()) +
			"\nAngular velocity: " + Physics.toString(rigidBody->getAngularVelocity());
	}

	physx::PxReal getMass(){
		return rigidBody->getMass();
	}
};

#endif
