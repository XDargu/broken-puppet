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

	CDBGTimer tm;

	ragdoll = (CCoreRagdoll*)ragdoll_manager.getByName(ragdoll_name.c_str());

	dbg("Load ragdoll %s took %g seconds. Seconds since scene load: %g\n", ragdoll_name, tm.seconds(), CApp::get().load_timer.seconds());
	CApp::get().load_ragdoll_time += tm.seconds();
	TCompName* name = assertRequiredComponent<TCompName>(this);

	for (auto& rigid : ragdoll->bone_map) {
		rigid.second->userData = CHandle(this).getOwner().asVoidPtr();
		rigid.second->setName(name->name);
	}

	setCollisionGroups();
	setActive(false);
}

void TCompRagdoll::fixedUpdate(float elapsed) {

	/*if (CIOStatus::get().becomesPressed(CIOStatus::F8_KEY)) {
		if (!((CEntity*)CHandle(this).getOwner())->hasTag("player"))
			setActive(!isRagdollActive());
	}*/


	// If the ragdoll is not active, the rigid bones must follow the bone position of the animation
	if (!ragdoll_active) {
		for (auto& it : ragdoll->bone_map) {
			TCompSkeleton* skel = skeleton;
			CalBone* cal_bone = skel->model->getSkeleton()->getBone(it.first);
			const CalVector& cal_pos = cal_bone->getTranslationAbsolute();
			const CalQuaternion& cal_mtx = cal_bone->getRotationAbsolute();
			if (cal_bone->getCoreBone()->getUserData() == false) {
			//if (it.second->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC)) {
				it.second->setKinematicTarget(PxTransform(Physics.XMVECTORToPxVec3(Cal2DX(cal_pos)), Physics.XMVECTORToPxQuat(Cal2DX(cal_mtx))));
			}
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
	/*else {
		TCompSkeleton* skel = skeleton;

		// find the prop1 bone. If exists, set the position far away
		int prop_id = skel->model->getCoreModel()->getBoneId("Prop1");
		if (prop_id != -1) {
			skel->model->getSkeleton()->getVectorBone()[prop_id]->setTranslation(CalVector(50000, 50000, 50000));
		}
	}*/

	// Make all the rigidboies kinematic if the ragdoll is not active, and make them not kinematic if active
	CEntity* e = (CEntity*)CHandle(this).getOwner();
	if (e->hasTag("player")){
		setCollisonPlayer(active);
		/*disableBoneTree(10);
		disableBoneTree(26);
		disableBoneTree(74);
		disableBoneTree(79);*/
		/*disableBoneTree(8);
		enableBoneTree(8);*/
	}else if(e->hasTag("enemy")){
		setCollisonEnemy(active);
	}
}

void TCompRagdoll::setCollisonPlayer(bool active){
	CEntity* e = (CEntity*)CHandle(this).getOwner();
	if (active){
		PxU32 myMask = FilterGroup::ePLAYER_RG;
		PxU32 notCollide = FilterGroup::ePLAYER;
		PxShape* collider;
		for (auto& it : ragdoll->bone_map) {
			it.second->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, false);
			it.second->getShapes(&collider, 1);
			setupFiltering(collider, myMask, notCollide);
		}
	}
	else{
		PxU32 myMask = FilterGroup::eNON_COLLISION;
		PxU32 notCollide = FilterGroup::eACTOR
			| FilterGroup::eACTOR_NON_COLLISION
			| FilterGroup::ePLAYER
			| FilterGroup::eENEMY
			| FilterGroup::eENEMY_RG
			| FilterGroup::eLEVEL
			| FilterGroup::eNON_COLLISION;
		PxShape* collider;
		for (auto& it : ragdoll->bone_map) {
			it.second->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
			it.second->getShapes(&collider, 1);
			setupFiltering(collider, myMask, notCollide);
		}
	}
}

void TCompRagdoll::setCollisonEnemy(bool active){
	CEntity* e = (CEntity*)CHandle(this).getOwner();
	if (active){
		PxU32 myMask = FilterGroup::eENEMY_RG;
		PxU32 notCollide = FilterGroup::eENEMY;
		PxShape* collider;
		for (auto& it : ragdoll->bone_map) {
			it.second->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, false);
			it.second->getShapes(&collider, 1);
			setupFiltering(collider, myMask, notCollide);
		}
	}else{
		PxU32 myMask = FilterGroup::eNON_COLLISION;
		PxU32 notCollide = FilterGroup::eACTOR
			| FilterGroup::eACTOR_NON_COLLISION
			| FilterGroup::ePLAYER
			| FilterGroup::eENEMY
			| FilterGroup::ePLAYER_RG
			| FilterGroup::eLEVEL
			| FilterGroup::eNON_COLLISION;
		PxShape* collider;
		for (auto& it : ragdoll->bone_map) {
			it.second->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
			it.second->getShapes(&collider, 1);
			setupFiltering(collider, myMask, notCollide);
		}
	}
}

