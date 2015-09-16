#include "mcv_platform.h"
#include "fsm_boss.h"
#include "../components/all_components.h"
#include "components\comp_skeleton.h"
#include "components\comp_skeleton_lookat.h"
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

	AddState("fbp_Proximity", (statehandler)&fsm_boss::Proximity);
	AddState("fbp_Hidden", (statehandler)&fsm_boss::Hidden);
	AddState("fbp_RiseUp", (statehandler)&fsm_boss::RiseUp);
	AddState("fbp_Idle1", (statehandler)&fsm_boss::Idle1);
	AddState("fbp_Hit1", (statehandler)&fsm_boss::Hit1);
	AddState("fbp_Stunned1", (statehandler)&fsm_boss::Stunned1);	
	AddState("fbp_Recover", (statehandler)&fsm_boss::Recover);
	AddState("fbp_WaveRight", (statehandler)&fsm_boss::WaveRight);
	AddState("fbp_WaveLeft", (statehandler)&fsm_boss::WaveLeft);
	AddState("fbp_Rain1Prepare", (statehandler)&fsm_boss::Rain1Prepare);
	AddState("fbp_Rain1Loop", (statehandler)&fsm_boss::Rain1Loop);
	AddState("fbp_Rain1Recover", (statehandler)&fsm_boss::Rain1Recover);
	AddState("fbp_Ball1Initial", (statehandler)&fsm_boss::Ball1Initial);
	AddState("fbp_Ball1Loop", (statehandler)&fsm_boss::Ball1Loop);
	AddState("fbp_Ball1Launch", (statehandler)&fsm_boss::Ball1Launch);
	AddState("fbp_Shoot1DownDef", (statehandler)&fsm_boss::Shoot1DownDef);
	AddState("fbp_Shoot1Shoot", (statehandler)&fsm_boss::Shoot1Shoot);
	AddState("fbp_Shoot1ReleaseDef", (statehandler)&fsm_boss::Shoot1ReleaseDef);
	AddState("fbp_Damaged1Left", (statehandler)&fsm_boss::Damaged1Left);
	AddState("fbp_Damaged1Right", (statehandler)&fsm_boss::Damaged1Right);

	AddState("fbp_Idle2", (statehandler)&fsm_boss::Idle2);
	AddState("fbp_Stunned2", (statehandler)&fsm_boss::Stunned2);
	AddState("fbp_Wave", (statehandler)&fsm_boss::Wave);
	AddState("fbp_Rain2", (statehandler)&fsm_boss::Rain2);
	AddState("fbp_Ball2", (statehandler)&fsm_boss::Ball2);
	AddState("fbp_Shoot2", (statehandler)&fsm_boss::Shoot2);
	AddState("fbp_Damaged2", (statehandler)&fsm_boss::Damaged2);
	AddState("fbp_FinalState", (statehandler)&fsm_boss::FinalState);
	AddState("fbp_Death", (statehandler)&fsm_boss::Death);


	// reset the state
	ChangeState("fbp_Hidden");

	comp_skeleton = ((CEntity*)entity)->get<TCompSkeleton>();
	comp_skeleton_lookat = ((CEntity*)entity)->get<TCompSkeletonLookAt>();

	TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
	skeleton_lookat->active = true;

	need_reorientate = false;
	reorientate_angle = deg2rad(40.f);
	reorientate_angle = cosf(reorientate_angle);
	no_reorientate_angle = deg2rad(5.f);
	no_reorientate_angle = cosf(no_reorientate_angle);


	state_time = 0.f;
	arm_state = 0;
	hurt_state = 0;

	// Init vars
	point_offset = PxVec3(0, 10, 0);
	distance_to_point = 12;
	
	m_entity_manager = &CEntityManager::get();

	m_player = m_entity_manager->getByName("Player");

	obj_distance = 0;
	obj_selected = CHandle();

	force = 24;

	last_anim_id = -1;
}

void fsm_boss::Hidden(){
	int i = 0;
	ChangeState("fbp_RiseUp");
}


void fsm_boss::RiseUp(){
	int i = 0;
	ChangeState("fbp_Idle1");
}

void fsm_boss::Idle1(float elapsed){

	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		loopAnimationIfNotPlaying(0, true);
		TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
		skeleton_lookat->active = true;
	}

	Reorientate(elapsed);
	
	CIOStatus& io = CIOStatus::get();
	// Update input

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
}

void fsm_boss::Hit1(float elapsed){
	if (on_enter){
		Release_def();
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(1);
		TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
		skeleton_lookat->active = false;
	}

	float time = getAnimationDuration(last_anim_id);

	if (state_time >= 1.16f){
		ChangeState("fbp_Stunned1");
	}
}

