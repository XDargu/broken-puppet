#include "mcv_platform.h"
#include "fsm_boss.h"
#include "../components/all_components.h"
#include "components\comp_skeleton.h"
#include "components\comp_skeleton_lookat.h"
#include "handle\prefabs_manager.h"
#include "io\iostatus.h"

using namespace DirectX;

fsm_boss::fsm_boss()
{
}

fsm_boss::~fsm_boss()
{
}

void fsm_boss::Init()
{
	// insert all states in the map

	AddState("fbp_Proximity", (statehandler)&fsm_boss::Proximity);//
	AddState("fbp_Hidden", (statehandler)&fsm_boss::Hidden);
	AddState("fbp_RiseUp", (statehandler)&fsm_boss::RiseUp);
	AddState("fbp_Idle1", (statehandler)&fsm_boss::Idle1);//
	AddState("fbp_Hit1", (statehandler)&fsm_boss::Hit1);
	AddState("fbp_Stunned1", (statehandler)&fsm_boss::Stunned1);	
	AddState("fbp_Recover", (statehandler)&fsm_boss::Recover);
	AddState("fbp_WaveRight", (statehandler)&fsm_boss::WaveRight);//
	AddState("fbp_WaveLeft", (statehandler)&fsm_boss::WaveLeft);//
	AddState("fbp_Rain1Prepare", (statehandler)&fsm_boss::Rain1Prepare);//
	AddState("fbp_Rain1Loop", (statehandler)&fsm_boss::Rain1Loop);//
	AddState("fbp_Rain1Recover", (statehandler)&fsm_boss::Rain1Recover);//
	AddState("fbp_Ball1Initial", (statehandler)&fsm_boss::Ball1Initial);//
	AddState("fbp_Ball1Loop", (statehandler)&fsm_boss::Ball1Loop);//
	AddState("fbp_Ball1Launch", (statehandler)&fsm_boss::Ball1Launch);//
	AddState("fbp_Shoot1DownDef", (statehandler)&fsm_boss::Shoot1DownDef);//
	AddState("fbp_Shoot1Shoot", (statehandler)&fsm_boss::Shoot1Shoot);//
	AddState("fbp_Shoot1Reload", (statehandler)&fsm_boss::Shoot1Reload);//
	AddState("fbp_Shoot1ReleaseDef", (statehandler)&fsm_boss::Shoot1ReleaseDef);//
	AddState("fbp_Damaged1Left", (statehandler)&fsm_boss::Damaged1Left);
	AddState("fbp_Damaged1Right", (statehandler)&fsm_boss::Damaged1Right);
	AddState("fbp_Damaged1LeftFinal", (statehandler)&fsm_boss::Damaged1LeftFinal);
	AddState("fbp_Damaged1RightFinal", (statehandler)&fsm_boss::Damaged1RightFinal);
	AddState("fbp_FinalState", (statehandler)&fsm_boss::FinalState);
	AddState("fbp_Death", (statehandler)&fsm_boss::Death);


	// reset the state
	ChangeState("fbp_Hidden");

	comp_skeleton = ((CEntity*)entity)->get<TCompSkeleton>();
	comp_skeleton_lookat = ((CEntity*)entity)->get<TCompSkeletonLookAt>();
	comp_ragdoll = ((CEntity*)entity)->get<TCompRagdoll>();

	if (comp_ragdoll.isValid()) {
		((TCompRagdoll*)comp_ragdoll)->setActive(false);
	}

	TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
	skeleton_lookat->active = true;

	need_reorientate = false;
	reorientate_angle = deg2rad(40.f);
	reorientate_angle = cosf(reorientate_angle);
	no_reorientate_angle = deg2rad(5.f);
	no_reorientate_angle = cosf(no_reorientate_angle);


	state_time = 0.f;
	has_left = true;
	has_right = true;

	hurt_state = 0;
	shoots_amount = 0;

	last_attack = 0.f;

	debris_created = 0;
	debris_creation_delay = 0;
	bomb_creation_delay = 0;

	// Init vars
	point_offset = PxVec3(0, 10, 0);
	distance_to_point = 12;
	
	m_entity_manager = &CEntityManager::get();

	m_player = m_entity_manager->getByName("Player");

	obj_distance = 0;
	obj_selected = CHandle();
	distance_to_hand = 2.f;

	force = 24;
	ball_size = 60000;

	last_anim_id = -1;

	can_proximity = false;
	can_proximity_hit = true;
}

