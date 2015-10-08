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
	/**
	0: Rain
	1: Shoot
	2: Ball
	3: WaveRight
	4: WaveLeft
	/**/
	
	TPattern p1;
	p1.attack = 1; // Shoot
	p1.wait_time = 3.f;

	TPattern p2;
	p2.attack = 2; // Ball
	p2.wait_time = 3.f;

	TPattern p3;
	p3.attack = 3; // WaveRight
	p3.wait_time = 3.f;

	TPattern p4;
	p4.attack = 4; // WaveLeft
	p4.wait_time = 3.f;

	TPattern p5;
	p5.attack = 5; // Random
	p5.wait_time = 3.f;

	// Basic "El de toda la vida"
	TAttackPattern ap1;
	ap1.pattern.push_back(p2);
	ap1.pattern.push_back(p1);
	ap1.pattern.push_back(p3);
	ap1.pattern.push_back(p4);

	// SemiBasic "Busca refugio madafaca"
	TAttackPattern ap2;
	ap2.pattern.push_back(p1);
	ap2.pattern.push_back(p2);
	ap2.pattern.push_back(p3);

	// Fully ground "El meneito"
	TAttackPattern ap3;
	ap3.pattern.push_back(p4);
	ap3.pattern.push_back(p2);
	ap3.pattern.push_back(p1);
	ap3.pattern.push_back(p3);

	// Big Bang "Correeee, que noooo"
	TAttackPattern ap4;
	ap4.pattern.push_back(p2);
	ap4.pattern.push_back(p1);
	ap4.pattern.push_back(p2);

	// Add all ap to list
	attack_pattern_list.push_back(ap1);
	attack_pattern_list.push_back(ap2);
	attack_pattern_list.push_back(ap3);
	attack_pattern_list.push_back(ap4);
	/**/
							  
	boss_out = false;
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
	reorientate_angle = deg2rad(10.f);
	reorientate_angle = cosf(reorientate_angle);
	no_reorientate_angle = deg2rad(5.f);
	no_reorientate_angle = cosf(no_reorientate_angle);
	max_shoot_angle = deg2rad(100.f);
	max_shoot_angle = cosf(max_shoot_angle);

	state_time = 0.f;
	has_left = true;
	has_right = true;

	hurt_state = 0;
	shoots_amount = 0;

	last_attack = 0.f;

	debris_created = 0;
	debris_creation_delay = 0;
	bomb_creation_delay = 0;
	r_hand_change = false;
	l_hand_change = false;

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

	bombs_destroyed = false;
	need_bombs = false;

	pattern_it = 0;
	pattern_current = -1;
	last_attack_it = -1;

	times_stunned = 0;

	original_pos = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->position;
	
	appear = false;
	lua_boss_init = false;
}

