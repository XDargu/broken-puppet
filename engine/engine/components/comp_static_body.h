#ifndef INC_COMP_STATIC_BODY_H_
#define INC_COMP_STATIC_BODY_H_

#include "base_component.h"

struct TCompStaticBody : TBaseComponent {
public:
	physx::PxRigidStatic* staticBody;

	TCompStaticBody() : staticBody(nullptr) {}

	void loadFromAtts(MKeyValue &atts) {
	}

	void init() {
		CEntity* e = CHandle(this).getOwner();
		TCompTransform* t = e->get<TCompTransform>();
		TCompCollider* c = e->get<TCompCollider>();

		assert(t || fatal("TStaticBody requieres a TTransform component"));
		assert(c || fatal("TStaticBody requieres a TCollider component"));

		staticBody = physx::PxCreateStatic(
			*Physics.gPhysicsSDK
			, physx::PxTransform(
				Physics.XMVECTORToPxVec3(t->position),
				Physics.XMVECTORToPxQuat(t->rotation))
			, *c->collider
			);
		
		Physics.gScene->addActor(*staticBody);
	}

	std::string toString() {
		return "Bounds: [" + Physics.toString(staticBody->getWorldBounds().minimum) + " ; " + Physics.toString(staticBody->getWorldBounds().maximum) + "]";
	}
};

#endif
