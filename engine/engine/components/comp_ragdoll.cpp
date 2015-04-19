#include "mcv_platform.h"
#include "comp_ragdoll.h"
#include "comp_skeleton.h"
#include "skeletons/skeleton_manager.h"

TCompRagdoll::TCompRagdoll() {
	ragdoll_active = false;
}

TCompRagdoll::~TCompRagdoll() {

}

void TCompRagdoll::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	skeleton = assertRequiredComponent<TCompSkeleton>(this);

	// Obtener los bones en base a sus nombres
	TCompSkeleton* skel = skeleton;
	auto& cal_bones = skel->model->getSkeleton()->getVectorBone();
	for (size_t bone_idx = 0; bone_idx < cal_bones.size(); ++bone_idx) {
		cal_bones[bone_idx]->getCoreBone()->getName();
	}

	for (int i = 53; i < 54; ++i) {
		CalBone* cal_bone = skel->model->getSkeleton()->getBone(i);
		const CalVector& cal_pos = cal_bone->getTranslationAbsolute();
		const CalQuaternion& cal_mtx = cal_bone->getRotation();

		XMVECTOR dx_pos = Cal2DX(cal_pos);
		XMVECTOR dx_rot = Cal2DX(cal_mtx);

		physx::PxMaterial* mat = Physics.gPhysicsSDK->createMaterial(0.5, 0.5, 0.5);

		PxShape* collider = Physics.gPhysicsSDK->createShape(
			physx::PxSphereGeometry(
			physx::PxReal(0.05)
			),
			*mat
			,
			true);

		PxRigidDynamic* rigidBody = physx::PxCreateDynamic(
			*Physics.gPhysicsSDK
			, physx::PxTransform(
			Physics.XMVECTORToPxVec3(dx_pos),
			Physics.XMVECTORToPxQuat(dx_rot))
			, *collider
			, 1);

		Physics.gScene->addActor(*rigidBody);

		bone_map[i] = rigidBody;
	}
	

	setActive(true);
}

void TCompRagdoll::fixedUpdate(float elapsed) {
	// If the ragdoll is not active, the rigid bones must follow the bone position of the animation
	if (!ragdoll_active) {
		for (auto& it : bone_map) {
			TCompSkeleton* skel = skeleton;
			CalBone* cal_bone = skel->model->getSkeleton()->getBone(it.first);
			const CalVector& cal_pos = cal_bone->getTranslationAbsolute();
			const CalQuaternion& cal_mtx = cal_bone->getRotation();
			it.second->setKinematicTarget(PxTransform(Physics.XMVECTORToPxVec3(Cal2DX(cal_pos)), Physics.XMVECTORToPxQuat(Cal2DX(cal_mtx))));
		}
	}
}

void TCompRagdoll::setActive(bool active) {
	ragdoll_active = active;

	// Make all the rigidboies kinematic if the ragdoll is not active, and make them not kinematic if active
	for (auto& it : bone_map) {
		it.second->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, !active);
	}	
}

bool TCompRagdoll::isRagdollActive() {
	return ragdoll_active;
}

PxRigidDynamic* TCompRagdoll::getBoneRigid(int id) {
	if (bone_map.find(id) != bone_map.end())
		return bone_map[id];
	return nullptr;
}