void fsm_boss::Hidden(float elapsed){
	Reorientate(elapsed,false);

	if (on_enter){
		XMVECTOR aux_pos = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->position;
		((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->position = XMVectorSetY(aux_pos, -500);
	}

	if (CIOStatus::get().becomesPressed(CIOStatus::V) || lua_boss_init){
		// Emitir particula
		XMVECTOR aux_rot = XMQuaternionRotationAxis(XMVectorSet(1, 0, 0, 0), deg2rad(-90));
		
		XMVECTOR aux_pos = XMVectorSet(0, 1, 0, 0);

		CHandle tapa_suelo = CEntityManager::get().getByName("tapa_hueco_boss");
		if (tapa_suelo.isValid()){
			TCompTransform* tapa_suelo_trans = ((CEntity*)tapa_suelo)->get<TCompTransform>();
			if (tapa_suelo_trans)
				aux_pos = tapa_suelo_trans->position;
		}

		aux_pos = XMVectorSet(0, 1, 0, 0);
		CHandle particle_entity = CLogicManager::get().instantiateParticleGroup("ps_boss_entry", aux_pos, aux_rot);
		appear = true;
		state_time = 0;
		lua_boss_init = false;
	}

	if (appear && (state_time >= 1.f)){		
		ChangeState("fbp_RiseUp");	
	}
	
}

void fsm_boss::RiseUp(){
	if (on_enter){
		TCompTransform* trans = ((CEntity*)entity)->get<TCompTransform>();
		TCompSkeleton* skeleton = comp_skeleton;

		// Liberar defensas
		Release_def();
		
		// Play rise up animation
		stopAllAnimations();
		skeleton->playAnimation(35);

		// Look at the player
		TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
		skeleton_lookat->active = false;

		// Move to the initial position
		trans->position = original_pos;
		
		/**/
		CHandle puente = CEntityManager::get().getByName("pasarela_boss");
		if (puente.isValid()){
			TCompTransform* puente_trans = ((CEntity*)puente)->get<TCompTransform>();
			if (puente_trans){
				puente_trans->teleport(XMVectorSet(1000, 1000, 1000, 1000));
			}
		}
		

		/**/
	}

	if ((state_time >= 6.79f) && (!boss_out)){
		boss_out = true;
		// Make the floor disapear
		CHandle floor = m_entity_manager->getByName("tapa_hueco_boss");
		if (floor.isValid()){
			TCompTransform* floor_trans = ((CEntity*)floor)->get<TCompTransform>();

			// Generate the broken floor prefab
			CHandle broken_floor = prefabs_manager.getInstanceByName("boss/tapa_boss_rota");
			//CHandle broken_floor = prefabs_manager.getInstanceByName("boss/tapa_boss_rota_2");
			/**/
			if (broken_floor.isValid() && (floor_trans)){
				TCompTransform* broken_floor_trans = ((CEntity*)broken_floor)->get<TCompTransform>();

				if (broken_floor_trans){
					broken_floor_trans->setType(0);
					broken_floor_trans->position = floor_trans->position;

					TCompRender* floor_render = ((CEntity*)floor)->get<TCompRender>();
					if (floor_render)
						floor_render->active = false;
				}
			}
			/**/
		}
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
		int attack = CalculateAttack();
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
		times_stunned++;
	}

	// Hacer danio a la izquierda
	if (CIOStatus::get().becomesPressed(CIOStatus::V)){
		EvaluateHit(0);
	}
	// Hacer danio a la derecha
	if (CIOStatus::get().becomesPressed(CIOStatus::B)){
		EvaluateHit(1);
	}

	if ((state_time >= 2) && (times_stunned <= 1)){
		ChangeState("fbp_Recover");
	}
	else if ((state_time >= 6) && (times_stunned <= 2)){
		ChangeState("fbp_Recover");
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
		
		/**
		r_hand_pos_y = XMVectorGetY(skeleton->getPositionOfBone(40));
		l_hand_pos_y = XMVectorGetY(skeleton->getPositionOfBone(21));
		/**/

		FistParticles(elapsed);
	}
	FistParticles(elapsed);
	


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
		need_bombs = false;
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

		
		FistParticles(elapsed);
	}

	FistParticles(elapsed);

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
		ball_lauched = false;
	}

	// Getting player position
	TCompTransform* player_comp_trans = (((CEntity*)m_player)->get<TCompTransform>());
	PxVec3 player_pos = Physics.XMVECTORToPxVec3(player_comp_trans->position);
	
	// Getting enemy position
	TCompTransform* enemy_comp_trans = ((CEntity*)entity)->get<TCompTransform>();
	PxVec3 enemy_pos = Physics.XMVECTORToPxVec3(enemy_comp_trans->position);

	// Calculate direction player enemy
	PxVec3 player_boss_dir = (player_pos - enemy_pos);
	player_boss_dir.y = 0;
	player_boss_dir = player_boss_dir.getNormalized();

	PxVec3	point_to_go = enemy_pos + point_offset + (player_boss_dir * distance_to_point);

	if (state_time < 1.3){
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
	else if(!ball_lauched) {
		ball_lauched = true;

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

void fsm_boss::Shoot1Reload(float elapsed){

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

	if (has_left && has_right){

		TCompTransform* player_comp_trans = (((CEntity*)m_player)->get<TCompTransform>());
		TCompTransform* enemy_comp_trans = ((CEntity*)entity)->get<TCompTransform>();

		// Get the enemy position
		PxVec3 enemy_pos = Physics.XMVECTORToPxVec3(enemy_comp_trans->position);
		PxVec3 player_pos = Physics.XMVECTORToPxVec3(player_comp_trans->position);

		// Calculate direction player enemy
		PxVec3 player_boss_dir = (enemy_pos - player_pos);

		Reorientate(0.f, true);
		float angle = getAngleBetweenVectors(enemy_comp_trans->getFront(), Physics.PxVec3ToXMVECTOR(player_boss_dir));
		if ((angle >= max_shoot_angle)){
			ChangeState("fbp_Shoot1ReleaseDef");
		}
	}
	else{
		Reorientate(elapsed, false);
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

			if ((state_time >= 4.f)&&(dist<3)){
				ChangeState("fbp_Shoot1Shoot");
			}
			else if (state_time >= 5.f) {
				state_time = 0;
				shoots_amount++;
				SelectObjToShoot();
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
void fsm_boss::Shoot1Shoot(float elapsed){
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
	if ((m_e)&&(state_time < 0.4f)) {
		TCompRigidBody* rigid = m_e->get<TCompRigidBody>();
		if (rigid) {
			PxRigidBody*  px_rigid = rigid->rigidBody;

			//px_rigid->addForce(force_dir, PxForceMode::eVELOCITY_CHANGE, true);
			px_rigid->setLinearVelocity(force_dir * 70);
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
void fsm_boss::WaveLeft(float elapsed){
	Reorientate(0.f, true);
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		skeleton->playAnimation(5);
		last_anim_id = -1;
	}	
	
	// Get boss pos
	TCompTransform* enemy_comp_trans = ((CEntity*)entity)->get<TCompTransform>();
	PxVec3 m_boss_pos = Physics.XMVECTORToPxVec3(enemy_comp_trans->position);

	if ((state_time >= 1.1f) && (state_time < 2.f)){

		for (int i = 0; i < m_entity_manager->rigid_list.size(); ++i){
			CEntity* e = m_entity_manager->rigid_list[i];
			if (!e->hasTag("player")){

				// Get obj to move
				TCompRigidBody* rigid = ((CEntity*)e)->get<TCompRigidBody>();
				if (((CHandle)rigid).isValid()){
					bool bossAccess = rigid->boss_level == 0;
					if (bossAccess){
						// If is in angle
						TCompSkeleton* skeleton = comp_skeleton;

						PxVec3 aux_hand_pos = Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(15));
						XMVECTOR aux_hand_dir = Physics.PxVec3ToXMVECTOR(PxVec3(aux_hand_pos.x, m_boss_pos.y, aux_hand_pos.z) - m_boss_pos);
						aux_hand_dir = XMVector3Normalize(aux_hand_dir);

						XMVECTOR aux_obj_pos = rigid->getPosition();
						aux_obj_pos = XMVectorSetY(aux_obj_pos, m_boss_pos.y);
						XMVECTOR aux_obj_dir = XMVector3Normalize(aux_obj_pos - Physics.PxVec3ToXMVECTOR(m_boss_pos));

						float aux_angle = getAngleBetweenVectors(aux_obj_dir, aux_hand_dir);

						if (aux_angle >= 0.95f)
						{
							PxRigidBody*  px_rigid = rigid->rigidBody;
							PxVec3 obj_boss_dir = px_rigid->getGlobalPose().p - m_boss_pos;
							PxVec3 m_force = (PxVec3(0, 1, 0).cross(obj_boss_dir).getNormalized());
							px_rigid->setLinearVelocity(-m_force * 40);
						}
					}
				}
			}
		}
	}

	if (state_time >= 2.3f){

		TCompTransform* enemy_comp_trans = ((CEntity*)entity)->get<TCompTransform>();
		PxVec3 m_boss_pos = Physics.XMVECTORToPxVec3(enemy_comp_trans->position);

		for (int i = 0; i < ball_list.size(); ++i){
			CEntity* e = ball_list[i];
			if (((CHandle)e).isValid()){
				TCompRigidBody* rigid = e->get<TCompRigidBody>();
				if (((CHandle)rigid).isValid()){
					PxRigidBody*  px_rigid = rigid->rigidBody;
					px_rigid->setLinearVelocity(px_rigid->getLinearVelocity() * 0.2f);
				}
			}
		}

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

	if ((state_time >= 1.1f) && (state_time < 2.f)){
		// Get boss pos
		TCompTransform* enemy_comp_trans = ((CEntity*)entity)->get<TCompTransform>();
		PxVec3 m_boss_pos = Physics.XMVECTORToPxVec3(enemy_comp_trans->position);

		for (int i = 0; i < m_entity_manager->rigid_list.size(); ++i){
			CEntity* e = m_entity_manager->rigid_list[i];
			if (!e->hasTag("player")){
				
				// Get obj to move
								
				TCompRigidBody* rigid = ((CEntity*)e)->get<TCompRigidBody>();
				if (((CHandle)rigid).isValid()){
					bool bossAccess = rigid->boss_level == 0;
					if (bossAccess){
						// If is in angle
						TCompSkeleton* skeleton = comp_skeleton;

						PxVec3 aux_hand_pos = Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(40));
						XMVECTOR aux_hand_dir = Physics.PxVec3ToXMVECTOR(PxVec3(aux_hand_pos.x, m_boss_pos.y, aux_hand_pos.z) - m_boss_pos);
						aux_hand_dir = XMVector3Normalize(aux_hand_dir);

						XMVECTOR aux_obj_pos = rigid->getPosition();
						aux_obj_pos = XMVectorSetY(aux_obj_pos, m_boss_pos.y);
						XMVECTOR aux_obj_dir = XMVector3Normalize(aux_obj_pos - Physics.PxVec3ToXMVECTOR(m_boss_pos));

						float aux_angle = getAngleBetweenVectors(aux_obj_dir, aux_hand_dir);					

						if (aux_angle >= 0.95f)
						{
							PxRigidBody*  px_rigid = rigid->rigidBody;
							PxVec3 obj_boss_dir = px_rigid->getGlobalPose().p - m_boss_pos;
							PxVec3 m_force = (obj_boss_dir.cross(PxVec3(0, 1, 0)).getNormalized());
							px_rigid->setLinearVelocity(-m_force * 40);
						}
					}
				}
			}
		}
	}


	if (state_time >= 2.3f){

		TCompTransform* enemy_comp_trans = ((CEntity*)entity)->get<TCompTransform>();
		PxVec3 m_boss_pos = Physics.XMVECTORToPxVec3(enemy_comp_trans->position);

		for (int i = 0; i < ball_list.size(); ++i){
			CEntity* e = ball_list[i];
			if (((CHandle)e).isValid()){
				TCompRigidBody* rigid = e->get<TCompRigidBody>();
				if (((CHandle)rigid).isValid()){
					PxRigidBody*  px_rigid = rigid->rigidBody;
					px_rigid->setLinearVelocity(px_rigid->getLinearVelocity() * 0.2f);
				}
			}
		}
		ChangeState("fbp_Idle1");
	}
}


//FinalState
void fsm_boss::FinalState(float elapsed){
	if (on_enter){
		TCompSkeleton* skeleton = comp_skeleton;
		stopAllAnimations();
		loopAnimationIfNotPlaying(26, true);
		TCompSkeletonLookAt* skeleton_lookat = comp_skeleton_lookat;
		skeleton_lookat->active = false;

		for (int i = 0; i < m_entity_manager->rigid_list.size(); ++i){
			CEntity* e = m_entity_manager->rigid_list[i];
			if (!e->hasTag("player")){
				TCompRigidBody* rigid = ((CEntity*)e)->get<TCompRigidBody>();
				if (((CHandle)rigid).isValid()){
					bool bossAccess = rigid->boss_level == 0;
					if (bossAccess){

						ball_list.push_back(e);
					}
				}

			}
		}
	}

	TCompTransform* enemy_comp_trans = ((CEntity*)entity)->get<TCompTransform>();
	PxVec3 m_boss_pos = Physics.XMVECTORToPxVec3(enemy_comp_trans->position);

	for (int i = 0; i < ball_list.size(); ++i){
		CEntity* e = ball_list[i];
		if (((CHandle)e).isValid()){
			TCompRigidBody* rigid = e->get<TCompRigidBody>();
			if (((CHandle)rigid).isValid()){
				PxRigidBody*  px_rigid = rigid->rigidBody;
				PxVec3 obj_boss_dir = px_rigid->getGlobalPose().p - m_boss_pos;
				PxVec3 aux_up = PxVec3(0, 1, 0);
				PxVec3 m_force = (obj_boss_dir.cross(PxVec3(0, 1, 0)).getNormalized());
				m_force = (m_force + (aux_up * 1)).getNormalized();
				px_rigid->addForce(m_force * 40.f * elapsed, PxForceMode::eVELOCITY_CHANGE, true);				
			}
		}
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
		times_stunned = 1;
		pattern_current = -1;
	}
	if (state_time >= 1.f){
		Reorientate(elapsed, false);		
	}
	if (state_time >= 2.f){
		destroyBombs();
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
		times_stunned = 1;
		pattern_current = -1;
	}
	if (state_time >= 1.f){
		Reorientate(elapsed, false);
	}
	if (state_time >= 2.f){
		destroyBombs();
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
		times_stunned = 1;
		pattern_current = -1;
	}

	if (state_time >= 0.5f){
		destroyBombs();
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

		times_stunned = 1;
		pattern_current = -1;
	}
	if (state_time >= 0.5f){
		destroyBombs();
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
		CApp::get().playFinalVideo();
		TCompRagdoll* ragdoll = comp_ragdoll;
		ragdoll->enableBoneTree(4);
		ragdoll->enableBoneTree(36);
		ragdoll->enableBoneTree(11);
		// Cambiar a video
		//ChangeState("fbp_Idle1");
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

int fsm_boss::CalculateAttack() {

	int next_attack = 0;
	last_attack = 0.f;
	if (m_entity_manager->rigid_list.size() < 130){
		next_attack = 0;
	}
	else{
		//return 2;
		/**/
		// Comprobamos si el patron es nulo, si lo es elegimos uno aleatorio
		if (pattern_current < 0){
			pattern_current = getRandomNumber(0, (attack_pattern_list.size() - 1));
			pattern_it = 0;
		}
		// Comprobamos que no hemos acabado el patron
		int aux = attack_pattern_list[pattern_current].pattern.size();
		if (aux > (pattern_it)){
			next_attack = attack_pattern_list[pattern_current].pattern[pattern_it].attack;
			pattern_it++;
			// Avoid imposible attack
			if (((next_attack == 3) && (!has_right)) || ((next_attack == 4) && (!has_left))){
				next_attack = getRandomNumber(1, 2);
			}
		}
		else{
			pattern_current = -1;
			pattern_it = 0;			
			next_attack = getRandomNumber(1,2);
			// Check if needs bombs
			if (bombs_destroyed){
				need_bombs = true;
				bombs_destroyed = false;
			}
			else{
				int aux_bomb_number = 0;
				for (int i = 0; i < m_entity_manager->rigid_list.size(); ++i){
					CEntity* e = m_entity_manager->rigid_list[i];
					if (!e->hasTag("player")){
						TCompExplosion* comp_explo = e->get<TCompExplosion>();
						if (comp_explo){
							aux_bomb_number++;
						}
					}
				}
				need_bombs = aux_bomb_number < 0;							
			}
			
		}
		/**/
	}
	//XDEBUG("devolviendo estado: %d", next_attack);

	ball_list.clear();
	return next_attack;
}

bool fsm_boss::RainDebris(float elapsed){

	// Cargar un prefab
	int debris_amount = 200;
	//int debris_amount = 2000000000;
	float debris_respawn_time = 0.05f;
	float bomb_respawn_time = 1.f;
	bool active = true;

	if (debris_created <= debris_amount){

		debris_creation_delay += elapsed;
		bomb_creation_delay += elapsed;

		if (debris_creation_delay >= debris_respawn_time){
			debris_creation_delay = 0;

			TCompTransform* enemy_comp_trans = ((CEntity*)entity)->get<TCompTransform>();

			XMVECTOR aux_boss_pos = enemy_comp_trans->position;

			XMVECTOR create_position;
			XMVECTOR random_point = getRandomVector3(
				XMVectorGetX(aux_boss_pos) - 17
				, XMVectorGetY(aux_boss_pos) + 60
				, XMVectorGetZ(aux_boss_pos) - 10
				, XMVectorGetX(aux_boss_pos) + 17
				, XMVectorGetY(aux_boss_pos) + 61
				, XMVectorGetZ(aux_boss_pos) + 10);
			
			bool equal =(Physics.XMVECTORToPxVec3(random_point) == Physics.XMVECTORToPxVec3(last_random_pos));

			if (equal){
				// Calculate a new pos
				PxVec3 m_boss_pos = Physics.XMVECTORToPxVec3(enemy_comp_trans->position);

				PxVec3 obj_boss_dir = Physics.XMVECTORToPxVec3(last_created_pos) - m_boss_pos;
				PxVec3 aux_up = PxVec3(0, 0.01f, 0);
				PxVec3 m_force = (obj_boss_dir.cross(PxVec3(0, 1, 0)).getNormalized());
	
				create_position = last_created_pos + Physics.PxVec3ToXMVECTOR(m_force * 6);

			}
			else{
				create_position = random_point;
			}

			last_created_pos = create_position;
			last_random_pos = random_point;

			std::string name = "";

			// Bombs
			if ((bomb_creation_delay >= bomb_respawn_time)){
				bomb_creation_delay = 0;
				name = "boss/bomb";
				
			}
			// Debris
			else{				
				int rnd = getRandomNumber(1, 20);
				name = "boss/debris_0" + std::to_string(rnd);
				
			}

			CEntity* prefab_entity = prefabs_manager.getInstanceByName(name.c_str());

			TCompTransform* prefab_t = prefab_entity->get<TCompTransform>();
			if (prefab_t){
				prefab_t->init();
				prefab_t->teleport(create_position);
			}				

			TCompParticleGroup* prefab_pg = prefab_entity->get<TCompParticleGroup>();
			if (prefab_pg)
				prefab_pg->init();

			TCompExplosion* prefab_E = prefab_entity->get<TCompExplosion>();
			if (prefab_E)
				prefab_E->init();

			TCompBossPrefab* prefab_BP = prefab_entity->get<TCompBossPrefab>();
			if (prefab_BP)
				prefab_BP->init();

			debris_created++;
		}
	}
	else{
		active = false;
	}

	return active;
}


bool fsm_boss::HeadHit() {

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
	return can_hit;
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

				if (!obj_to_shoot.isValid()) { 
					obj_to_shoot = e; 
				}
				
				TCompSkeleton* skeleton = comp_skeleton;
				
				float dist_aux = V3DISTANCE(skeleton->getPositionOfBone(40), obj_comp_trans->position);
				if ((dist_aux <= dist)&&(rigid->getMass() > 1500.f)){
					if (obj_to_shoot.isValid()){
						if (obj_to_shoot != e)
						{
							dist = dist_aux;
							obj_to_shoot = e;
						}							
					}
					else{
						dist = dist_aux;
						obj_to_shoot = e;
					}
				}				
			}
		}
	}
}

void fsm_boss::FistParticles(float elapsed) {

	TCompSkeleton* skeleton = comp_skeleton;

	if (has_right){
		XMVECTOR r_hand_pos = skeleton->getPositionOfBone(40);
		XMVECTOR r_hand_rot = skeleton->getRotationOfBone(40);

		if ((r_hand_change) && (r_hand_pos_y < XMVectorGetY(r_hand_pos)))
		{
			// Adding particle sistem
			/**/

			CHandle particle_entity = CLogicManager::get().instantiateParticleGroup("ps_metal_hit", XMVectorSetY(r_hand_pos, 0), r_hand_rot);

			if (particle_entity.isValid()) {
				TCompParticleGroup* pg = ((CEntity*)particle_entity)->get<TCompParticleGroup>();
				pg->destroy_on_death = true;
				if (pg->particle_systems->size() > 0)
				{
					(*pg->particle_systems)[0].emitter_generation->inner_radius = 1.f;
					(*pg->particle_systems)[0].emitter_generation->radius = 3.f;
				}
			}
			/**/
		}
		// 
		if (r_hand_pos_y > XMVectorGetY(r_hand_pos))
			r_hand_change = true;
		else
			r_hand_change = false;
		r_hand_pos_y = XMVectorGetY(skeleton->getPositionOfBone(40));
	}


	if (has_left){
		XMVECTOR l_hand_pos = skeleton->getPositionOfBone(21);
		XMVECTOR l_hand_rot = skeleton->getRotationOfBone(21);

		if ((l_hand_change) && (l_hand_pos_y < XMVectorGetY(l_hand_pos)))
		{
			// Adding particle sistem
			/**/
			CHandle particle_entity = CLogicManager::get().instantiateParticleGroup("ps_metal_hit", XMVectorSetY(l_hand_pos, 0), l_hand_rot);

			if (particle_entity.isValid()) {
				TCompParticleGroup* pg = ((CEntity*)particle_entity)->get<TCompParticleGroup>();
				pg->destroy_on_death = true;
				if (pg->particle_systems->size() > 0)
				{
					(*pg->particle_systems)[0].emitter_generation->inner_radius = 1.f;
					(*pg->particle_systems)[0].emitter_generation->radius = 3.f;
				}
			}
			/**/
		}
		// 
		if (l_hand_pos_y > XMVectorGetY(l_hand_pos))
			l_hand_change = true;
		else
			l_hand_change = false;

		l_hand_pos_y = XMVectorGetY(skeleton->getPositionOfBone(21));
	}
}

void fsm_boss::destroyBombs() {
	if (!bombs_destroyed){
		for (int i = 0; i < m_entity_manager->rigid_list.size(); ++i){
			CEntity* e = m_entity_manager->rigid_list[i];
			if (!e->hasTag("player")){
				TCompExplosion* comp_explo = e->get<TCompExplosion>();
				if (comp_explo){
					comp_explo->Explote(true);
				}
			}
		}
	}	
	bombs_destroyed = true;
}

void fsm_boss::rainJustBombs(){

	std::vector<XMVECTOR>aux_positions;
	aux_positions.push_back(XMVectorSet( 15, 60,  15, 0));
	aux_positions.push_back(XMVectorSet(-15, 60,  15, 0)); 
	aux_positions.push_back(XMVectorSet( 15, 60, -15, 0));
	aux_positions.push_back(XMVectorSet(-15, 60, -15, 0));

	for (int i = 0; i < aux_positions.size(); i++){
		CEntity* prefab_entity = prefabs_manager.getInstanceByName("boss/bomb");

		TCompTransform* prefab_t = prefab_entity->get<TCompTransform>();
		if (prefab_t){
			prefab_t->init();
			prefab_t->teleport(aux_positions[i]);
		}

		TCompParticleGroup* prefab_pg = prefab_entity->get<TCompParticleGroup>();
		if (prefab_pg)
			prefab_pg->init();

		TCompExplosion* prefab_E = prefab_entity->get<TCompExplosion>();
		if (prefab_E)
			prefab_E->init();
	}

}