void fsm_boss::Hidden(){
	int i = 0;
	if (CIOStatus::get().becomesPressed(CIOStatus::V)){		
		ChangeState("fbp_RiseUp");
	}
	
}

void fsm_boss::RiseUp(){
	if (on_enter){
		Release_def();
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(35);
		TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
		skeleton_lookat->active = false;
	}
	if (state_time >= 20.9f){
		ChangeState("fbp_Idle1");
	}
	
}

void fsm_boss::Idle1(float elapsed){

	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		loopAnimationIfNotPlaying(0, true);
		TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
		skeleton_lookat->active = true;

		shoots_amount = 0;
		last_attack = 0.f;
		can_proximity = true;
		can_proximity_hit = true;
	}

	Reorientate(elapsed, false);
	last_attack += elapsed;
	
	CIOStatus& io = CIOStatus::get();
	/**/
	if (last_attack > 3){
		int attack = Calculate_attack();
		//int attack = 1;
		switch (attack)
		{
		case 0:
			ChangeState("fbp_Rain1Prepare");
			break;

		case 1:
			ChangeState("fbp_Shoot1DownDef");
			break;

		case 2:
			ChangeState("fbp_Ball1Initial");
			break;

		case 3:
			ChangeState("fbp_WaveRight");
			break;
		case 4:
			ChangeState("fbp_WaveLeft");
			break;
		default:
			break;
		}
	}

	/**/

	// Update input
	/**
	if (CIOStatus::get().becomesPressed(CIOStatus::P)){
		ChangeState("fbp_Ball1Initial");
	}
	// Bajar defensas y ataque simple
	if (CIOStatus::get().becomesPressed(CIOStatus::O)){
		ChangeState("fbp_Shoot1DownDef");
	}
	// Rain de objetos
	if (CIOStatus::get().becomesPressed(CIOStatus::I)){
		ChangeState("fbp_Rain1Prepare");
	}
	// Girar golpe de objeto en cabeza
	if (CIOStatus::get().becomesPressed(CIOStatus::U)){
		ChangeState("fbp_Hit1");
	}
	// Leche de proximidad
	if (CIOStatus::get().becomesPressed(CIOStatus::L)){
		ChangeState("fbp_Proximity");
	}
	// Girar cosas a la Derecha
	if (CIOStatus::get().becomesPressed(CIOStatus::M)){
		ChangeState("fbp_WaveRight");
	}
	// Girar cosas a la izquierda
	if (CIOStatus::get().becomesPressed(CIOStatus::N)){
		ChangeState("fbp_WaveLeft");
	}
	// Muerte
	if (CIOStatus::get().becomesPressed(CIOStatus::H)){
		ChangeState("fbp_Death");
	}
	// Hacer danio a la izquierda
	if (CIOStatus::get().becomesPressed(CIOStatus::V)){
		EvaluateHit(0);
	}
	// Hacer danio a la derecha
	if (CIOStatus::get().becomesPressed(CIOStatus::B)){
		EvaluateHit(1);
	}
	// Hacer danio a la izquierda final
	if (CIOStatus::get().becomesPressed(CIOStatus::K)){
		EvaluateHit(2);
	}
	// Hacer danio a la derecha final
	if (CIOStatus::get().becomesPressed(CIOStatus::J)){
		EvaluateHit(3);
	}
	/**/
}

void fsm_boss::Hit1(float elapsed){
	if (on_enter){
		Release_def();
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(1);
		TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
		skeleton_lookat->active = false;

		can_proximity = false;
	}

	float time = getAnimationDuration(last_anim_id);

	if (state_time >= 1.6f){
		ChangeState("fbp_Stunned1");
	}
}

