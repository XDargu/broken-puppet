#ifndef INC_COMP_COLLIDER_H_
#define INC_COMP_COLLIDER_H_

#include "base_component.h"

struct TCompCollider : TBaseComponent {

	physx::PxShape* collider;

	TCompCollider() { }

	void loadFromAtts(MKeyValue &atts) {

		collider = Physics.gPhysicsSDK->createShape(
			physx::PxBoxGeometry(
				physx::PxReal(atts.getFloat("boxX", 0.5))
				, physx::PxReal(atts.getFloat("boxY", 0.5))
				, physx::PxReal(atts.getFloat("boxZ", 0.5))
			),
			*Physics.gPhysicsSDK->createMaterial(
				atts.getFloat("staticFriction", 0.5)
				, atts.getFloat("dynamicFriction", 0.5)
				, atts.getFloat("restitution", 0.5))
			,
			true);
	}

	void init() {
	}

	physx::PxMaterial* getMaterial() {
		physx::PxMaterial* mat;
		collider->getMaterials(&mat, 1);
			return mat;
	}

	// Returns the material properties as a vector
	XMVECTOR getMaterialProperties() {
		physx::PxMaterial* mat;
		collider->getMaterials(&mat, 1);
		return XMVectorSet(
			mat->getStaticFriction(),
			mat->getDynamicFriction(),
			mat->getRestitution(),
			0
			);
	}

	std::string toString() {
		return "Static friction: " + std::to_string(getMaterial()->getStaticFriction()) +
			"\nDynamic friction: " + std::to_string(getMaterial()->getDynamicFriction()) +
			"\nRestitution: " + std::to_string(getMaterial()->getRestitution());
	}
};

#endif
