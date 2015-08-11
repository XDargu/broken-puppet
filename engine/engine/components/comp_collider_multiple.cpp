#include "mcv_platform.h"
#include "comp_collider_multiple.h"
#include "comp_recast_aabb.h"
#include "nav_mesh_manager.h"

void TCompColliderMultiple::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	if (elem == "colliderMulti") {

		// Make the material
		mat = Physics.gPhysicsSDK->createMaterial(
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
	}


	if (elem == "shape") {

		std::string type = atts.getString("type", "BOX");
		XMVECTOR pos_offset = atts.getPoint("posOffset");
		XMVECTOR rot_offset = atts.getQuat("rotOffset");
		PxTransform local_offset = PxTransform( Physics.XMVECTORToPxVec3(pos_offset), Physics.XMVECTORToPxQuat(rot_offset) );
		PxShape* shape_collider = nullptr;

		if (type == "BOX") {
			shape_collider = Physics.gPhysicsSDK->createShape(
				physx::PxBoxGeometry(
					physx::PxReal(atts.getFloat("length", 0.5) * 0.5f)
					, physx::PxReal(atts.getFloat("height", 0.5) * 0.5f)
					, physx::PxReal(atts.getFloat("width", 0.5) * 0.5f)
				),
				*mat
				,
				true);			
		}

		shape_collider->setLocalPose(local_offset);
		colliders.push_back(shape_collider);
		
		//AABB recast_aabb = AABB(XMVectorSet(-22.f, 0.f, -33.f, 0.f), XMVectorSet(5.0f, 1.f, -8.f, 0));
		//setCollisionGroups();
		//collider->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
	}
}

void TCompColliderMultiple::init() {
}