void fsm_boss::Stunned1(){
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		loopAnimationIfNotPlaying(2, true);
	}

	// Hacer danio a la izquierda
	if (CIOStatus::get().becomesPressed(CIOStatus::V)){
		EvaluateHit(0);
	}
	// Hacer danio a la derecha
	if (CIOStatus::get().becomesPressed(CIOStatus::B)){
		EvaluateHit(1);
	}

	if (state_time >= 10){
		ChangeState("fbp_Recover");
	}
}

void fsm_boss::Recover(float elapsed){
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(3);
		last_anim_id = -1;
	}
	if (state_time >= 3.9f){
		ChangeState("fbp_Idle1");
	}
	
}


void fsm_boss::Rain1Prepare(){
	Reorientate(0, true);
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(11);
		last_anim_id = -1;
	}
	if (state_time >= 1.3f){
		ChangeState("fbp_Rain1Loop");
	}
}

void fsm_boss::Rain1Loop(float elapsed){
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		loopAnimationIfNotPlaying(12, true);

		TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
		skeleton_lookat->active = false;
		bomb_creation_delay = 0;
		debris_creation_delay = 0;
		debris_created = 0;
	}

	if (!RainDebris(elapsed)){
		ChangeState("fbp_Rain1Recover");
	}

}

void fsm_boss::Rain1Recover(){
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(13);
		last_anim_id = -1;

		TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
		skeleton_lookat->active = false;
	}
	if (state_time >= 0.6f){
		ChangeState("fbp_Idle1");
	}
}


void fsm_boss::Proximity(float elapsed){
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(4);
		last_anim_id = -1;
		can_proximity = false;

		
	}

	if ((state_time >= 0.6f)&&(can_proximity_hit)){

		can_proximity_hit = false;
		((CEntity*)m_player)->sendMsg(TActorHit(entity, 110000.f, true));

	}
	

	if (state_time >= 1.49f){
		ChangeState("fbp_Idle1");
		
	}
}

void fsm_boss::Ball1Initial(float elapsed){

	Reorientate(elapsed, false);

	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(17);
		last_anim_id = -1;

		ball_list.clear();
		float aux_ball_size = 0;

		// Get obj till mass

		for (int i = 0; i < m_entity_manager->rigid_list.size(); ++i){
			CEntity* e = m_entity_manager->rigid_list[i];
			if (!e->hasTag("player")){
				TCompRigidBody* rigid = e->get<TCompRigidBody>();
				bool bossAccess = rigid->boss_level == 0;
				if (bossAccess){
					aux_ball_size += rigid->getMass();
					ball_list.push_back((CHandle)e);					
				}
			}
			if (aux_ball_size >= ball_size) break;
		}
	}

	if (state_time >= 2.6f){
		ChangeState("fbp_Ball1Loop");
	}

}


void fsm_boss::Ball1Loop(float elapsed){
	Reorientate(elapsed, false);

	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		loopAnimationIfNotPlaying(19, true);
	}

	TCompTransform* enemy_comp_trans = ((CEntity*)entity)->get<TCompTransform>();

	// Get the enemy position
	PxVec3 enemy_pos = Physics.XMVECTORToPxVec3(enemy_comp_trans->position);

	TCompTransform* player_comp_trans = (((CEntity*)m_player)->get<TCompTransform>());
	PxVec3 player_pos = Physics.XMVECTORToPxVec3(player_comp_trans->position);


	// Calculate direction player enemy
	PxVec3 player_boss_dir = (player_pos - enemy_pos);
	player_boss_dir.y = 0;
	player_boss_dir = player_boss_dir.getNormalized();

	PxVec3	point_to_go = enemy_pos + point_offset + (player_boss_dir * distance_to_point);

	if (state_time < 10){
		for (int i = 0; i < ball_list.size(); ++i){
			CEntity* e = ball_list[i];
			if (((CHandle)e).isValid()){
				TCompRigidBody* rigid = e->get<TCompRigidBody>();
				if (((CHandle)rigid).isValid()){

					PxRigidDynamic*  px_rigid = rigid->rigidBody;

					PxVec3 force_dir = (point_to_go - px_rigid->getGlobalPose().p).getNormalized();
					px_rigid->addForce(force_dir * force, PxForceMode::eACCELERATION, true);
					px_rigid->setLinearDamping(0.05f);
					px_rigid->setAngularDamping(0.05f);

					float aux_dist = (px_rigid->getGlobalPose().p - point_to_go).magnitude();
					if (aux_dist < 4){
						px_rigid->setLinearDamping(1);
						px_rigid->setAngularDamping(0.5f);
					}
				}
			}
		}
	}
	else {
		ChangeState("fbp_Ball1Launch");
	}
}

