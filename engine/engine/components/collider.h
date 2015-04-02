#ifndef INC_COLLIDER_H_
#define INC_COLLIDER_H_

#include "physics_manager.h"

class CCollider {
public:
	PxShape* collider;

	CCollider() : collider(nullptr) {}

	PxMaterial* getMaterial() {
		PxMaterial* mat;
		collider->getMaterials(&mat, 1);
		return mat;
	}

	// Set the collider static friction, dynamic friction and restitution given by a vector
	void setMaterialProperties(XMVECTOR properties) {
		PxMaterial* mat;
		collider->getMaterials(&mat, 1);
		mat->setStaticFriction(XMVectorGetX(properties));
		mat->setDynamicFriction(XMVectorGetY(properties));
		mat->setRestitution(XMVectorGetZ(properties));
	}

	// Returns the material properties as a vector
	XMVECTOR getMaterialProperties() {
		PxMaterial* mat;
		collider->getMaterials(&mat, 1);
		return XMVectorSet(
			mat->getStaticFriction(),
			mat->getDynamicFriction(),
			mat->getRestitution(),
			0
		);
	}
};

#endif