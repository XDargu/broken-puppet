#ifndef INC_COMP_COLLIDER_BOX_H_
#define INC_COMP_COLLIDER_BOX_H_

#include "base_component.h"
#include "collider.h"

struct TCompColliderBox : public CCollider, TBaseComponent {

	TCompColliderBox() : CCollider() { }

	void setShape(float boxX, float boxY, float boxZ, float static_friction, float dynamic_friction, float restitution) {
		collider = Physics.gPhysicsSDK->createShape(
			physx::PxBoxGeometry(
			physx::PxReal(boxX)
			, physx::PxReal(boxY)
			, physx::PxReal(boxZ)
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
			physx::PxBoxGeometry(
				physx::PxReal(atts.getFloat("boxX", 0.5))
				, physx::PxReal(atts.getFloat("boxY", 0.5))
				, physx::PxReal(atts.getFloat("boxZ", 0.5))
			),
			*mat
			,
			true);		
		//collider->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
	}

	void init() {
	}
};

#endif