void fsm_boss::Ball1Launch(float elapsed){

	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(18);
		last_anim_id = -1;
	}

	if (on_enter){

		TCompTransform* player_comp_trans = (((CEntity*)m_player)->get<TCompTransform>());
		PxVec3 player_pos = Physics.XMVECTORToPxVec3(player_comp_trans->position);

		for (int i = 0; i < ball_list.size(); ++i){
			CEntity* e = ball_list[i];
			if (((CHandle)e).isValid()){
				TCompRigidBody* rigid = e->get<TCompRigidBody>();
				if (((CHandle)rigid).isValid()){
					TCompRigidBody* rigid = e->get<TCompRigidBody>();
					PxRigidDynamic*  px_rigid = rigid->rigidBody;
					PxVec3 force_dir = (player_pos - px_rigid->getGlobalPose().p).getNormalized();
					px_rigid->addForce(force_dir * force * 2, PxForceMode::eVELOCITY_CHANGE, true);
					px_rigid->setLinearDamping(0.05f);
					px_rigid->setAngularDamping(0.05f);
				}
			}
		}
	}
	if (state_time >= 2.3f){
		ChangeState("fbp_Idle1");
	}
}
//Shoot1
void fsm_boss::Shoot1DownDef(){
	Reorientate(0.f, true);

	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();

		if (hurt_state > 0){
			if (has_left){
				skeleton->playAnimation(27);
			}
			else{
				skeleton->playAnimation(31);
			}
		}
		else { 
			skeleton->playAnimation(14); 
		}

		last_anim_id = -1;
	}

	if (on_enter){
		// Set Down protections
		for (int i = 0; i < m_entity_manager->rigid_list.size(); ++i){
			CEntity* e = m_entity_manager->rigid_list[i];
			TCompRigidBody* rigid = e->get<TCompRigidBody>();
			bool bossAccess = rigid->boss_level == 1;
			if (bossAccess){
				PxRigidBody*  px_rigid = rigid->rigidBody;
				PxVec3 force_dir = (PxVec3(0, -1, 0));
				px_rigid->addForce(force_dir * force, PxForceMode::eACCELERATION, true);
				px_rigid->setMass(10000000);
				if (((PxRigidDynamic*)px_rigid)->isRigidActor()){
					if (((PxRigidDynamic*)px_rigid)->isSleeping()){
						((PxRigidDynamic*)px_rigid)->wakeUp();
					}
				}
			}
		}
	}
	if (state_time >= 2.3f){
		ChangeState("fbp_Shoot1Reload");
	}
	
}

