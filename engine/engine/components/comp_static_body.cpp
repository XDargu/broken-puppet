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

	CEntity* e = CHandle(this).getOwner();
	TCompColliderBox* box_c = e->get<TCompColliderBox>();
	TCompColliderMesh* mesh_c = e->get<TCompColliderMesh>();
	TCompColliderConvex* capsule_cvx = e->get<TCompColliderConvex>();

	TCompColliderSphere* sphere_c = e->get<TCompColliderSphere>();
	TCompColliderCapsule* capsule_c = e->get<TCompColliderCapsule>();

	TCompColliderMultiple* multiple_c = e->get<TCompColliderMultiple>();

	TCompTransform* trans = (TCompTransform*)h_transform;

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
	if (multiple_c)
		col = multiple_c;


	XASSERT(col != nullptr, "TRigidBody requieres a Collider component");

	if (multiple_c) {
		staticBody = physx::PxCreateStatic(
			*Physics.gPhysicsSDK
			, physx::PxTransform(
			Physics.XMVECTORToPxVec3(trans->position),
			Physics.XMVECTORToPxQuat(XMQuaternionNormalize(trans->rotation)))
			, *multiple_c->colliders[0]
			);

		for (int i = 1; i < multiple_c->colliders.size(); i++) {
			staticBody->attachShape(*multiple_c->colliders[i]);
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