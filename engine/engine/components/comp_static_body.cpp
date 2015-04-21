#include "mcv_platform.h"
#include "comp_transform.h"
#include "comp_static_body.h"
#include "comp_collider_box.h"
#include "comp_collider_mesh.h"

TCompStaticBody::~TCompStaticBody() { Physics.gScene->removeActor(*staticBody); }

void TCompStaticBody::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	h_transform = assertRequiredComponent<TCompTransform>(this);

	CEntity* e = CHandle(this).getOwner();
	TCompTransform* t = h_transform;
	TCompColliderBox* c = e->get<TCompColliderBox>();
	TCompColliderMesh* mesh_c = e->get<TCompColliderMesh>();

	assert(t || fatal("TStaticBody requieres a TTransform component"));
	assert((c || mesh_c) || fatal("TStaticBody requieres a TCollider or a TColliderMesh component"));

	if (c) {
		staticBody = physx::PxCreateStatic(
			*Physics.gPhysicsSDK
			, physx::PxTransform(
			Physics.XMVECTORToPxVec3(t->position),
			Physics.XMVECTORToPxQuat(t->rotation))
			, *c->collider
			);
	}
	if (mesh_c) {
		staticBody = physx::PxCreateStatic(
			*Physics.gPhysicsSDK
			, physx::PxTransform(
			Physics.XMVECTORToPxVec3(t->position),
			Physics.XMVECTORToPxQuat(t->rotation))
			, *mesh_c->collider
			);
	}

	Physics.gScene->addActor(*staticBody);

	staticBody->setName(e->getName());
	staticBody->userData = CHandle(this).getOwner().asVoidPtr();
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