void fsm_boss::Shoot1Reload(){

	Reorientate(0.f, true);

	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();

		if (hurt_state > 0){
			if (has_left){
				loopAnimationIfNotPlaying(30, true);
			}
			else{
				loopAnimationIfNotPlaying(34, true);
			}
		}
		else {
			loopAnimationIfNotPlaying(25, true);
		}

		obj_to_shoot = nullptr;

		SelectObjToShoot();
	}

	if (obj_to_shoot.isValid()){
		if (shoots_amount <= 2){
			TCompTransform* enemy_comp_trans = ((CEntity*)entity)->get<TCompTransform>();

			// Get the enemy position
			PxVec3 enemy_pos = Physics.XMVECTORToPxVec3(enemy_comp_trans->position);
			TCompTransform* player_comp_trans = (((CEntity*)m_player)->get<TCompTransform>());
			PxVec3 player_pos = Physics.XMVECTORToPxVec3(player_comp_trans->position);
			TCompSkeleton* skeleton = comp_skeleton;


			// Calculate direction player enemy
			PxVec3 player_boss_dir = (player_pos - enemy_pos);
			player_boss_dir.y = 0;
			player_boss_dir = player_boss_dir.getNormalized();

			PxVec3	point_to_go = PxVec3(0, 0, 0);
			if (has_right)	{
				point_to_go = Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(40)) + (player_boss_dir * (distance_to_hand));
			}
			else{
				point_to_go = Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(15)) + (player_boss_dir * (distance_to_hand));
			}
			

			CEntity* m_e = obj_to_shoot;
			TCompRigidBody* rigid = m_e->get<TCompRigidBody>();

			const char *name = m_e->getName();
			PxRigidBody*  px_rigid = rigid->rigidBody;
			PxVec3 force_dir = (point_to_go - px_rigid->getGlobalPose().p).getNormalized();
			float dist = (point_to_go - px_rigid->getGlobalPose().p).magnitude();

			if (dist > 10) {
				px_rigid->addForce(force_dir * 2, PxForceMode::eVELOCITY_CHANGE, true);
			}
			else {
				px_rigid->setLinearVelocity(force_dir * clamp(dist * 2, 0, 12));
			}

			if (state_time >= 2.f){
				ChangeState("fbp_Shoot1Shoot");
			}
		}
		else if (state_time >= 2.f){
			ChangeState("fbp_Shoot1ReleaseDef");
		}
	}
	else if (state_time >= 2.f){
		ChangeState("fbp_Shoot1ReleaseDef");
	}
}

//Shoot
void fsm_boss::Shoot1Shoot(){
	Reorientate(0.f, true);
	
	static PxVec3 force_dir;

	if (on_enter){
		CEntity* m_e = obj_to_shoot;
		TCompRigidBody* rigid = m_e->get<TCompRigidBody>();
		PxRigidBody*  px_rigid = rigid->rigidBody;

		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();

		if (hurt_state > 0){
			if (has_left){
				skeleton->playAnimation(28);
			}
			else{
				skeleton->playAnimation(32);
			}
		}
		else {
			skeleton->playAnimation(15);
		}

		
		last_anim_id = -1;
		shoots_amount += 1;

		//Shoot 
		TCompTransform* player_comp_trans = (((CEntity*)m_player)->get<TCompTransform>());
		PxVec3 player_pos = Physics.XMVECTORToPxVec3(player_comp_trans->position);

		force_dir = (player_pos - px_rigid->getGlobalPose().p).getNormalized();


	}

	CEntity* m_e = obj_to_shoot;
	if (m_e) {
		TCompRigidBody* rigid = m_e->get<TCompRigidBody>();
		if (rigid) {
			PxRigidBody*  px_rigid = rigid->rigidBody;

			px_rigid->addForce(force_dir, PxForceMode::eVELOCITY_CHANGE, true);
			//px_rigid->setLinearVelocity(force_dir);
		}
	}


	// Set Down protections
	if (state_time >= 1.3f){
		ChangeState("fbp_Shoot1Reload");
	}
}

//Shoot1ReleaseDef
void fsm_boss::Shoot1ReleaseDef(){
	Reorientate(0.f, true);
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();

		if (hurt_state < 1){
			if (has_left){
				skeleton->playAnimation(29);
			}
			else{
				skeleton->playAnimation(33);
			}
		}
		else {
			skeleton->playAnimation(16);
		}
		
		last_anim_id = -1;
	}
	if (on_enter){

		for (int i = 0; i < m_entity_manager->rigid_list.size(); ++i){
			CEntity* e = m_entity_manager->rigid_list[i];
			TCompRigidBody* rigid = e->get<TCompRigidBody>();
			bool bossAccess = rigid->boss_level == 1;
			if (bossAccess){
				PxRigidBody*  px_rigid = rigid->rigidBody;
				PxVec3 force_dir = (PxVec3(0, -1, 0));
				//px_rigid->addForce(force_dir * force * 3, PxForceMode::eACCELERATION, true);
				px_rigid->setMass(700);
				if (((PxRigidDynamic*)px_rigid)->isRigidActor()){
					if (((PxRigidDynamic*)px_rigid)->isSleeping()){
						((PxRigidDynamic*)px_rigid)->wakeUp();
					}
				}
			}
		}
	}	
	if (state_time >= 1.f){
		ChangeState("fbp_Idle1");
	}
	
}

