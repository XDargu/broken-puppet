#include "mcv_platform.h"
#include "comp_rigid_body.h"
#include "comp_transform.h"
#include "comp_collider_box.h"
#include "comp_collider_capsule.h"
#include "comp_collider_mesh.h"
#include "comp_collider_sphere.h"
#include "comp_collider_multiple.h"
#include "comp_rope.h"
#include "comp_distance_joint.h"
#include "physics_manager.h"
#include "rope_manager.h"
#include "comp_recast_aabb.h"

void TCompRigidBody::create(float density, bool is_kinematic, bool use_gravity) {

	rigid_entity = CHandle(this).getOwner();
	transform = assertRequiredComponent<TCompTransform>(this);
	TCompColliderBox* box_c = ((CEntity*)rigid_entity)->get<TCompColliderBox>();
	TCompColliderMesh* mesh_c = ((CEntity*)rigid_entity)->get<TCompColliderMesh>();
	TCompColliderSphere* sphere_c = ((CEntity*)rigid_entity)->get<TCompColliderSphere>();
	TCompColliderCapsule* capsule_c = ((CEntity*)rigid_entity)->get<TCompColliderCapsule>();
	TCompColliderConvex* capsule_cvx = ((CEntity*)rigid_entity)->get<TCompColliderConvex>();

	TCompTransform* trans = (TCompTransform*)transform;

	impact_timestamp = 0.f;

	CCollider* col = nullptr;
	if (box_c)
		col = box_c;
	if (mesh_c)
		col = mesh_c;
	if (sphere_c)
		col = sphere_c;
	if (capsule_c)
		col = capsule_c;
	if (capsule_cvx)
		col = capsule_cvx;

	XASSERT(col != nullptr, "TRigidBody requieres a TCollider or TMeshCollider component");

	rigidBody = physx::PxCreateDynamic(
		*Physics.gPhysicsSDK
		, physx::PxTransform(
		Physics.XMVECTORToPxVec3(trans->position),
		Physics.XMVECTORToPxQuat(trans->rotation))
		, *col->collider
		, density);

	//Asignación de la fuerza minima para hacer hacer saltar el callback de collisiones
	physx::PxReal threshold = 410.f;
	rigidBody->setContactReportThreshold(threshold);

	Physics.gScene->addActor(*rigidBody);
	setKinematic(is_kinematic);
	setUseGravity(use_gravity);

	// Set the owner entity as the rigidbody user data
	rigidBody->setName(((CEntity*)rigid_entity)->getName());
	rigidBody->userData = CHandle(this).getOwner().asVoidPtr();

	// Default drag
	rigidBody->setLinearDamping(0.1f);
	rigidBody->setAngularDamping(0.05f);
}

void TCompRigidBody::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	rigid_entity = CHandle(this).getOwner();
	density = atts.getFloat("density", 1);
	bool temp_is_kinematic = atts.getBool("kinematic", false);
	bool temp_use_gravity = atts.getBool("gravity", true);
	boss_level = atts.getInt("bossLevel", 0);

	CEntity* e = CHandle(this).getOwner();
	transform = assertRequiredComponent<TCompTransform>(this);
	TCompColliderBox* box_c = e->get<TCompColliderBox>();
	TCompColliderMesh* mesh_c = e->get<TCompColliderMesh>();
	TCompColliderSphere* sphere_c = e->get<TCompColliderSphere>();
	TCompColliderCapsule* capsule_c = e->get<TCompColliderCapsule>();
	TCompColliderConvex* capsule_cvx = e->get<TCompColliderConvex>();

	TCompColliderMultiple* multiple_c = e->get<TCompColliderMultiple>();

	TCompTransform* trans = (TCompTransform*)transform;

	CCollider* col = nullptr;
	if (box_c){
		col = box_c;
		kind = colliderType::BOX;
	}
	if (mesh_c){
		col = mesh_c;
		kind = colliderType::MESH;
	}
	if (sphere_c){
		col = sphere_c;
		kind = colliderType::SPHERE;
	}
	if (capsule_c){
		col = capsule_c;
		kind = colliderType::CAPSULE;
	}
	if (capsule_cvx){
		col = capsule_cvx;
		kind = colliderType::CONVEX;
	}
	if (multiple_c){
		col = multiple_c;
		kind = colliderType::MULTIPLE;
	}

	XASSERT(col != nullptr, "TRigidBody requieres a Collider component");

	if (multiple_c) {
		rigidBody = physx::PxCreateDynamic(
			*Physics.gPhysicsSDK
			, physx::PxTransform(
			Physics.XMVECTORToPxVec3(trans->position),
			Physics.XMVECTORToPxQuat(XMQuaternionNormalize(trans->rotation)))
			, *multiple_c->colliders[0]
			, density);

		for (int i = 1; i < multiple_c->colliders.size(); i++) {
			rigidBody->attachShape(*multiple_c->colliders[i]);
		}
	}
	else {
		rigidBody = physx::PxCreateDynamic(
			*Physics.gPhysicsSDK
			, physx::PxTransform(
			Physics.XMVECTORToPxVec3(trans->position),
			Physics.XMVECTORToPxQuat(XMQuaternionNormalize(trans->rotation)))
			, *col->collider
			, density);
	}
	//Asignación de la fuerza minima para hacer hacer saltar el callback de collisiones
	physx::PxReal threshold = 410.f;
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

	impact_timestamp = 0;
}

