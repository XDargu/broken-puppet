#include "mcv_platform.h"
#include "comp_rigid_body.h"
#include "comp_transform.h"
#include "comp_collider_box.h"
#include "comp_collider_capsule.h"
#include "comp_collider_mesh.h"
#include "comp_collider_sphere.h"
#include "physics_manager.h"

void TCompRigidBody::create(float density, bool is_kinematic, bool use_gravity) {

	CEntity* e = CHandle(this).getOwner();
	transform = assertRequiredComponent<TCompTransform>(this);
	TCompColliderBox* box_c = e->get<TCompColliderBox>();
	TCompColliderMesh* mesh_c = e->get<TCompColliderMesh>();
	TCompColliderSphere* sphere_c = e->get<TCompColliderSphere>();
	TCompColliderCapsule* capsule_c = e->get<TCompColliderCapsule>();

	TCompTransform* trans = (TCompTransform*)transform;

	CCollider* col = nullptr;
	if (box_c)
		col = box_c;
	if (mesh_c)
		col = mesh_c;
	if (sphere_c)
		col = sphere_c;
	if (capsule_c)
		col = capsule_c;

	XASSERT(col != nullptr, "TRigidBody requieres a TCollider or TMeshCollider component");

	rigidBody = physx::PxCreateDynamic(
		*Physics.gPhysicsSDK
		, physx::PxTransform(
		Physics.XMVECTORToPxVec3(trans->position),
		Physics.XMVECTORToPxQuat(trans->rotation))
		, *col->collider
		, density);

	//Asignación de la fuerza minima para hacer hacer saltar el callback de collisiones
	physx::PxReal threshold = 15000.f;
	rigidBody->setContactReportThreshold(threshold);

	Physics.gScene->addActor(*rigidBody);
	setKinematic(is_kinematic);
	setUseGravity(use_gravity);

	// Set the owner entity as the rigidbody user data
	rigidBody->setName(e->getName());
	rigidBody->userData = CHandle(this).getOwner().asVoidPtr();
}

void TCompRigidBody::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	float temp_density = atts.getFloat("density", 1);
	bool temp_is_kinematic = atts.getBool("kinematic", false);
	bool temp_use_gravity = atts.getBool("gravity", true);

	CEntity* e = CHandle(this).getOwner();
	transform = assertRequiredComponent<TCompTransform>(this);
	TCompColliderBox* box_c = e->get<TCompColliderBox>();
	TCompColliderMesh* mesh_c = e->get<TCompColliderMesh>();
	TCompColliderSphere* sphere_c = e->get<TCompColliderSphere>();
	TCompColliderCapsule* capsule_c = e->get<TCompColliderCapsule>();

	TCompTransform* trans = (TCompTransform*)transform;

	CCollider* col = nullptr;
	if (box_c)
		col = box_c;
	if (mesh_c)
		col = mesh_c;
	if (sphere_c)
		col = sphere_c;
	if (capsule_c)
		col = capsule_c;

	XASSERT(col != nullptr, "TRigidBody requieres a TCollider or TMeshCollider component");

	rigidBody = physx::PxCreateDynamic(
		*Physics.gPhysicsSDK
		, physx::PxTransform(
		Physics.XMVECTORToPxVec3(trans->position),
		Physics.XMVECTORToPxQuat(trans->rotation))
		, *col->collider
		, temp_density);

	//Asignación de la fuerza minima para hacer hacer saltar el callback de collisiones
	physx::PxReal threshold = 15000.f;
	rigidBody->setContactReportThreshold(threshold);


	Physics.gScene->addActor(*rigidBody);
	setKinematic(temp_is_kinematic);
	setUseGravity(temp_use_gravity);

	// Block the rigidbody, if needed
	block_joint = PxD6JointCreate(*Physics.gPhysicsSDK, rigidBody, physx::PxTransform::createIdentity(), NULL, rigidBody->getGlobalPose());

	bool lock_x_pos = atts.getBool("lockXPos", false);
	bool lock_y_pos = atts.getBool("lockYPos", false);
	bool lock_z_pos = atts.getBool("lockZPos", false);

	bool lock_x_rot = atts.getBool("lockXRot", false);
	bool lock_y_rot = atts.getBool("lockYRot", false);
	bool lock_z_rot = atts.getBool("lockZRot", false);

	block_joint->setMotion(physx::PxD6Axis::eX, lock_x_pos ? physx::PxD6Motion::eLOCKED : physx::PxD6Motion::eFREE);
	block_joint->setMotion(physx::PxD6Axis::eY, lock_y_pos ? physx::PxD6Motion::eLOCKED : physx::PxD6Motion::eFREE);
	block_joint->setMotion(physx::PxD6Axis::eZ, lock_z_pos ? physx::PxD6Motion::eLOCKED : physx::PxD6Motion::eFREE);

	block_joint->setMotion(physx::PxD6Axis::eTWIST, lock_x_rot ? physx::PxD6Motion::eLOCKED : physx::PxD6Motion::eFREE);
	block_joint->setMotion(physx::PxD6Axis::eSWING1, lock_y_rot ? physx::PxD6Motion::eLOCKED : physx::PxD6Motion::eFREE);
	block_joint->setMotion(physx::PxD6Axis::eSWING2, lock_z_rot ? physx::PxD6Motion::eLOCKED : physx::PxD6Motion::eFREE);

	// Set the owner entity as the rigidbody user data
	rigidBody->setName(e->getName());
	rigidBody->userData = CHandle(this).getOwner().asVoidPtr();
}

