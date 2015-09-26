#include "mcv_platform.h"
#include "comp_transform.h"
#include "comp_static_body.h"
#include "comp_collider_box.h"
#include "comp_collider_mesh.h"
#include "comp_collider_capsule.h"
#include "comp_collider_convex.h"
#include "comp_collider_multiple.h"

TCompStaticBody::~TCompStaticBody() { Physics.gScene->removeActor(*staticBody); }

void TCompStaticBody::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	h_transform = assertRequiredComponent<TCompTransform>(this);

	static_entity = CHandle(this).getOwner();
	TCompColliderBox* box_c = ((CEntity*)static_entity)->get<TCompColliderBox>();
	TCompColliderMesh* mesh_c = ((CEntity*)static_entity)->get<TCompColliderMesh>();
	TCompColliderConvex* capsule_cvx = ((CEntity*)static_entity)->get<TCompColliderConvex>();

	TCompColliderSphere* sphere_c = ((CEntity*)static_entity)->get<TCompColliderSphere>();
	TCompColliderCapsule* capsule_c = ((CEntity*)static_entity)->get<TCompColliderCapsule>();

	TCompColliderMultiple* multiple_c = ((CEntity*)static_entity)->get<TCompColliderMultiple>();

	TCompTransform* trans = (TCompTransform*)h_transform;

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
		staticBody = physx::PxCreateStatic(
			*Physics.gPhysicsSDK
			, physx::PxTransform(
			Physics.XMVECTORToPxVec3(trans->position),
			Physics.XMVECTORToPxQuat(XMQuaternionNormalize(trans->rotation)))
			, *(*multiple_c->colliders)[0]
			);

		for (int i = 1; i < multiple_c->colliders->size(); i++) {
			staticBody->attachShape(*(*multiple_c->colliders)[i]);
		}
	}
	else {
		staticBody = physx::PxCreateStatic(
			*Physics.gPhysicsSDK
			, physx::PxTransform(
			Physics.XMVECTORToPxVec3(trans->position),
			Physics.XMVECTORToPxQuat(XMQuaternionNormalize(trans->rotation)))
			, *col->collider
			);
	}
		
	Physics.gScene->addActor(*staticBody);

	staticBody->setName(((CEntity*)static_entity)->getName());
	staticBody->userData = CHandle(this).getOwner().asVoidPtr();

	checkIfInsideRecastAABB();
}

void TCompStaticBody::init() {
}

void TCompStaticBody::fixedUpdate(float elapsed) {
	TCompTransform* transform = h_transform;

	if (transform->transformChanged()) {
		staticBody->setGlobalPose(
			PxTransform(
			Physics.XMVECTORToPxVec3(transform->position),
			Physics.XMVECTORToPxQuat(transform->rotation)
			)
			);
	}
}

void TCompStaticBody::checkIfInsideRecastAABB(){
	CEntity* r_entity = ((CEntity*)static_entity);
	std::string name = ((CEntity*)CHandle(this).getOwner())->getName();
	if (kind == colliderType::BOX){
		TCompColliderBox* col_box = r_entity->get<TCompColliderBox>();
		col_box->checkIfInsideRecastAABB();
	}
	else if (kind == colliderType::CAPSULE){
		TCompColliderCapsule* col_capsule = r_entity->get<TCompColliderCapsule>();
		col_capsule->checkIfInsideRecastAABB();
	}
	else if (kind == colliderType::CONVEX){
		TCompColliderConvex* col_convex = r_entity->get<TCompColliderConvex>();
		col_convex->checkIfInsideRecastAABB();
	}
	else if (kind == colliderType::SPHERE){
		TCompColliderSphere* col_sphere = r_entity->get<TCompColliderSphere>();
		col_sphere->checkIfInsideRecastAABB();
	}
}