void TCompRigidBody::init() {
}

void TCompRigidBody::fixedUpdate(float elapsed) {
	TCompTransform* trans = (TCompTransform*)transform;

	CEntity* e = CHandle(rigidBody->userData);

	if (auto_translate_transform)
		trans->position = Physics.PxVec3ToXMVECTOR(rigidBody->getGlobalPose().p);

	if (auto_rotate_transform)
		trans->rotation = Physics.PxQuatToXMVECTOR(rigidBody->getGlobalPose().q);

	bool kinematic = rigidBody->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC);

	//Check if rigidBody is inside a recastAABB
	checkIfInsideRecastAABB();

	if (kinematic) { return; }
	if (!e->hasTag("player")) {
		float water_level = CApp::get().water_level;
		float atten = 0.2f;
		float proportion = min(1, (water_level - rigidBody->getGlobalPose().p.y) / atten);

		if (rigidBody->getGlobalPose().p.y < water_level) {
			float volume = rigidBody->getMass() / density;
			float water_density = 500;

			rigidBody->addForce(PxVec3(0, 1, 0) * volume * water_density * 10 * proportion);
			rigidBody->setLinearDamping(1);
			rigidBody->setAngularDamping(0.5f);
		}
		else {
			rigidBody->setLinearDamping(0.05f);
			rigidBody->setAngularDamping(0.05f);
		}
	}

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


void TCompRigidBody::onExplosion(const TMsgExplosion& msg){

	CEntity* e = CHandle(this).getOwner();

	PxVec3 mpos = rigidBody->getGlobalPose().p;
	PxVec3 opos = Physics.XMVECTORToPxVec3(msg.source);
	PxVec3 dir = mpos - opos;
	dir.x = dir.x + 7;
	dir = dir.getNormalized();
	PxVec3 aux_final_force = dir / msg.radius * msg.damage;
	rigidBody->addForce(aux_final_force, PxForceMode::eVELOCITY_CHANGE, true);

}

void TCompRigidBody::checkIfInsideRecastAABB(){
	CEntity* r_entity = ((CEntity*)rigid_entity);
	if (kind == colliderType::BOX){
		TCompColliderBox* col_box = r_entity->get<TCompColliderBox>();
		col_box->checkIfInsideRecastAABB();
	}else if (kind == colliderType::CAPSULE){
		TCompColliderCapsule* col_capsule = r_entity->get<TCompColliderCapsule>();
		col_capsule->checkIfInsideRecastAABB();
	}else if (kind == colliderType::CONVEX){
		TCompColliderConvex* col_convex = r_entity->get<TCompColliderConvex>();
		col_convex->checkIfInsideRecastAABB();
	}else if (kind == colliderType::SPHERE){
		TCompColliderSphere* col_sphere = r_entity->get<TCompColliderSphere>();
		col_sphere->checkIfInsideRecastAABB();
	}
}