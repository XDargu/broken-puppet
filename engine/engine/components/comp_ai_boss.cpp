#include "mcv_platform.h"
#include "comp_ai_boss.h"
#include "entity_manager.h"
#include "comp_rigid_body.h"
#include "comp_transform.h"
#include "io\iostatus.h"
#include "handle\prefabs_manager.h"

void TCompAiBoss::init(){

	m_fsm_boss.Init();
	
	mPlayer = CEntityManager::get().getByName("Player");
	mBoss = CEntityManager::get().getByName("BossBody");
	point_offset = PxVec3(0, -6, 0);
	distance_to_point = 12;

	activate = false;
	shoot = false;
	attack2Active = false;
	attack2Time = 0;

	move_things = 0;
	debris_creation_delay = 0;
	debris_created = 0;

	force = 24;


}

void TCompAiBoss::update(float elapsed){

	m_fsm_boss.update(elapsed);

	CIOStatus& io = CIOStatus::get();
	// Update input

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
}

void TCompAiBoss::fixedUpdate(float elapsed){

	// Calculate the point to go
	TCompTransform* enemy_comp_trans = ((CEntity*)mBoss)->get<TCompTransform>();
	
	// Get the enemy position
	PxVec3 enemy_pos =  Physics.XMVECTORToPxVec3(enemy_comp_trans->position);
	
	// Get the player position
	CEntityManager& entity_manager = CEntityManager::get();
	TCompTransform* player_comp_trans = (((CEntity*)mPlayer)->get<TCompTransform>());
	PxVec3 player_pos = Physics.XMVECTORToPxVec3(player_comp_trans->position);

	// Calculate direction player enemy
	PxVec3 player_boss_dir = (player_pos - enemy_pos);
	player_boss_dir.y = 0;
	player_boss_dir = player_boss_dir.getNormalized();

	// Add offset to the enemy position
	// Set point to go
		point_to_go = enemy_pos + point_offset + (player_boss_dir * distance_to_point);

	if (isKeyPressed('P')){
		activate = true;
	}
	if (isKeyPressed('U'))
	{
		attack2Active = true;		
		attack2Time = 0;
	}
	if (shoot){
		activate = false;
		shoot = false;
		// apply force the player
		

		for (int i = 0; i < entity_manager.rigid_list.size(); ++i){
			CEntity* e = entity_manager.rigid_list[i];			

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

	if (activate){
		// apply to the point
		CEntityManager& entity_manager = CEntityManager::get();
		
		for (int i = 0; i < entity_manager.rigid_list.size(); ++i){
			CEntity* e = entity_manager.rigid_list[i];
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

	// Fallinf debris
	if (move_things == 3){

		// Cargar un prefab
		int debris_amount= 20;
		float debris_respawn_time = 0.2f;
		
		if (debris_created <= debris_amount){

			debris_creation_delay += elapsed;

			if (debris_creation_delay >= debris_respawn_time){
				debris_creation_delay = 0;

				int rnd = getRandomNumber(1, 3);
				std::string name = "boss/debris_0" + std::to_string(rnd);
				CEntity* prefab_entity = prefabs_manager.getInstanceByName(name.c_str());

				XMVECTOR random_point = getRandomVector3(-20, 15, -20, 20, 16, 20);
				TCompTransform* prefab_t = prefab_entity->get<TCompTransform>();
				prefab_t->init();
				prefab_t->teleport(random_point);

				debris_created++;
			}
		}
		else {
			debris_created = 0;
			move_things = 0;
			debris_creation_delay = 0;
		}
	}

	// Move thing to the left
	if (move_things == 1){
		
		// Get the up vector
		PxVec3 m_up = PxVec3(0, 1, 0);

		// Calculate the direction
		
		//PxVec3 m_force = player_boss_dir.cross(m_up);

		for (int i = 0; i < entity_manager.rigid_list.size(); ++i){
			CEntity* e = entity_manager.rigid_list[i];
			if (!e->hasTag("player")){
				TCompRigidBody* rigid = e->get<TCompRigidBody>();
				PxRigidBody*  px_rigid = rigid->rigidBody;
				PxVec3 obj_boss_dir = px_rigid->getGlobalPose().p - enemy_pos;
				PxVec3 m_force = (obj_boss_dir.cross(m_up).getNormalized()) * force;
				PxVec3 force_dir = (point_to_go - px_rigid->getGlobalPose().p).getNormalized();
				m_force += force_dir;

				px_rigid->addForce(m_force, PxForceMode::eVELOCITY_CHANGE, true);
			}
		}

		// Add atraction force to the boss
		move_things = 0;
	}

	// Move thing to the left
	if (move_things == 2){

		// Get the up vector
		PxVec3 m_up = PxVec3(0, 1, 0);

		// Calculate the direction

		//PxVec3 m_force = player_boss_dir.cross(m_up);

		for (int i = 0; i < entity_manager.rigid_list.size(); ++i){
			CEntity* e = entity_manager.rigid_list[i];
			if (!e->hasTag("player")){
				TCompRigidBody* rigid = e->get<TCompRigidBody>();
				PxRigidBody*  px_rigid = rigid->rigidBody;
				PxVec3 obj_boss_dir = px_rigid->getGlobalPose().p - enemy_pos;

				PxVec3 m_force = (m_up.cross(obj_boss_dir).getNormalized()) * force;
				PxVec3 force_dir = (point_to_go - px_rigid->getGlobalPose().p).getNormalized();
				m_force += force_dir;

				px_rigid->addForce(m_force, PxForceMode::eVELOCITY_CHANGE, true);
			}
		}

		// Add atraction force to the boss
		move_things = 0;
	}

	// Getting down Covers
	if (attack2Active){

		for (int i = 0; i < entity_manager.rigid_list.size(); ++i){
			CEntity* e = entity_manager.rigid_list[i];
			TCompRigidBody* rigid = e->get<TCompRigidBody>();
			bool bossAccess = rigid->boss_level == 1;
			if (bossAccess){
				PxRigidBody*  px_rigid = rigid->rigidBody;
				PxVec3 force_dir = (PxVec3(0, -1, 0));
				px_rigid->addForce(force_dir * force * 3, PxForceMode::eACCELERATION, true);
			}
	
		}
		

		attack2Time += elapsed;
		if (attack2Time > 5) {
			attack2Active = false;
			attack2Time = 0;
		}
	}

}