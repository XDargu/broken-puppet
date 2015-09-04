#include "mcv_platform.h"
#include "fsm_boss.h"
#include "../components/all_components.h"
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

	AddState("fbp_Hidden", (statehandler)&fsm_boss::Hidden);
	AddState("fbp_RiseUp", (statehandler)&fsm_boss::RiseUp);
	AddState("fbp_Idle1", (statehandler)&fsm_boss::Idle1);
	AddState("fbp_Stunned1", (statehandler)&fsm_boss::Stunned1);
	AddState("fbp_Rain1", (statehandler)&fsm_boss::Rain1);
	AddState("fbp_Ball1", (statehandler)&fsm_boss::Ball1);
	AddState("fbp_Shoot1DownDef", (statehandler)&fsm_boss::Shoot1DownDef);
	AddState("fbp_Shoot1Shoot", (statehandler)&fsm_boss::Shoot1Shoot);
	AddState("fbp_Shoot1ReleaseDef", (statehandler)&fsm_boss::Shoot1ReleaseDef);
	AddState("fbp_Damaged1l", (statehandler)&fsm_boss::Damaged1);
	AddState("fbp_Idle2", (statehandler)&fsm_boss::Idle2);
	AddState("fbp_Stunned2", (statehandler)&fsm_boss::Stunned2);
	AddState("fbp_Wave", (statehandler)&fsm_boss::Wave);
	AddState("fbp_Rain2", (statehandler)&fsm_boss::Rain2);
	AddState("fbp_Ball2", (statehandler)&fsm_boss::Ball2);
	AddState("fbp_Shoot2", (statehandler)&fsm_boss::Shoot2);
	AddState("fbp_Damaged2", (statehandler)&fsm_boss::Damaged2);
	AddState("fbp_FinalState", (statehandler)&fsm_boss::FinalState);
	AddState("fbp_Dead", (statehandler)&fsm_boss::Dead);

	// reset the state
	ChangeState("fbp_Hidden");

	state_time = 0.f;

	// Init vars
	point_offset = PxVec3(0, -6, 0);
	distance_to_point = 12;
	
	m_entity_manager = &CEntityManager::get();

	m_player = m_entity_manager->getByName("Player");

	obj_distance = 0;
	obj_selected = CHandle();

	force = 24;
}
void fsm_boss::Hidden(){
	int i = 0;
	ChangeState("fbp_RiseUp");
}

void fsm_boss::RiseUp(){
	int i = 0;
	ChangeState("fbp_Idle1");
}

void fsm_boss::Idle1(){
	
	CIOStatus& io = CIOStatus::get();
	// Update input

	if (CIOStatus::get().becomesPressed(CIOStatus::P)){
		ChangeState("fbp_Ball1");
	}
	// Girar cosas a la derecha
	if (CIOStatus::get().becomesPressed(CIOStatus::O)){
		ChangeState("fbp_Shoot1DownDef");
	}
	// Girar cosas a la izquierda
	if (CIOStatus::get().becomesPressed(CIOStatus::I)){
		ChangeState("fbp_Rain1");
	}
	// Girar cosas a la izquierda
	if (CIOStatus::get().becomesPressed(CIOStatus::U)){
	}
	// Girar cosas a la izquierda
	if (CIOStatus::get().becomesPressed(CIOStatus::L)){
	}
}

void fsm_boss::Stunned1(){
	int i = 0;
	ChangeState("fbp_Idle1");
}

void fsm_boss::Rain1(){
	int i = 0;
	ChangeState("fbp_Idle1");
}

void fsm_boss::Ball1(){

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
		ChangeState("fbp_Idle1");
	}
}

//Shoot1
void fsm_boss::Shoot1DownDef(){

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

	ChangeState("fbp_Shoot1Shoot");
}

//Shoot
void fsm_boss::Shoot1Shoot(){
	
	// Init vars
	if (on_enter){
//		obj_distance = float
	}

	// Take and shoot loop
	for (int i = 0; i < m_entity_manager->rigid_list.size(); ++i){
		CEntity* e = m_entity_manager->rigid_list[i];
		TCompRigidBody* rigid = e->get<TCompRigidBody>();

		bool bossAccess = rigid->boss_level == 0;
		if (bossAccess){
			PxRigidBody*  px_rigid = rigid->rigidBody;
			PxVec3 force_dir = (PxVec3(0, -1, 0));
			px_rigid->addForce(force_dir * force, PxForceMode::eACCELERATION, true);
			px_rigid->setMass(10000000);
		}

	}
	// take obj
	// check distance
	// shoot

	// Leave protections

	// Set Down protections
	if (state_time >= 3){
		ChangeState("fbp_Shoot1ReleaseDef");
	}
}

//Shoot1ReleaseDef
void fsm_boss::Shoot1ReleaseDef(){

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
	ChangeState("fbp_Idle1");
}

//Damaged1
void fsm_boss::Damaged1(){
	ChangeState("fbp_Idle1");
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

//Dead
void fsm_boss::Dead(){
	int i = 0;
}