//WaveLeft
void fsm_boss::WaveLeft(){
	Reorientate(0.f, true);
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(5);
		last_anim_id = -1;
	}	
	if (state_time >= 2.3f){
		ChangeState("fbp_Idle1");
	}

}

//WaveRight
void fsm_boss::WaveRight(){
	Reorientate(0.f, true);
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(6);
		last_anim_id = -1;
	}
	if (state_time >= 2.3f){
		ChangeState("fbp_Idle1");
	}
}


//FinalState
void fsm_boss::FinalState(){
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		loopAnimationIfNotPlaying(26, true);
		TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
		skeleton_lookat->active = false;
	}
}

void fsm_boss::Damaged1Left(float elapsed){
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(21);
		last_anim_id = -1;
		TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
		skeleton_lookat->active = false;

		// Brazo izquierdo desabilitado y su joint roto
		TCompRagdoll* ragdoll = comp_ragdoll;
		ragdoll->disableBoneTree(11);
		ragdoll->breakJoint(6);
	}
	if (state_time >= 1.f){
		Reorientate(elapsed, false);
	}

	if (state_time >= 2.6f){
		ChangeState("fbp_Idle1");
	}
}

void fsm_boss::Damaged1Right(float elapsed){
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(23);
		last_anim_id = -1;
		TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
		skeleton_lookat->active = false;

		// Brazo derecho desabilitado y su joint roto
		TCompRagdoll* ragdoll = comp_ragdoll;
		ragdoll->disableBoneTree(36);
		ragdoll->breakJoint(4);
	}
	if (state_time >= 1.f){
		Reorientate(elapsed, false);
	}
	if (state_time >= 2.6f){
		ChangeState("fbp_Idle1");
	}
}


void fsm_boss::Damaged1LeftFinal(){
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(22);
		last_anim_id = -1;
		TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
		skeleton_lookat->active = false;

		// Brazo izquierdo desabilitado y su joint roto
		TCompRagdoll* ragdoll = comp_ragdoll;
		ragdoll->disableBoneTree(11);
		ragdoll->breakJoint(6);
	}
	if (state_time >= 1.f){
		ChangeState("fbp_FinalState");
	}
}

void fsm_boss::Damaged1RightFinal(){
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(24);
		last_anim_id = -1;
		TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
		skeleton_lookat->active = false;

		// Brazo derecho desabilitado y su joint roto
		TCompRagdoll* ragdoll = comp_ragdoll;
		ragdoll->disableBoneTree(36);
		ragdoll->breakJoint(4);
	}
	if (state_time >= 1.f){
		ChangeState("fbp_FinalState");
	}
}


//Dead
void fsm_boss::Death(){

	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(20);
		last_anim_id = -1;
		TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
		skeleton_lookat->active = false;

		/*
		// Prueba: desactivar árbol base (para que el ragdoll esté activo, pero solo la parte superior)
		TCompRagdoll* ragdoll = comp_ragdoll;
		ragdoll->disableBoneTree(4);
		/**/
	}
	
	if (state_time >= 7.f){
		TCompRagdoll* ragdoll = comp_ragdoll;
		ragdoll->enableBoneTree(4);
		ragdoll->enableBoneTree(36);
		ragdoll->enableBoneTree(11);
		ChangeState("fbp_Idle1");
	}
}

