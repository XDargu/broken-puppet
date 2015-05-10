#include "mcv_platform.h"
#include "comp_ragdoll.h"
#include "comp_skeleton.h"
#include "comp_name.h"
#include "skeletons/skeleton_manager.h"
#include "io\iostatus.h"

TCompRagdoll::TCompRagdoll() {
	ragdoll_active = false;
}

TCompRagdoll::~TCompRagdoll() {

}

void TCompRagdoll::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	skeleton = assertRequiredComponent<TCompSkeleton>(this);

	std::string ragdoll_name = atts["name"];

	ragdoll = (CCoreRagdoll*)ragdoll_manager.getByName(ragdoll_name.c_str());

	TCompName* name = assertRequiredComponent<TCompName>(this);

	for (auto& rigid : ragdoll->bone_map) {
		rigid.second->userData = CHandle(this).getOwner().asVoidPtr();
		rigid.second->setName(name->name);
	}

	setCollisionGroups();
	setActive(false);
}

void TCompRagdoll::fixedUpdate(float elapsed) {

	if (CIOStatus::get().becomesPressed(CIOStatus::F8_KEY)) {
		if (!((CEntity*)CHandle(this).getOwner())->hasTag("player"))
			setActive(!isRagdollActive());
	}


	// If the ragdoll is not active, the rigid bones must follow the bone position of the animation
	if (!ragdoll_active) {
		for (auto& it : ragdoll->bone_map) {
			TCompSkeleton* skel = skeleton;
			CalBone* cal_bone = skel->model->getSkeleton()->getBone(it.first);
			const CalVector& cal_pos = cal_bone->getTranslationAbsolute();
			const CalQuaternion& cal_mtx = cal_bone->getRotationAbsolute();
			it.second->setKinematicTarget(PxTransform(Physics.XMVECTORToPxVec3(Cal2DX(cal_pos)), Physics.XMVECTORToPxQuat(Cal2DX(cal_mtx))));
			/*PxVec3 targetVel = Physics.XMVECTORToPxVec3(Cal2DX(cal_pos)) - it.second->getGlobalPose().p;
			targetVel.normalize();
			it.second->setLinearVelocity(targetVel);*/
		}
	}
}

void TCompRagdoll::setActive(bool active) {
	ragdoll_active = active;

	// Call the skeleton to save the ragdoll bone positions
	if (!active) {
		TCompSkeleton* skel = skeleton;
		skel->ragdollUnactive();
	}

	// Make all the rigidboies kinematic if the ragdoll is not active, and make them not kinematic if active
	for (auto& it : ragdoll->bone_map) {
		it.second->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, !active);
	}
}

bool TCompRagdoll::isRagdollActive() {
	return ragdoll_active;
}

PxRigidDynamic* TCompRagdoll::getBoneRigid(int id) {
	if (ragdoll->bone_map.find(id) != ragdoll->bone_map.end())
		return ragdoll->bone_map[id];
	return nullptr;
}

void TCompRagdoll::setCollisionGroups(){
	CEntity* e = (CEntity*)CHandle(this).getOwner();
	PxU32 myMask;
	if (e->hasTag("enemy")){
		myMask = convertStrInCollisionFilter("Enemy_Ragdoll");
	}
	else if (e->hasTag("player")){
		myMask = convertStrInCollisionFilter("Player_Ragdoll");
	}
	PxU32 notCollide = 0;
	bool found = false;
	auto it = CPhysicsManager::get().m_collision->find(myMask);
	if (it != CPhysicsManager::get().m_collision->end()){
		std::vector<physx::PxU32>colFil = it->second;
		if (!colFil.empty()){
			for (int i = 0; i < colFil.size(); i++){
				notCollide |= colFil[i];
			}
		}
	}

	for (auto& it : ragdoll->bone_map) {
		PxShape* collider;
		it.second->getShapes(&collider, 1);

		setupFiltering(collider, myMask, notCollide);
	}
}

void TCompRagdoll::breakJoints() {
	float force;
	float torque;
	for (PxD6Joint* joint : ragdoll->articulations) {
		joint->getBreakForce(force, torque);
		if (force == 10000) {
			joint->setBreakForce(0.05f, 0.05f);
		}
	}
}