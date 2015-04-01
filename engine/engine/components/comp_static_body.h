#ifndef INC_COMP_STATIC_BODY_H_
#define INC_COMP_STATIC_BODY_H_

#include "base_component.h"

struct TCompStaticBody : TBaseComponent {
public:
	physx::PxRigidStatic* staticBody;

	TCompStaticBody() : staticBody(nullptr) {}

	~TCompStaticBody() { Physics.gScene->removeActor(*staticBody); }

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {

		CEntity* e = CHandle(this).getOwner();
		TCompTransform* t = e->get<TCompTransform>();
		TCompCollider* c = e->get<TCompCollider>();
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
	}

	void init() {
	}

	std::string toString() {
		return "Bounds: [" + Physics.toString(staticBody->getWorldBounds().minimum) + " ; " + Physics.toString(staticBody->getWorldBounds().maximum) + "]";
	}
};

#endif