//Reorientate: this method handle the look at player and the reorientation
void fsm_boss::Reorientate(float elapsed, bool just_look){

	TCompTransform* player_comp_trans = (((CEntity*)m_player)->get<TCompTransform>());	
	TCompTransform* enemy_comp_trans = ((CEntity*)entity)->get<TCompTransform>();

	TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
	skeleton_lookat->target = player_comp_trans->position;
	skeleton_lookat->active = true;

	if (!just_look){

		// Get the enemy position
		PxVec3 enemy_pos = Physics.XMVECTORToPxVec3(enemy_comp_trans->position);

		PxVec3 player_pos = Physics.XMVECTORToPxVec3(player_comp_trans->position);

		// Calculate direction player enemy
		PxVec3 player_boss_dir = (player_pos - enemy_pos);
		player_boss_dir.y = 0;
		player_boss_dir = player_boss_dir.getNormalized();

		// Get the enemy front	

		float angle = getAngleBetweenVectors(enemy_comp_trans->getFront(), Physics.PxVec3ToXMVECTOR(player_boss_dir));
		// check if rotation is necesary
		// if angle between boss forward and player and boss direction
		if ((angle <= reorientate_angle) && (!need_reorientate)){
			need_reorientate = true;
		}
		if (need_reorientate){
			XMVECTOR aux_pos = player_comp_trans->position;
			aux_pos = XMVectorSetY(aux_pos, XMVectorGetY(enemy_comp_trans->position));

			enemy_comp_trans->aimAt(aux_pos, enemy_comp_trans->getUp(), 0.8f * elapsed);
			if (angle >= no_reorientate_angle){
				need_reorientate = false;
			}
		}
	}

}

void fsm_boss::Release_def(){

	for (int i = 0; i < m_entity_manager->rigid_list.size(); ++i){
		CEntity* e = m_entity_manager->rigid_list[i];
		TCompRigidBody* rigid = e->get<TCompRigidBody>();
		bool bossAccess = rigid->boss_level == 1;

		if (bossAccess){

			PxRigidBody*  px_rigid = rigid->rigidBody;
			PxVec3 force_dir = (PxVec3(0, -1, 0));
			px_rigid->setMass(700);

			if (((PxRigidDynamic*)px_rigid)->isRigidActor()){
				if (((PxRigidDynamic*)px_rigid)->isSleeping()){
					((PxRigidDynamic*)px_rigid)->wakeUp();
				}

			}
		}
	}
}

void fsm_boss::stopAllAnimations() {
	TCompSkeleton* m_skeleton = comp_skeleton;
	
	for (int i = 0; i < 50; ++i) {
		m_skeleton->model->getMixer()->clearCycle(i, 0.3f);
	}
}

void fsm_boss::loopAnimationIfNotPlaying(int id, bool restart) {
	TCompSkeleton* m_skeleton = comp_skeleton;
	if (id != last_anim_id) {
		if (restart) {
			m_skeleton->resetAnimationTime();
		}
		stopAnimation(last_anim_id);
		last_anim_id = id;
		m_skeleton->loopAnimation(id);
	}
}

void fsm_boss::stopAnimation(int id) {
	TCompSkeleton* m_skeleton = comp_skeleton;
	m_skeleton->stopAnimation(id);
}

float fsm_boss::getAnimationDuration(int id) {
	TCompSkeleton* m_skeleton = comp_skeleton;

	float res = m_skeleton->model->getMixer()->getAnimationDuration();
	return res;
}

bool fsm_boss::EvaluateHit(int arm_damaged) {
	
	// 0 left arm
	if (arm_damaged == 0){
		if (has_left){
			if (hurt_state == 0){
				ChangeState("fbp_Damaged1Left");
			}
			else{
				ChangeState("fbp_Damaged1LeftFinal");
			}
			has_left = false;
			hurt_state += 1;
		}		
	}
	// 1 right arm
	if (arm_damaged == 1){
		if (has_right){
			if (hurt_state == 0){
				ChangeState("fbp_Damaged1Right");
			}
			else{
				ChangeState("fbp_Damaged1RightFinal");
			}
			has_right = false;
			hurt_state += 1;
		}
	}
	return false;
}

int fsm_boss::Calculate_attack() {
	
	last_attack = 0.f;
	int rnd = 0;	
	if (m_entity_manager->rigid_list.size() < 70){
		rnd = 0;
	}
	else if (m_entity_manager->rigid_list.size() > 330){
		//rnd = 1;
		rnd = getRandomNumber(1, 4);
	}
	else {
		//rnd = 1;
		rnd = getRandomNumber(1, 4);
	}
	return rnd;
}

