#include "mcv_platform.h"
#include "comp_player_rope.h"
#include "comp_distance_joint.h"
#include "entity_manager.h"
#include "comp_skeleton.h"
#include "comp_transform.h"
#include "comp_collider_sphere.h"
#include "comp_rope.h"
#include "comp_name.h"
#include "comp_rigid_body.h"
#include "handle\prefabs_manager.h"
#include "physics_manager.h"


TCompPlayerRope::TCompPlayerRope(){
	entity_chain = new std::vector<CHandle>();
	links_distance = .001f;
	hand_joint = nullptr;
}
TCompPlayerRope::~TCompPlayerRope(){
	delete entity_chain;
}

void TCompPlayerRope::loadFromAtts(const std::string& elem, MKeyValue &atts) {

}

void TCompPlayerRope::init(){

	// Get the owner, it´s supose to be the player
	CHandle player = CHandle(this).getOwner();
	if (player.isValid()){

		// Get the skeleton
		CHandle comp_skeleton = ((CEntity*)player)->get<TCompSkeleton>();
		TCompTransform* player_trans = ((CEntity*)player)->get<TCompTransform>();
		PxVec3 player_pos = Physics.XMVECTORToPxVec3(player_trans->position);

		if (comp_skeleton.isValid()){
			TCompSkeleton* skeleton = comp_skeleton;

			// R Hand position
			XMVECTOR r_hand_pos = skeleton->getPositionOfBone(28);

			// Reel position
			XMVECTOR reel_position = skeleton->getPositionOfBone(58);

			// Make the chain
			int chain_size = 12;

			// Temp variables
			CHandle last_link = CHandle();
			CHandle last_link_rigid = CHandle();
			PxRigidDynamic* m_rigid_1 = nullptr;
			PxRigidDynamic* m_rigid_2 = nullptr;

			for (int i = 0; i < chain_size; i++){
				// Loading chain_links
				CHandle link = (CHandle)prefabs_manager.getInstanceByName("chain_link");
				TCompName* name = ((CEntity*)link)->get<TCompName>();
				std::string m_name = "chain_link" + std::to_string(i);
				std::strcpy(name->name, m_name.c_str());

				// Joint the objeto if it´s not the firts one

				// Take the last obj in the list				
				if (i > 0)  {
					last_link = entity_chain->back();
				}				

				if (last_link.isValid()) {
					last_link_rigid = ((CEntity*)last_link)->get<TCompRigidBody>();
				}
				
				CHandle m_link_rigid = ((CEntity*)link)->get<TCompRigidBody>();
				TCompColliderSphere* m_link_collider = ((CEntity*)link)->get<TCompColliderSphere>();

				PxU32 myMask = FilterGroup::ePLAYER_RG;
				PxU32 notCollide = FilterGroup::eACTOR
					| FilterGroup::eACTOR_NON_COLLISION
					| FilterGroup::ePLAYER
					| FilterGroup::eENEMY
					//| FilterGroup::ePLAYER_RG
					| FilterGroup::ePLAYERINACTIVE
					//| FilterGroup::eLEVEL
					| FilterGroup::eNON_COLLISION;
				setupFiltering(m_link_collider->collider, myMask, notCollide);

				PxTransform zero = PxTransform(PxVec3(0, 0, 0));

				if (last_link_rigid.isValid()) {
					m_rigid_1 = ((TCompRigidBody*)last_link_rigid)->rigidBody;
				}

				m_rigid_2 = ((TCompRigidBody*)m_link_rigid)->rigidBody;
				((TCompRigidBody*)m_link_rigid)->water_damping = false;
				m_rigid_2->setLinearDamping(9.f);
				m_rigid_2->setAngularDamping(9.f);
				m_rigid_2->setSolverIterationCounts(16, 8);

				// Generating joint
				if ((i == 0) || (last_link_rigid.isValid() && m_link_rigid.isValid())){

					TCompDistanceJoint* comp_joint = CHandle::create<TCompDistanceJoint>();
					((CEntity*)link)->add(comp_joint);


					if (i == chain_size - 1){
						back_joint = physx::PxDistanceJointCreate(*Physics.gPhysicsSDK, NULL, zero, m_rigid_2, zero);
					}
					if (i == 0)
					{
						comp_joint->create(NULL, m_rigid_2, 2, player_pos, player_pos, zero, zero);
						hand_joint = comp_joint->joint;
					}
					else
					{
						comp_joint->create(m_rigid_1, m_rigid_2, 2, player_pos, player_pos, zero, zero);
					}


					PxDistanceJoint* joint = comp_joint->joint;

					if (joint){
						joint->setMaxDistance(links_distance);
						joint->setDamping(200);
						joint->setStiffness(9000);
						joint->setDistanceJointFlag(physx::PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);
						joint->setDistanceJointFlag(physx::PxDistanceJointFlag::eSPRING_ENABLED, true);
						joint->setConstraintFlag(physx::PxConstraintFlag::eCOLLISION_ENABLED, false);
						
						joint->setConstraintFlag(PxConstraintFlag::ePROJECTION, true);
						joint->setTolerance(0.001f);

						/*if ((i == 0) || (i == chain_size - 1)) {
							joint->setDamping(2000);
							joint->setStiffness(30000);
						}*/
					}

					if (i > 0) {
						TCompRope* rope = CHandle::create<TCompRope>();
						((CEntity*)link)->add(rope);
						rope->loadFromAtts("rope", MKeyValue());
						rope->joint = comp_joint;
						rope->width = 0.005;
					}

					if (i == 0) {
						((TCompRigidBody*)m_link_rigid)->setKinematic(true);
						first_chain = m_rigid_2;
					}
					if (i == chain_size - 1) {
						((TCompRigidBody*)m_link_rigid)->setKinematic(true);
						last_chain = m_rigid_2;
					}
				}

				
				// Add link to entity chain
				entity_chain->push_back(link);
			}
		}
	}
}

