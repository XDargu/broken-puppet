#include "mcv_platform.h"
#include "comp_ai_boss.h"
#include "entity_manager.h"
#include "comp_rigid_body.h"
#include "comp_transform.h"
#include "comp_skeleton.h"
#include "comp_ragdoll.h"
#include "io\iostatus.h"
#include "handle\prefabs_manager.h"


TCompAiBoss::TCompAiBoss() {
	m_fsm_boss = new fsm_boss;
}

TCompAiBoss::~TCompAiBoss() {
	delete m_fsm_boss;
}

void TCompAiBoss::loadFromAtts(const std::string& elem, MKeyValue &atts){ 
	L_hitch_joint = nullptr; 
	R_hitch_joint = nullptr; 
	can_break_hitch = false; 
	death_time = false;
	proximity_distance = 14.f;
};

void TCompAiBoss::init(){

	mPlayer = CEntityManager::get().getByName("Player");
	mBoss = CEntityManager::get().getByName("Boss");

	point_offset = PxVec3(0, -6, 0);
	distance_to_point = 12;

	activate = false;
	shoot = false;
	attack2Active = false;
	attack2Time = 0;

	move_things = 0;
	debris_creation_delay = 0;
	debris_created = 0;

	is_death = false;

	force = 24;
	R_hitch_joint = nullptr;

	m_fsm_boss->entity = mBoss;
	m_fsm_boss->Init();


	// Create Right Hitch
	/**/
	comp_skeleton = ((CEntity*)mBoss)->get<TCompSkeleton>();
	TCompSkeleton* skeleton = comp_skeleton;

	// Right Arm 72
	std::string rname = "boss/enganche_R";
	R_hitch = prefabs_manager.getInstanceByName(rname.c_str());
	TCompRigidBody* R_hitch_rigid = ((CEntity*)R_hitch)->get<TCompRigidBody>();
	R_hitch_rigid->init();

	TCompTransform* R_hitch_trans = ((CEntity*)R_hitch)->get<TCompTransform>();
	if (R_hitch_trans) R_hitch_trans->setType(0);
	PxRigidDynamic*  R_hitch_px_rigid = R_hitch_rigid->rigidBody;

	PxQuat r_rot = Physics.XMVECTORToPxQuat(skeleton->getRotationOfBone(48));
	PxVec3 r_pos = Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(48));

	PxTransform r_bone_trans = PxTransform(r_pos, r_rot);
	
	R_hitch_joint = PxFixedJointCreate(
		*Physics.gPhysicsSDK
		, R_hitch_px_rigid
		, PxTransform(PxVec3(0, 0, 0))
		, NULL
		, r_bone_trans
		);

	R_hitch_joint->setLocalPose(PxJointActorIndex::eACTOR1, r_bone_trans);
	R_hitch_px_rigid->setGlobalPose(r_bone_trans);
	/**/


	//Left Arm
	std::string lname = "boss/enganche_L";
	L_hitch = prefabs_manager.getInstanceByName(lname.c_str());
	TCompRigidBody* L_hitch_rigid = ((CEntity*)L_hitch)->get<TCompRigidBody>();
	L_hitch_rigid->init();
	PxRigidDynamic*  L_hitch_px_rigid = L_hitch_rigid->rigidBody;

	TCompTransform* L_hitch_trans = ((CEntity*)L_hitch)->get<TCompTransform>();
	if (L_hitch_trans) L_hitch_trans->setType(0);

	PxQuat l_rot = Physics.XMVECTORToPxQuat(skeleton->getRotationOfBone(23));
	PxVec3 l_pos = Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(23));

	PxTransform l_bone_trans = PxTransform(l_pos, l_rot);

	L_hitch_joint = PxFixedJointCreate(
		*Physics.gPhysicsSDK
		, L_hitch_px_rigid
		, PxTransform(PxVec3(0, 0, 0))
		, NULL
		, l_bone_trans
		);

	L_hitch_joint->setLocalPose(PxJointActorIndex::eACTOR1, l_bone_trans);
	L_hitch_px_rigid->setGlobalPose(l_bone_trans);
	/**/

	// Right Arm
	std::string hname = "boss/heart";
	H_hitch = prefabs_manager.getInstanceByName(hname.c_str());
	TCompRigidBody* H_hitch_rigid = ((CEntity*)H_hitch)->get<TCompRigidBody>();
	H_hitch_rigid->init();

	TCompTransform* H_hitch_trans = ((CEntity*)H_hitch)->get<TCompTransform>();
	if (H_hitch_trans) H_hitch_trans->setType(0);
	PxRigidDynamic*  H_hitch_px_rigid = H_hitch_rigid->rigidBody;

	PxQuat h_rot = Physics.XMVECTORToPxQuat(skeleton->getRotationOfBone(72));
	PxVec3 h_pos = Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(72));

	PxTransform h_bone_trans = PxTransform(h_pos, h_rot);

	H_hitch_joint = PxFixedJointCreate(
		*Physics.gPhysicsSDK
		, H_hitch_px_rigid
		, PxTransform(PxVec3(0, 0, 0))
		, NULL
		, h_bone_trans
		);

	H_hitch_joint->setLocalPose(PxJointActorIndex::eACTOR1, h_bone_trans);
	H_hitch_px_rigid->setGlobalPose(h_bone_trans);
	/**/

}

