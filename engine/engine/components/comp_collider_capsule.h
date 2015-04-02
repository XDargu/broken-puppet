#ifndef INC_COMP_COLLIDER_CAPSULE_H_
#define INC_COMP_COLLIDER_CAPSULE_H_

#include "base_component.h"
#include "collider.h"

struct TCompColliderCapsule : public CCollider, TBaseComponent {

	TCompColliderCapsule() : CCollider() {}

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

		// Rotate the capsule (we want the capsule in a vertical position)
		physx::PxTransform relativePose(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1)));
		collider->setLocalPose(relativePose);
		//collider->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
	}

	void init() {
	}

	// Returns the half height of the capsule
	float getHalfHeight() {
		physx::PxCapsuleGeometry geom;
		collider->getCapsuleGeometry(geom);
		return geom.halfHeight;
	}

	// Returns the radius of the capsule
	float getRadius() {
		physx::PxCapsuleGeometry geom;
		collider->getCapsuleGeometry(geom);
		return geom.radius;
	}
};

#endif