void TCompPlayerRope::fixedUpdate(float elapsed){
	// follow the hand
	// Get the owner, it´s supose to be the player
	CHandle player = CHandle(this).getOwner();
	if (player.isValid()){

		// Get the skeleton
		CHandle comp_skeleton = ((CEntity*)player)->get<TCompSkeleton>();

		if (comp_skeleton.isValid()){
			TCompSkeleton* skeleton = comp_skeleton;

			// R Hand position
			XMVECTOR r_hand_pos = skeleton->getPositionOfBone(28);

			// Reel position
			XMVECTOR reel_position = skeleton->getPositionOfBone(58);
			TCompTransform* player_trans = ((CEntity*)player)->get<TCompTransform>();

			// Set the rigidBody to that position
			XMVECTOR pos;
			if (hand_joint){
				pos = skeleton->getPositionOfBone(29);
				hand_joint->setLocalPose(PxJointActorIndex::eACTOR0, PxTransform(Physics.XMVECTORToPxVec3(pos)));
				first_chain->setKinematicTarget(PxTransform(Physics.XMVECTORToPxVec3(pos)));
				first_chain->setGlobalPose(PxTransform(Physics.XMVECTORToPxVec3(pos)));
			}
			if (back_joint){
				pos = skeleton->getPositionOfBone(58);
				back_joint->setLocalPose(PxJointActorIndex::eACTOR0, PxTransform(Physics.XMVECTORToPxVec3(pos)));
				last_chain->setKinematicTarget(PxTransform(Physics.XMVECTORToPxVec3(pos)));
				last_chain->setGlobalPose(PxTransform(Physics.XMVECTORToPxVec3(pos)));
			}
			/**
			CHandle reel_link = entity_chain->at(0);
			if (reel_link.isValid()){
			CHandle comp_transform = ((CEntity*)reel_link)->get<TCompTransform>();

			if (comp_transform.isValid()){
			((TCompTransform*)comp_transform)->teleport(reel_position);
			}
			}
			/**/
		}
	}
}