void TCompRigidBody::init() {
}

void TCompRigidBody::fixedUpdate(float elapsed) {
	TCompTransform* trans = (TCompTransform*)transform;

	if (auto_translate_transform)
		trans->position = Physics.PxVec3ToXMVECTOR(rigidBody->getGlobalPose().p);

	if (auto_rotate_transform)
		trans->rotation = Physics.PxQuatToXMVECTOR(rigidBody->getGlobalPose().q);

}

XMVECTOR TCompRigidBody::getPosition() {
	return Physics.PxVec3ToXMVECTOR(rigidBody->getGlobalPose().p);
}

XMVECTOR TCompRigidBody::getRotation() {
	return Physics.PxQuatToXMVECTOR(rigidBody->getGlobalPose().q);
}

void TCompRigidBody::setKinematic(bool is_kinematic) {
	rigidBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, is_kinematic);
}

bool TCompRigidBody::isKinematic() {
	return rigidBody->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC);
}

void TCompRigidBody::setUseGravity(bool use_gravity) {
	rigidBody->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !use_gravity);
}

bool TCompRigidBody::isUsingGravity() {
	return !rigidBody->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_GRAVITY);
}

void TCompRigidBody::setLockXPos(bool locked) {
	block_joint->setMotion(physx::PxD6Axis::eX, locked ? physx::PxD6Motion::eLOCKED : physx::PxD6Motion::eFREE);
}

void TCompRigidBody::setLockYPos(bool locked) {
	block_joint->setMotion(physx::PxD6Axis::eY, locked ? physx::PxD6Motion::eLOCKED : physx::PxD6Motion::eFREE);
}

void TCompRigidBody::setLockZPos(bool locked) {
	block_joint->setMotion(physx::PxD6Axis::eZ, locked ? physx::PxD6Motion::eLOCKED : physx::PxD6Motion::eFREE);
}

void TCompRigidBody::setLockXRot(bool locked) {
	block_joint->setMotion(physx::PxD6Axis::eTWIST, locked ? physx::PxD6Motion::eLOCKED : physx::PxD6Motion::eFREE);
}

void TCompRigidBody::setLockYRot(bool locked) {
	block_joint->setMotion(physx::PxD6Axis::eSWING1, locked ? physx::PxD6Motion::eLOCKED : physx::PxD6Motion::eFREE);
}

void TCompRigidBody::setLockZRot(bool locked) {
	block_joint->setMotion(physx::PxD6Axis::eSWING2, locked ? physx::PxD6Motion::eLOCKED : physx::PxD6Motion::eFREE);
}

physx::PxReal TCompRigidBody::getMass(){
	return rigidBody->getMass();
}