bool fsm_boss::RainDebris(float elapsed){

	// Cargar un prefab
	int debris_amount = 200;
	float debris_respawn_time = 0.05f;
	float bomb_respawn_time = 1.f;
	bool active = true;

	if (debris_created <= debris_amount){

		debris_creation_delay += elapsed;
		// Debris
		if (debris_creation_delay >= debris_respawn_time){
			debris_creation_delay = 0;

			int rnd = getRandomNumber(1, 20);
			std::string name = "boss/debris_0" + std::to_string(rnd);
			CEntity* prefab_entity = prefabs_manager.getInstanceByName(name.c_str());

			XMVECTOR random_point = getRandomVector3(-20, 60, -10, 20, 61, 30);
			TCompTransform* prefab_t = prefab_entity->get<TCompTransform>();
			prefab_t->init();
			prefab_t->teleport(random_point);

			debris_created++;
		}

		bomb_creation_delay += elapsed;
		// Bombs
		if (bomb_creation_delay >= bomb_respawn_time){
			bomb_creation_delay = 0;
			std::string name = "boss/bomb";
			CEntity* prefab_entity = prefabs_manager.getInstanceByName(name.c_str());

			XMVECTOR random_point = getRandomVector3(-20, 60, -10, 20, 61, 30);
			TCompTransform* prefab_t = prefab_entity->get<TCompTransform>();
			prefab_t->init();
			TCompParticleGroup* prefab_pg = prefab_entity->get<TCompParticleGroup>();
			prefab_pg->init();
			TCompExplosion* prefab_E = prefab_entity->get<TCompExplosion>();
			prefab_E->init();

			prefab_t->teleport(random_point);
		}
	}
	else{
		active = false;
	}

	return active;
}


void fsm_boss::HeadHit() {

	std::string m_current_state = getState();

	bool can_hit =
		   (m_current_state != "fbp_Hidden")
		&& (m_current_state != "fbp_RiseUp")
		&& (m_current_state != "fbp_Hit1")
		&& (m_current_state != "fbp_Stunned1")
		&& (m_current_state != "fbp_Recover")
		&& (m_current_state != "fbp_Damaged1Left")
		&& (m_current_state != "fbp_Damaged1Right")
		&& (m_current_state != "fbp_Damaged1LeftFinal")
		&& (m_current_state != "fbp_Damaged1RightFinal")
		&& (m_current_state != "fbp_FinalState")
		&& (m_current_state != "fbp_Death");
	
	if (can_hit){
		ChangeState("fbp_Hit1");
	}
}

bool fsm_boss::HeartHit() {

	std::string m_current_state = getState();
	bool hitted = false;

	bool can_hit = (m_current_state == "fbp_FinalState");

	if (can_hit){
		ChangeState("fbp_Death");
		hitted = true;
	}

	return hitted;
}

void fsm_boss::SelectObjToShoot() {
	CHandle obj_aux;
	float dist = 10000000000;

	for (int i = 0; i < m_entity_manager->rigid_list.size(); ++i){
		CEntity* e = m_entity_manager->rigid_list[i];

		if (!e->hasTag("player")){
			TCompRigidBody* rigid = e->get<TCompRigidBody>();
			bool bossAccess = rigid->boss_level == 0;
			if (bossAccess){
				// Take the distance
				TCompTransform* enemy_comp_trans = ((CEntity*)entity)->get<TCompTransform>();
				TCompTransform* obj_comp_trans = e->get<TCompTransform>();

				if (!obj_to_shoot.isValid()) { obj_to_shoot = e; }
				
				TCompSkeleton* skeleton = comp_skeleton;
				
				float dist_aux = V3DISTANCE(skeleton->getPositionOfBone(40), obj_comp_trans->position);
				if ((dist_aux <= dist)&&(rigid->getMass() > 1500.f)){
					dist = dist_aux;
					obj_to_shoot = e;
				}				
			}
		}
	}
}