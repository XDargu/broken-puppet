#ifndef INC_COMP_COLLIDER_CAPSULE_H_
#define INC_COMP_COLLIDER_CAPSULE_H_

#include "base_component.h"

struct TCompColliderCapsule : TBaseComponent {

	physx::PxShape* collider;

	TCompColliderCapsule() : collider(nullptr) {}

	void setShape(float radius, float half_height, float static_friction, float dynamic_friction, float restitution) {
		collider = Physics.gPhysicsSDK->createShape(
			physx::PxCapsuleGeometry(
				radius,
				half_height
			),
			*Physics.gPhysicsSDK->createMaterial(
				static_friction
				, dynamic_friction
				, restitution
			)
			,
			true);
		//collider->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
	}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {

		physx::PxMaterial* mat = Physics.gPhysicsSDK->createMaterial(
			atts.getFloat("staticFriction", 0.5)
			, atts.getFloat("dynamicFriction", 0.5)
			, atts.getFloat("restitution", 0.5)
			);
		std::string frictionCombineMode = atts.getString("frictionCombineMode", "None");
		std::string restitutionCombineMode = atts.getString("restitutionCombineMode", "None");

		if (frictionCombineMode == "Min") {
			mat->setFrictionCombineMode(physx::PxCombineMode::eMIN);
		}
		if (frictionCombineMode == "Max") {
			mat->setFrictionCombineMode(physx::PxCombineMode::eMAX);
		}
		if (frictionCombineMode == "Average") {
			mat->setFrictionCombineMode(physx::PxCombineMode::eAVERAGE);
		}
		if (frictionCombineMode == "Multiply") {
			mat->setFrictionCombineMode(physx::PxCombineMode::eMULTIPLY);
		}

		if (restitutionCombineMode == "Min") {
			mat->setRestitutionCombineMode(physx::PxCombineMode::eMIN);
		}
		if (restitutionCombineMode == "Max") {
			mat->setRestitutionCombineMode(physx::PxCombineMode::eMAX);
		}
		if (restitutionCombineMode == "Average") {
			mat->setRestitutionCombineMode(physx::PxCombineMode::eAVERAGE);
		}
		if (restitutionCombineMode == "Multiply") {
			mat->setRestitutionCombineMode(physx::PxCombineMode::eMULTIPLY);
		}

		collider = Physics.gPhysicsSDK->createShape(
			physx::PxCapsuleGeometry(
				atts.getFloat("radius", 0.5),
				atts.getFloat("halfHeight", 0.5)
			),
			*mat
			,
			true);
		//collider->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
	}

	void init() {
	}

	physx::PxMaterial* getMaterial() {
		physx::PxMaterial* mat;
		collider->getMaterials(&mat, 1);
		return mat;
	}

	// Set the collider static friction, dynamic friction and restitution given by a vector
	void setMaterialProperties(XMVECTOR properties) {
		physx::PxMaterial* mat;
		collider->getMaterials(&mat, 1);
		mat->setStaticFriction(XMVectorGetX(properties));
		mat->setDynamicFriction(XMVectorGetY(properties));
		mat->setRestitution(XMVectorGetZ(properties));
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
};

#endif