void TCompAiBoss::update(float elapsed){

	CIOStatus& io = CIOStatus::get();

	if (CIOStatus::get().becomesPressed(CIOStatus::H)){
		m_fsm_boss->HeadHit();
	}
	if (CIOStatus::get().becomesPressed(CIOStatus::J)){
		m_fsm_boss->HeartHit();
	}


	if (m_fsm_boss->can_proximity && (mBoss.isValid()) && (mPlayer.isValid())){
		player_trans = ((CEntity*)mBoss)->get<TCompTransform>();
		boss_trans = ((CEntity*)mPlayer)->get<TCompTransform>();

		if ((boss_trans.isValid()) && (player_trans.isValid())){			
			if (V3DISTANCE(((TCompTransform*)boss_trans)->position, ((TCompTransform*)player_trans)->position) < proximity_distance){
				m_fsm_boss->ChangeState("fbp_Proximity");
			}
		}
	}
	
	/**/
	comp_skeleton = ((CEntity*)mBoss)->get<TCompSkeleton>();
	TCompSkeleton* skeleton = comp_skeleton;
	/**/

	TCompRigidBody* R_hitch_rigid = ((CEntity*)R_hitch)->get<TCompRigidBody>();
	PxRigidDynamic*  R_hitch_px_rigid = R_hitch_rigid->rigidBody;

	PxQuat r_rot = Physics.XMVECTORToPxQuat(skeleton->getRotationOfBone(48));
	PxVec3 r_pos = Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(48));
	PxTransform r_bone_trans = PxTransform(r_pos, r_rot);

	R_hitch_joint->setLocalPose(PxJointActorIndex::eACTOR1, r_bone_trans);
	R_hitch_px_rigid->setGlobalPose(r_bone_trans);

	/**/
	TCompRigidBody* L_hitch_rigid = ((CEntity*)L_hitch)->get<TCompRigidBody>();
	PxRigidDynamic*  L_hitch_px_rigid = L_hitch_rigid->rigidBody;

	PxQuat l_rot = Physics.XMVECTORToPxQuat(skeleton->getRotationOfBone(23));
	PxVec3 l_pos = Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(23));
	PxTransform l_bone_trans = PxTransform(l_pos, l_rot);

	L_hitch_joint->setLocalPose(PxJointActorIndex::eACTOR1, l_bone_trans);
	L_hitch_px_rigid->setGlobalPose(l_bone_trans);

	/**/
	

	if (!is_death){
		/**/
		TCompRigidBody* H_hitch_rigid = ((CEntity*)H_hitch)->get<TCompRigidBody>();
		PxRigidDynamic*  H_hitch_px_rigid = H_hitch_rigid->rigidBody;

		PxQuat h_rot = Physics.XMVECTORToPxQuat(skeleton->getRotationOfBone(72));
		PxVec3 h_pos = Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(72));
		PxTransform h_bone_trans = PxTransform(h_pos, h_rot);

		H_hitch_joint->setLocalPose(PxJointActorIndex::eACTOR1, h_bone_trans);
		H_hitch_px_rigid->setGlobalPose(h_bone_trans);

		/**/
	}	

	if ((m_fsm_boss->getState() == "fbp_Stunned1") && (!can_break_hitch)) {
		can_break_hitch = true;		

		TCompTransform* R_hitch_trans = ((CEntity*)R_hitch)->get<TCompTransform>();
		if (R_hitch_trans) R_hitch_trans->setType(1);
		TCompTransform* L_hitch_trans = ((CEntity*)L_hitch)->get<TCompTransform>();
		if (L_hitch_trans) L_hitch_trans->setType(1);
	}
	else if ((m_fsm_boss->getState() != "fbp_Stunned1") && (can_break_hitch)) {
		can_break_hitch = false;

		TCompTransform* R_hitch_trans = ((CEntity*)R_hitch)->get<TCompTransform>();
		if (R_hitch_trans) R_hitch_trans->setType(0);
		TCompTransform* L_hitch_trans = ((CEntity*)L_hitch)->get<TCompTransform>();
		if (L_hitch_trans) L_hitch_trans->setType(0);
	}
	else if ((m_fsm_boss->getState() == "fbp_FinalState") && (!death_time)) {
		death_time = true;

		TCompTransform* H_hitch_trans = ((CEntity*)H_hitch)->get<TCompTransform>();
		if (H_hitch_trans) H_hitch_trans->setType(1);
	}


	// Update input
	/*
	if (CIOStatus::get().becomesPressed(CIOStatus::ALT)){
		shoot = true;
	}

	
	// Girar cosas a la derecha
	if (CIOStatus::get().becomesPressed(CIOStatus::O)){
		move_things = 2;
	}
	// Girar cosas a la izquierda
	if (CIOStatus::get().becomesPressed(CIOStatus::I)){
		move_things = 1;
	}
	// Girar cosas a la izquierda
	if (CIOStatus::get().becomesPressed(CIOStatus::U)){
		move_things = 0;
	}
	// Girar cosas a la izquierda
	if (CIOStatus::get().becomesPressed(CIOStatus::L)){
		move_things = 3;
	}
	*/
}

void TCompAiBoss::fixedUpdate(float elapsed){

	m_fsm_boss->update(elapsed);
	
}


void TCompAiBoss::breakHitch(CHandle m_hitch){
	if ((m_hitch == R_hitch)&&(can_break_hitch)) { 
		m_fsm_boss->EvaluateHit(1);
	}
	if ((m_hitch == L_hitch)&&(can_break_hitch)) { 
		m_fsm_boss->EvaluateHit(0);
	}
	if ((m_hitch == H_hitch) && (can_break_hitch)) {
		m_fsm_boss->HeartHit();
		H_hitch_joint->setBreakForce(0, 0);
		is_death = true;
	}
}

void TCompAiBoss::stun(){
	m_fsm_boss->HeadHit();
}