void fsm_boss::Stunned1(){
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		loopAnimationIfNotPlaying(2, true);
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

void fsm_boss::Rain1Loop(){
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		loopAnimationIfNotPlaying(12, true);
	}
	if (state_time >= 5){
		ChangeState("fbp_Rain1Recover");
	}
}

void fsm_boss::Rain1Recover(){
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(13);
		last_anim_id = -1;
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
	}
	if (state_time >= 0.6f){
		ChangeState("fbp_Idle1");
	}
}

void fsm_boss::Ball1Initial(float elapsed){

	Reorientate(elapsed);

	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(17);
		last_anim_id = -1;
	}

	if (state_time >= 0.4f){
		ChangeState("fbp_Ball1Loop");
	}

}


void fsm_boss::Ball1Loop(float elapsed){
	Reorientate(elapsed);

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
		for (int i = 0; i < m_entity_manager->rigid_list.size(); ++i){
			CEntity* e = m_entity_manager->rigid_list[i];
			if (!e->hasTag("player")){
				TCompRigidBody* rigid = e->get<TCompRigidBody>();
				bool bossAccess = rigid->boss_level == 0;
				if (bossAccess){
					PxRigidBody*  px_rigid = rigid->rigidBody;
					PxVec3 force_dir = (point_to_go - px_rigid->getGlobalPose().p).getNormalized();
					px_rigid->addForce(force_dir * force, PxForceMode::eACCELERATION, true);
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

		for (int i = 0; i < CEntityManager::get().rigid_list.size(); ++i){
			CEntity* e = CEntityManager::get().rigid_list[i];

			if (!e->hasTag("player")){
				TCompRigidBody* rigid = e->get<TCompRigidBody>();
				bool bossAccess = rigid->boss_level == 0;

				if (bossAccess){
					PxRigidBody*  px_rigid = rigid->rigidBody;
					PxVec3 force_dir = (player_pos - px_rigid->getGlobalPose().p).getNormalized();
					px_rigid->addForce(force_dir * force, PxForceMode::eVELOCITY_CHANGE, true);
				}

			}
		}
	}

	if (state_time >= 1.1f){
		ChangeState("fbp_Idle1");
	}


}
//Shoot1
void fsm_boss::Shoot1DownDef(){

	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(14);
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
		ChangeState("fbp_Shoot1Shoot");
	}
	
}

//Shoot
void fsm_boss::Shoot1Shoot(){
	
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		loopAnimationIfNotPlaying(15, true);
	}

	// Take and shoot loop
	// take obj
	// check distance
	// shoot

	// Leave protections

	// Set Down protections
	if (state_time >= 5){
		ChangeState("fbp_Shoot1ReleaseDef");
	}
}

//Shoot1ReleaseDef
void fsm_boss::Shoot1ReleaseDef(){

	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(16);
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
	if (state_time >= 1.3f){
		ChangeState("fbp_Idle1");
	}
	
}

//WaveLeft
void fsm_boss::WaveLeft(){
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

//Idle2
void fsm_boss::Idle2(){
	int i = 0;
}

//Stunned2
void fsm_boss::Stunned2(){
	int i = 0;
}

//Wave
void fsm_boss::Wave(){
	int i = 0;
}

//Rain2
void fsm_boss::Rain2(){
	int i = 0;
}

//Ball2
void fsm_boss::Ball2(){
	int i = 0;
}

//Shoot2
void fsm_boss::Shoot2(){
	int i = 0;
}

//Damaged2
void fsm_boss::Damaged2(){
	int i = 0;
}

//FinalState
void fsm_boss::FinalState(){
	int i = 0;
}

void fsm_boss::Damaged1Left(){
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(21);
		last_anim_id = -1;
		TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
		skeleton_lookat->active = false;
	}
	if (state_time >= 2.6f){
		ChangeState("fbp_Idle1");
	}
}

void fsm_boss::Damaged1Right(){
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(23);
		last_anim_id = -1;
		TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
		skeleton_lookat->active = false;
	}
	if (state_time >= 2.6f){
		ChangeState("fbp_Idle1");
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
	}
	if (state_time >= 7.f){
		ChangeState("fbp_Idle1");
	}
}

//Reorientate: this method handle the look at player and the reorientation
void fsm_boss::Reorientate(float elapsed){

	TCompTransform* player_comp_trans = (((CEntity*)m_player)->get<TCompTransform>());	
	TCompTransform* enemy_comp_trans = ((CEntity*)entity)->get<TCompTransform>();

	TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
	skeleton_lookat->target = player_comp_trans->position;


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

	for (int i = 0; i < 20; ++i) {
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
	
	// Simplificada	
	if (arm_damaged == 0){
		ChangeState("fbp_Damaged1Left");
	}
	if (arm_damaged == 1){
		ChangeState("fbp_Damaged1Right");
	}

	return false;
}