void TCompRagdoll::setCollisonPlayerBone(bool active, int bone_id){
	CEntity* e = (CEntity*)CHandle(this).getOwner();
	if (active){
		PxU32 myMask = FilterGroup::ePLAYER_RG;
		PxU32 notCollide = FilterGroup::ePLAYER;
		PxShape* collider;
		for (auto& it : ragdoll->bone_map) {
			if (it.first == bone_id) {
				it.second->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, false);
				it.second->getShapes(&collider, 1);
				setupFiltering(collider, myMask, notCollide);
			}
		}
	}
	else{
		PxU32 myMask = FilterGroup::eNON_COLLISION;
		PxU32 notCollide = FilterGroup::eACTOR
			| FilterGroup::eACTOR_NON_COLLISION
			| FilterGroup::ePLAYER
			| FilterGroup::eENEMY
			| FilterGroup::eENEMY_RG
			| FilterGroup::eLEVEL
			| FilterGroup::eNON_COLLISION;
		PxShape* collider;
		for (auto& it : ragdoll->bone_map) {
			if (it.first == bone_id) {
				it.second->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
				it.second->getShapes(&collider, 1);
				setupFiltering(collider, myMask, notCollide);
			}
		}
	}
}

void TCompRagdoll::setCollisonEnemyBone(bool active, int bone_id){
	CEntity* e = (CEntity*)CHandle(this).getOwner();
	if (active){
		PxU32 myMask = FilterGroup::eENEMY_RG;
		PxU32 notCollide = FilterGroup::eENEMY;
		PxShape* collider;
		for (auto& it : ragdoll->bone_map) {
			if (it.first == bone_id) {
				it.second->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, false);
				it.second->getShapes(&collider, 1);
				setupFiltering(collider, myMask, notCollide);
			}
		}
	}
	else{
		PxU32 myMask = FilterGroup::eNON_COLLISION;
		PxU32 notCollide = FilterGroup::eACTOR
			| FilterGroup::eACTOR_NON_COLLISION
			| FilterGroup::ePLAYER
			| FilterGroup::eENEMY
			| FilterGroup::ePLAYER_RG
			| FilterGroup::eLEVEL
			| FilterGroup::eNON_COLLISION;
		PxShape* collider;
		for (auto& it : ragdoll->bone_map) {
			if (it.first == bone_id) {
				it.second->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
				it.second->getShapes(&collider, 1);
				setupFiltering(collider, myMask, notCollide);
			}
		}
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

void TCompRagdoll::breakJoint(int id) {
	float force;
	float torque;
	ragdoll->articulations[id]->setBreakForce(0.05f, 0.05f);
}

PxRigidDynamic* TCompRagdoll::getBoneRigidRaycast(XMVECTOR origin, XMVECTOR dir) {

	PxRaycastBuffer buf;

	Physics.raycastAll(origin, dir, 10, buf);

	// TODO: Return the nearest if no raycast collision
	//PxRigidDynamic* nearest;
	//float nearest_dist;

	for (unsigned int i = 0; i < buf.nbTouches; i++)
	{
		for (auto& rigid : ragdoll->bone_map) {
			if (buf.touches[i].actor == rigid.second) {
				return rigid.second;
			}
		}		
	}

	return ragdoll->bone_map.begin()->second;
	return nullptr;
}

void TCompRagdoll::disableBoneTree(PxRigidDynamic* rigid_bone) {
	for (auto& it : ragdoll->bone_map) {
		if (it.second == rigid_bone) {
			disableBoneTree(it.first);
		}
	}
}

void TCompRagdoll::disableBoneTree(int bone_id) {
	TCompSkeleton* skel = skeleton;
	CalSkeleton* m_skel = skel->model->getSkeleton();
	auto& cal_bones = m_skel->getVectorBone();

	int size = (int)cal_bones.size();

	for (int bone_idx = 0; bone_idx < (int)cal_bones.size(); ++bone_idx) {
		CalBone* bone = cal_bones[bone_idx];
		int parent_id = bone->getCoreBone()->getParentId();

		// If my parents is disabled, disable me too
		if (bone_id == parent_id) {
			disableBoneTree(bone_idx);
		}

		if (bone_id == bone_idx) {
			for (auto& it : ragdoll->bone_map) {
				if (it.first == bone_id) {

					CEntity* e = (CEntity*)CHandle(this).getOwner();
					if (e->hasTag("player")){
						setCollisonPlayerBone(true, bone_id);
					}
					else if (e->hasTag("enemy")){
						setCollisonEnemyBone(true, bone_id);
					}
				}
			}
			skel->setBoneRagdoll(bone_id, true);
		}
	}
}

void TCompRagdoll::enableBoneTree(int bone_id) {
	TCompSkeleton* skel = skeleton;
	CalSkeleton* m_skel = skel->model->getSkeleton();
	auto& cal_bones = m_skel->getVectorBone();

	int size = (int)cal_bones.size();

	for (int bone_idx = 0; bone_idx < (int)cal_bones.size(); ++bone_idx) {
		CalBone* bone = cal_bones[bone_idx];
		int parent_id = bone->getCoreBone()->getParentId();

		// If my parents is disabled, disable me too
		if (bone_id == parent_id) {
			enableBoneTree(bone_idx);
		}

		if (bone_id == bone_idx) {
			for (auto& it : ragdoll->bone_map) {
				if (it.first == bone_id) {
					CEntity* e = (CEntity*)CHandle(this).getOwner();
					if (e->hasTag("player")){
						setCollisonPlayerBone(false, bone_id);
					}
					else if (e->hasTag("enemy")){
						setCollisonEnemyBone(false, bone_id);
					}
				}
			}
			skel->setBoneRagdoll(bone_id, false);
		}
	}
}