#include "mcv_platform.h"
#include "bt_soldier.h"
#include "../entity_manager.h"
#include "../components/all_components.h"
#include "utils.h"
#include "nav_mesh_manager.h"
#include "components\comp_skeleton.h"
#include "font\font.h"

//Constants
const int max_bf_posibilities = 7;
const float max_dist_close_attack = 1.7f;
const float max_time_player_lost = 2.f;
const float max_distance_to_attack = 2.15f;
const float max_time_player_search = 7.f;
const float max_range_role = 7.f;
const float max_distance_taunter = 4.f;
const float delta_time_close_attack = 2.8f;
const float distance_change_way_point = 0.55f;
const float force_large_impact = 40000.f;
const float force_medium_impact = 25000.f;
const float max_time_ragdoll = 3.f;
const float radius = 7.f;

const float walk_speed = 0.8f;
const float run_speed = 2.f;
const float run_angry_speed = 5.2f;


// Sensor
const float sensor_delay = 1.f;

const string particle_name_dismemberment = "ps_wood_hit";
const string particle_name_initial_hit = "ps_attack2";


void bt_soldier::create(string s)
{
	name = s;
	createRoot("Root", PRIORITY, NULL, NULL);
	addChild("Root", "Ragdoll", SEQUENCE, (btcondition)&bt_soldier::conditionis_ragdoll, NULL);
	addChild("Ragdoll", "ActionRagdoll1", ACTION, INTERNAL, NULL, (btaction)&bt_soldier::actionRagdoll);
	addChild("Ragdoll", "Awake", PRIORITY, NULL, NULL);
	addChild("Awake", "WakeUp", SEQUENCE, INTERNAL, (btcondition)&bt_soldier::conditiontied_event, NULL);
	addChild("Awake", "GroundedTied", PRIORITY, NULL, NULL);
	addChild("GroundedTied", "ActionWakeUp2", ACTION, INTERNAL, (btcondition)&bt_soldier::conditionis_grounded, (btaction)&bt_soldier::actionWakeUp);

	addChild("Awake", "Grounded", PRIORITY, (btcondition)&bt_soldier::conditiontrue, NULL);
	addChild("Grounded", "ActionWakeUp4", ACTION, (btcondition)&bt_soldier::conditionis_grounded, (btaction)&bt_soldier::actionWakeUp);
	addChild("Grounded", "Leave5", ACTION, (btcondition)&bt_soldier::conditiontrue, (btaction)&bt_soldier::actionLeave);

	addChild("Root", "events", PRIORITY, (btcondition)&bt_soldier::conditionare_events, NULL);
	addChild("events", "HurtEvent7", ACTION, EXTERNAL, (btcondition)&bt_soldier::conditionhurt_event, (btaction)&bt_soldier::actionHurtEvent);
	addChild("events", "TiedEvent9", SEQUENCE, (btcondition)&bt_soldier::conditiontied_event, NULL);
	addChild("TiedEvent9", "TiedHit", ACTION, EXTERNAL, NULL, (btaction)&bt_soldier::actionHurtEvent);


	addChild("events", "No events10", ACTION, (btcondition)&bt_soldier::conditiontrue, (btaction)&bt_soldier::actionNoevents);
	addChild("Root", "Angry", PRIORITY, (btcondition)&bt_soldier::conditionis_angry, NULL);


	addChild("Angry", "Warcry11", ACTION, (btcondition)&bt_soldier::conditionhave_to_warcry, (btaction)&bt_soldier::actionWarcry);
	addChild("Angry", "LookForPlayer", PRIORITY, EXTERNAL, (btcondition)&bt_soldier::conditionplayer_lost, NULL);
	addChild("Angry", "PlayerAlert12", ACTION, EXTERNAL, (btcondition)&bt_soldier::conditionsee_player, (btaction)&bt_soldier::actionPlayerAlert);

	addChild("LookForPlayer", "LookAroundPriority", PRIORITY, EXTERNAL, (btcondition)&bt_soldier::conditiontrue, NULL);
	addChild("LookAroundPriority", "LookAroundSequence", SEQUENCE, EXTERNAL, (btcondition)&bt_soldier::conditionLook_time, NULL);
	addChild("LookAroundPriority", "CalmDown13", ACTION, EXTERNAL, (btcondition)&bt_soldier::conditiontrue, (btaction)&bt_soldier::actionCalmDown);
	addChild("LookAroundSequence", "SearchLastPoint", ACTION, EXTERNAL, NULL, (btaction)&bt_soldier::actionSearchArroundLastPoint);
	addChild("LookAroundSequence", "LookAround14", ACTION, EXTERNAL, NULL, (btaction)&bt_soldier::actionLookAround);
	addChild("LookAroundSequence", "LookingForPlayer", ACTION, EXTERNAL, NULL, (btaction)&bt_soldier::actionLookingFor);

	addChild("Angry", "TryAttack", SEQUENCE, EXTERNAL, (btcondition)&bt_soldier::conditiontrue, NULL);
	addChild("TryAttack", "SelectRole15", ACTION, EXTERNAL, NULL, (btaction)&bt_soldier::actionSelectRole);
	addChild("TryAttack", "ExecuteRole", PRIORITY, EXTERNAL, NULL, NULL);

	addChild("ExecuteRole", "AttackRoutine", PRIORITY, EXTERNAL, (btcondition)&bt_soldier::conditionis_attacker, NULL);
	addChild("AttackRoutine", "InitialAttackSeq", SEQUENCE, EXTERNAL, (btcondition)&bt_soldier::conditioninitial_attack, NULL);
	addChild("InitialAttackSeq", "InitialAttack16", ACTION, INTERNAL, NULL, (btaction)&bt_soldier::actionInitialAttack);
	addChild("InitialAttackSeq", "InitialAttack17", ACTION, EXTERNAL, NULL, (btaction)&bt_soldier::actionIdleWarDelay);

	addChild("AttackRoutine", "NormalAttack17", ACTION, INTERNAL, (btcondition)&bt_soldier::conditionnormal_attack, (btaction)&bt_soldier::actionNormalAttack);
	addChild("AttackRoutine", "Situate18", ACTION, EXTERNAL, (btcondition)&bt_soldier::conditionfar_from_target_pos, (btaction)&bt_soldier::actionSituate);
	addChild("AttackRoutine", "IdleWa19r", ACTION, EXTERNAL, (btcondition)&bt_soldier::conditiontrue, (btaction)&bt_soldier::actionIdleWar);
	addChild("ExecuteRole", "Taunter", PRIORITY, EXTERNAL, (btcondition)&bt_soldier::conditionis_taunter, NULL);
	addChild("Taunter", "Situate20", ACTION, EXTERNAL, (btcondition)&bt_soldier::conditionfar_from_target_pos, (btaction)&bt_soldier::actionSituate);
	addChild("Taunter", "Taunter21", ACTION, EXTERNAL, NULL, (btaction)&bt_soldier::actionTaunter);

	addChild("ExecuteRole", "ChaseRoleDistance22", ACTION, EXTERNAL, (btcondition)&bt_soldier::conditiontrue, (btaction)&bt_soldier::actionChaseRoleDistance);

	addChild("Root", "Peacefull", PRIORITY, EXTERNAL, (btcondition)&bt_soldier::conditiontrue, NULL);


	addChild("Peacefull", "FreeTime", RANDOM, (btcondition)&bt_soldier::conditiontrue, NULL);
	addChild("FreeTime", "Idle29", ACTION, EXTERNAL, NULL, (btaction)&bt_soldier::actionIdle, 70);
	addChild("FreeTime", "Wander30", SEQUENCE, EXTERNAL, NULL, NULL, 30);

	addChild("Wander30", "SearchPoint", ACTION, EXTERNAL, NULL, (btaction)&bt_soldier::actionSearchPoint);
	addChild("Wander30", "ActionWander", ACTION, EXTERNAL, NULL, (btaction)&bt_soldier::actionWander);


	last_anim_id = -1;
	ind_path = 0;
	own_transform = ((CEntity*)entity)->get<TCompTransform>();
	center = ((TCompTransform*)own_transform)->position;
	character_controller = ((CEntity*)entity)->get<TCompCharacterController>();
	enemy_skeleton = ((CEntity*)entity)->get<TCompSkeleton>();
	enemy_ragdoll = ((CEntity*)entity)->get<TCompRagdoll>();
	enemy_rigid = ((CEntity*)entity)->get<TCompRigidBody>();
	last_time_player_saw = 0;
	last_time = delta_time_close_attack;
	mov_direction = PxVec3(0, 0, 0);
	look_direction = PxVec3(0, 0, 0);
	player = CEntityManager::get().getByName("Player");
	tied_event = false;
	event_detected = false;
	tied_succesfull = false;
	too_close_attack = false;
	is_angry = false;
	have_to_warcry = false;
	is_ragdoll = false;
	hurt_event = false;
	player_viewed_sensor = false;
	player_previously_lost = false;
	initial_attack = false;
	see_player = false;
	animation_done = false;
	active = false;
	attacked = false;
	lost_player = false;
	needle_hit = false;

	player_out_navMesh = false;

	ropeRef = CHandle();
	player_detected_pos = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	previous_point_search = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	slot_position = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	player_pos_sensor = ((CEntity*)entity)->get<TCompPlayerPosSensor>();
	tied_sensor = ((CEntity*)entity)->get<TCompSensorTied>();
	player_transform = ((CEntity*)player)->get<TCompTransform>();
	rol = role::UNASIGNATED;

	((TCompCharacterController*)character_controller)->lerpRotation = 0.23f;

	resetBot();
}

//Se mantiene en modo ragdoll durante un tiempo
int bt_soldier::actionRagdoll()
{
	TCompRagdoll* m_ragdoll = enemy_ragdoll;
	TCompLife* m_life = ((CEntity*)entity)->get<TCompLife>();

	if (on_enter) {
		stopAllAnimations();


		if (m_ragdoll) {

			if (!m_ragdoll->isRagdollActive()) {
				m_ragdoll->setActive(true);

				if (m_life->life <= 0) {
					TCompRagdoll* m_ragdoll = enemy_ragdoll;
					m_ragdoll->breakJoints();
					TCompTransform* m_transform = own_transform;
					TCompAABB* ragdoll_aabb = (TCompAABB*)((CEntity*)entity)->get<TCompAABB>();
					//XMVECTOR min = XMVectorSet(-50, -50, -50, 0);
					//XMVECTOR max = XMVectorSet(50, 50, 50, 0);

					
					if (this->getRol() == role::ATTACKER)
						aimanager::get().RemoveEnemyAttacker(this);
					else
						aimanager::get().RemoveEnemyTaunt(this);

					CNav_mesh_manager::get().removeCapsule(((CEntity*)entity)->get<TCompColliderCapsule>());
					CEntityManager::get().remove(((CEntity*)entity)->get<TCompRigidBody>());
					CEntityManager::get().remove(((CEntity*)entity)->get<TCompColliderCapsule>());
					
					CEntityManager::get().remove(((CEntity*)entity)->get<TCompCharacterController>());
					//ragdoll_aabb->setIdentityMinMax(min, max);

					aimanager::get().removeBot(this->getId());

					CEntityManager::get().remove(((CEntity*)entity)->get<TCompBtSoldier>());

					TCompTransform* p_transform = player_transform;
					if (V3DISTANCE(p_transform->position, m_transform->position) < 10) {
						CEntity* camera = CEntityManager::get().getByName("PlayerCamera");
						TCompTransform* c_transform = camera->get<TCompTransform>();
						TCompCamera* c_camera = camera->get<TCompCamera>();
						if (c_transform->isInFov(m_transform->position, c_camera->getFov())) {
							CApp::get().slowMotion(1.5f);
						}
					}

				}

			}
		}
	}

	if (m_life->life > 0) {
		XMVECTOR spine_pos = ((TCompSkeleton*)enemy_skeleton)->getPositionOfBone(17);

		XMVECTOR pos_orig = Physics.PxVec3ToXMVECTOR(((TCompRigidBody*)enemy_rigid)->rigidBody->getGlobalPose().p);
		XMVECTOR pos_final = XMVectorLerp(pos_orig, spine_pos, 0.1f);

		if (m_life->life <= 0) {
			pos_final = XMVectorSet(10000, 10000, 10000, 0);
		}

		((TCompRigidBody*)enemy_rigid)->rigidBody->setGlobalPose(
			physx::PxTransform(
			Physics.XMVECTORToPxVec3(pos_final),
			((TCompRigidBody*)enemy_rigid)->rigidBody->getGlobalPose().q
			)
			);
	}

	if (state_time < max_time_ragdoll){
		return STAY;
	}
	else{
		return LEAVE;
	}
}

//Ejecuta la animacin de levantarse
int bt_soldier::actionWakeUp()
{
	if (on_enter) {
		is_ragdoll = false;
		TCompRagdoll* m_ragdoll = enemy_ragdoll;
		TCompSkeleton* m_skeleton = enemy_skeleton;
		m_ragdoll->setActive(false);
		m_skeleton->playAnimation(20);

	}
	stopMovement();
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = last_look_direction;

	if (state_time > 3.9f) {
		playAnimationIfNotPlaying(0);
		return LEAVE;
	}
	else
		return STAY;

}

//Corta todas las cuerdas a la que est atada
int bt_soldier::actionCutOwn()
{
	return LEAVE;
}

//
int bt_soldier::actionLeave()
{
	return LEAVE;
}

//Attack to the player when he is too close
int bt_soldier::actionTooCloseAttack()
{
	if (on_enter) {
		TCompTransform* p_transform = player_transform;
		TCompTransform* m_transform = own_transform;

		//Play close attack animation 
		if (m_transform->isInLeft(p_transform->position))
			playAnimationIfNotPlaying(2);
		else
			playAnimationIfNotPlaying(3);

	}
	stopMovement();
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = last_look_direction;

	if (state_time >= getAnimationDuration(7)) {
		((CEntity*)player)->sendMsg(TActorHit(((CEntity*)player), 61000.f, false));
		return LEAVE;
	}
	else
	{
		return STAY;
	}

}

//Select the idle and play it
int bt_soldier::actionIdle()
{
	//TCompSkeleton* skeleton = ((CEntity*)entity)->get<TCompSkeleton>();
	stopMovement();
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = last_look_direction;

	if (on_enter) {
		playAnimationIfNotPlaying(0);
	}

	if (state_time >= 2){
		return LEAVE;

	}
	else{
		return STAY;
	}

}

//Select a point to go 
int bt_soldier::actionSearchPoint()
{
	float aux_time = state_time;
	bool aux_on_enter = on_enter;

	rand_point = CNav_mesh_manager::get().getRandomNavMeshPoint(center, radius, ((TCompTransform*)own_transform)->position);
	if (V3DISTANCE(rand_point, previous_point_search) < 1.3){
		rand_point = XMVectorSet(XMVectorGetX(center), XMVectorGetY(((TCompTransform*)own_transform)->position), XMVectorGetZ(center), 0);
	}
	previous_point_search = rand_point;
	stopMovement();
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = last_look_direction;

	return LEAVE;

}

//Chase the selected point
int bt_soldier::actionWander()
{
	if (on_enter) {
		playAnimationIfNotPlaying(1);

		TCompCharacterController* m_char_controller = character_controller;

		m_char_controller->moveSpeedMultiplier = walk_speed;
		m_char_controller->airSpeed = walk_speed * 0.8f;

	}

	jump = false;
	//Tratamos de evitar cambios demasiado repentinos de ruta
	if (on_enter){
		((TCompSkeleton*)(((CEntity*)entity)->get<TCompSkeleton>()))->loopAnimation(1);
		CNav_mesh_manager::get().findPath(((TCompTransform*)own_transform)->position, rand_point, path);
		find_path_time = state_time;
		ind_path = 0;
	}
	else{
		if ((state_time - find_path_time) > 1.f){
			CNav_mesh_manager::get().findPath(((TCompTransform*)own_transform)->position, rand_point, path);
			find_path_time = state_time;
		}
	}

	if (path.size() > 0){
		if (ind_path < path.size()){
			chasePoint(((TCompTransform*)own_transform), path[ind_path]);
			if ((V3DISTANCE(((TCompTransform*)own_transform)->position, path[ind_path]) < distance_change_way_point)){
				ind_path++;
				return STAY;
			}
			else{
				return STAY;
			}
		}
		else{
			//last_look_direction = look_direction;
			return LEAVE;
		}
	}
	else{
		return LEAVE;
	}
}

//Makes a warcry
int bt_soldier::actionWarcry()
{
	if (on_enter) {
		playAnimationIfNotPlaying(18);
	}
	stopMovement();
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = last_look_direction;


	if (state_time >= getAnimationDuration(18) + 1) {
		aimanager::get().warningToClose(this, 20.f, player_transform);
		have_to_warcry = false;
		lost_player = false;
		time_searching_player = 0;
		return LEAVE;
	}
	else {
		return STAY;
	}
}

//Alert to the other grandma about the player
int bt_soldier::actionPlayerAlert()
{
	if (on_enter) {
		stopAllAnimations();
		resetTimeAnimation();
		playAnimationIfNotPlaying(21);
	}

	TCompTransform* p_transform = player_transform;
	TCompTransform* m_transform = own_transform;
	XMVECTOR dir = XMVector3Normalize(p_transform->position - m_transform->position);
	mov_direction = PxVec3(0, 0, 0);
	look_direction = Physics.XMVECTORToPxVec3(dir);
	((TCompCharacterController*)character_controller)->Move(mov_direction, false, jump, look_direction);

	if (state_time > getAnimationDuration(18)) {
		//Call the iaManager method for warning the rest of the grandmas
		aimanager::get().warningPlayerFound(this);
		return LEAVE;
	}
	else
		return STAY;
}

//Leave the angry state, go to peacefull
int bt_soldier::actionCalmDown()
{
	is_angry = false;
	player_viewed_sensor = false;
	time_searching_player = 0;
	return LEAVE;
}

//Search random point around the last place where the player was saw
int bt_soldier::actionSearchArroundLastPoint()
{
	rand_point = CNav_mesh_manager::get().getRandomNavMeshPoint(last_point_player_saw, radius, ((TCompTransform*)own_transform)->position);
	//stopMovement();
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = last_look_direction;

	return LEAVE;

}

//plays the looking for player animation
int bt_soldier::actionLookingFor(){
	if (on_enter) {
		stopAllAnimations();
		resetTimeAnimation();
		playAnimationIfNotPlaying(22);
	}

	time_searching_player += CApp::get().delta_time;
	stopMovement();

	if (state_time > getAnimationDuration(22))
		return LEAVE;
	else
		return STAY;
}

//look the player around the his last point
int bt_soldier::actionLookAround()
{
	float aux_time = state_time;
	bool aux_on_enter = on_enter;
	jump = false;
	time_searching_player += CApp::get().delta_time;
	//Tratamos de evitar cambios demasiado repentinos de ruta
	if (on_enter){

		TCompCharacterController* m_char_controller = character_controller;

		m_char_controller->moveSpeedMultiplier = run_speed;
		m_char_controller->airSpeed = run_speed * 0.8f;

		playAnimationIfNotPlaying(14);

		CNav_mesh_manager::get().findPath(((TCompTransform*)own_transform)->position, rand_point, path);
		find_path_time = state_time;
		ind_path = 0;

		//Aqui tendría que borrar el enemigo de los slots del aiManager para su posterior re-asignacion
		aimanager::get().RemoveEnemyAttacker(this);
		//----------------------------------------------------------------------------------------------
	}
	else{
		if ((state_time - find_path_time) > 1.f){
			CNav_mesh_manager::get().findPath(((TCompTransform*)own_transform)->position, rand_point, path);
			find_path_time = state_time;
		}
	}

	if (path.size() > 0){
		if (ind_path < path.size()){
			chasePoint(((TCompTransform*)own_transform), path[ind_path]);
			if ((V3DISTANCE(((TCompTransform*)own_transform)->position, path[ind_path]) < 0.4f)){
				ind_path++;
				return STAY;
			}
			else{
				return STAY;
			}
		}
		else{
			//last_look_direction = look_direction;
			return LEAVE;
		}
	}
	else{
		return LEAVE;
	}
}

//Takes a roll, attacker or taunter and a poisition to go
int bt_soldier::actionSelectRole()
{
	TCompTransform* p_transform = player_transform;
	TCompTransform* m_transform = own_transform;

	XMVECTOR left = XMVectorSet(-1, 0, 0, 0);
	XMVECTOR right = XMVectorSet(1, 0, 0, 0);
	XMVECTOR front = XMVectorSet(0, 0, 1, 0);

	see_player = false;

	time_searching_player = 0;
	CNav_mesh_manager::get().findPath(m_transform->position, p_transform->position, path);
	if (path.size() > 0) {
		float distance_x = abs(XMVectorGetX(path[path.size() - 1]) - XMVectorGetX(path[0]));
		float distance_y = abs(XMVectorGetY(path[path.size() - 1]) - XMVectorGetY(path[0]));
		float distance_z = abs(XMVectorGetZ(path[path.size() - 1]) - XMVectorGetZ(path[0]));
		if ((distance_x <= 7.5f) && (distance_z <= 7.5f) && (distance_y <= 1.5f)){
			if ((V3DISTANCE(m_transform->position, p_transform->position)) < 7.5f){
				aimanager::get().setEnemyRol(this);
				if (rol == role::ATTACKER){
					if (slot == attacker_slots::NORTH){
						slot_position = front * max_distance_to_attack;
					}
					else if (slot == attacker_slots::EAST){
						slot_position = left * max_distance_to_attack;
					}
					else if (slot == attacker_slots::WEST){
						slot_position = right * max_distance_to_attack;
					}
				}
				else if (rol == role::TAUNTER){
					slot_position = -(p_transform->position - m_transform->position);
					slot_position = XMVector3Normalize(slot_position)*max_distance_taunter;
				}
			}
			return LEAVE;
		}
	}
	return LEAVE;
}

//bool is_reacheable = false;
//Go to his position
int bt_soldier::actionChaseRoleDistance()
{
	TCompTransform* m_transform = own_transform;
	TCompTransform* p_transform = player_transform;
	CNav_mesh_manager::get().findPath(m_transform->position, p_transform->position, path);
	if (on_enter) {
		if (path.size() > 0){

			float distance = V3DISTANCE(p_transform->position, path[path.size() - 1]);
			if (distance>2.5f){
				player_out_navMesh = true;
				playAnimationIfNotPlaying(0);
				return LEAVE;
			}

			TCompCharacterController* m_char_controller = character_controller;

			m_char_controller->moveSpeedMultiplier = run_angry_speed;

			stopAllAnimations();
			resetTimeAnimation();
			playAnimationIfNotPlaying(15);

			//m_char_controller->airSpeed = run_angry_speed * 0.8f;
			ind_path = 0;
		}
		else{
			player_out_navMesh = true;
			playAnimationIfNotPlaying(0);
			return LEAVE;
		}
	}

	if (findPlayer())
		wander_target = p_transform->position;// last_point_player_saw;

	float distance = V3DISTANCE(m_transform->position, p_transform->position);
	if (distance < 7.5f) {
		return LEAVE;
	}

	if (path.size() > 0){
		float distance_x = abs(XMVectorGetX(path[path.size() - 1]) - XMVectorGetX(path[0]));
		float distance_y = abs(XMVectorGetY(path[path.size() - 1]) - XMVectorGetY(path[0]));
		float distance_z = abs(XMVectorGetZ(path[path.size() - 1]) - XMVectorGetZ(path[0]));
		if ((distance_x <= 2.f) && (distance_z <= 2.f) && (distance_y >= 1.5f)){
			return LEAVE;
		}
	}

	if (path.size() > 0){
		if (ind_path < path.size()){
			chasePoint(m_transform, path[ind_path]);
			if ((V3DISTANCE(m_transform->position, path[ind_path]) < 0.15f)){
				ind_path++;
				return STAY;
			}
			else{
				return STAY;
			}
		}
		else{
			return LEAVE;
		}
	}
	else{
		null_node = true;
		return LEAVE;
	}
}

//First attack
int bt_soldier::actionInitialAttack()
{
	TCompTransform* p_transform = player_transform;
	TCompTransform* m_transform = own_transform;

	if (on_enter) {
		initial_attack = true;
		stopAllAnimations();
		resetTimeAnimation();
		playAnimationIfNotPlaying(19);
		attacked = false;
	}

	float stop_time = 1.30f;
	if (state_time <= stop_time){
		XMVECTOR dir = XMVector3Normalize(p_transform->position - m_transform->position);
		((TCompCharacterController*)character_controller)->moveSpeedMultiplier = 5.f;
		mov_direction = Physics.XMVECTORToPxVec3(dir);
		((TCompCharacterController*)character_controller)->Move(mov_direction, false, jump, mov_direction);
	}else{
		stopMovement();
	}

	float attack_time = 0.92f;
	float distance = XMVectorGetX(XMVector3Length(p_transform->position - m_transform->position));

	if (state_time > attack_time && attacked == false && distance <= 1.55f){
		// Check if the attack reach the player
		XMVECTOR attack_direction_path = (p_transform->position - m_transform->position);
		attack_direction_path = XMVector3Normalize(attack_direction_path);
		XMVECTOR front_path = XMVector3Normalize(m_transform->getFront());
		XMVECTOR dir_path = XMVector3AngleBetweenVectors(attack_direction_path, front_path);
		float rads_path = XMVectorGetX(dir_path);
		float angle_deg_path = rad2deg(rads_path);
		if (angle_deg_path < 40.f){
			XMVECTOR particles_pos = p_transform->position - attack_direction_path * 0.5f + XMVectorSet(0, 1.7f, 0, 0);
			CHandle particle_entity = CLogicManager::get().instantiateParticleGroupOneShot(particle_name_initial_hit, particles_pos);
			((CEntity*)player)->sendMsg(TActorHit(((CEntity*)player), 101000.f, false));
			attacked = true;

			// Sound
			CSoundManager::get().playEvent("SOLDIER_HIT", m_transform->position);
		}
		else{
			last_time = timer;
			attacked = true;

			// MISS
			CSoundManager::get().playEvent("SOLDIER_MISS", m_transform->position);
			attacked = true;
		}
	}

	if (state_time >= getAnimationDuration(19)){
		last_time = timer;
		return LEAVE;
	}
	else{
		return STAY;
	}
}

//Move step by step to the roll position (leave on reach or lost)
int bt_soldier::actionSituate()
{
	TCompTransform* m_transform = own_transform;
	TCompTransform* p_transform = player_transform;

	wander_target = p_transform->position;// + slot_position;
	TCompCharacterController* m_char_controller = character_controller;
	CNav_mesh_manager::get().findPath(m_transform->position, wander_target, path);
	if (on_enter) {
		if (path.size() > 0){

			ind_path = 0;

			stopAllAnimations();
			resetTimeAnimation();
			playAnimationIfNotPlaying(15);
		}
		else{
			player_out_navMesh = true;
			playAnimationIfNotPlaying(0);
			return LEAVE;
		}
	}

	if (path.size() > 0){
		float distance_path = V3DISTANCE(wander_target, path[path.size() - 1]);
		if (distance_path >= 2.0f){
			player_out_navMesh = true;
			playAnimationIfNotPlaying(0);
			return LEAVE;
		}
	}

	m_char_controller->moveSpeedMultiplier = run_angry_speed;
	float distance = V3DISTANCE(m_transform->position, wander_target);
	if (!initial_attack){
		if (distance < 6.5f){
			return LEAVE;
		}
	}
	else{
		if (distance < 2.0f){
			return LEAVE;
		}
	}

	float distance_x = abs(XMVectorGetX(path[path.size() - 1]) - XMVectorGetX(path[0]));
	float distance_y = abs(XMVectorGetY(path[path.size() - 1]) - XMVectorGetY(path[0]));
	float distance_z = abs(XMVectorGetZ(path[path.size() - 1]) - XMVectorGetZ(path[0]));
	if ((distance_x <= 2.f) && (distance_z <= 2.f) && (distance_y >= 1.5f)){
		return LEAVE;
	}

	CNav_mesh_manager::get().findPath(m_transform->position, wander_target, path);
	if (path.size() > 0){
		if (ind_path < path.size()){
			chasePoint(m_transform, path[ind_path]);
			XMVECTOR prueba = m_transform->position;
			if ((V3DISTANCE(m_transform->position, path[ind_path]) < 0.2f)){
				ind_path++;
				return STAY;
			}
			else{
				return STAY;
			}
		}
		else{
			return LEAVE;
		}
	}
	else{
		return LEAVE;
	}
}

//
int bt_soldier::actionNormalAttack()
{
	if (on_enter) {
		attacked = false;
		int anim = getRandomNumber(4, 6);
		TCompSkeleton* m_skeleton = enemy_skeleton;
		m_skeleton->resetAnimationTime();
		playAnimationIfNotPlaying(anim);
	}

	TCompTransform* p_transform = player_transform;
	TCompTransform* m_transform = own_transform;
	XMVECTOR dir = XMVector3Normalize(p_transform->position - m_transform->position);
	look_direction = Physics.XMVECTORToPxVec3(dir);
	stopMovement();
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = Physics.XMVECTORToPxVec3(dir);

	float attack_time = 0.966f;

	if (state_time >= attack_time && attacked == false) {
		// Check if the attack reach the player
		float distance = XMVectorGetX(XMVector3Length(p_transform->position - m_transform->position));
		if (distance <= max_distance_to_attack)
		{
			XMVECTOR particles_pos = p_transform->position - dir * 0.5f + XMVectorSet(0, 1.55f, 0, 0);
			CHandle particle_entity = CLogicManager::get().instantiateParticleGroupOneShot(particle_name_initial_hit, particles_pos);
			((CEntity*)player)->sendMsg(TActorHit(((CEntity*)player), 61000.f, false));
			attacked = true;
			// Sound
			CSoundManager::get().playEvent("SOLDIER_HIT", m_transform->position);
		}
		else {
			// MISS
			CSoundManager::get().playEvent("SOLDIER_MISS", m_transform->position);
			attacked = true;
		}
	}

	if (state_time >= getAnimationDuration(last_anim_id)){
		attacked = false;
		return LEAVE;
	}
	else{
		return STAY;
	}

}

//Play a Idle war animation
int bt_soldier::actionIdleWar()
{
	if (on_enter) {
		stopAllAnimations();
		resetTimeAnimation();
		playAnimationIfNotPlaying(18);
	}

	TCompTransform* p_transform = player_transform;
	TCompTransform* m_transform = own_transform;
	XMVECTOR dir = XMVector3Normalize(p_transform->position - m_transform->position);
	look_direction = Physics.XMVECTORToPxVec3(dir);
	stopMovement();

	if (state_time > getAnimationDuration(18))
		return LEAVE;
	else
		return STAY;
}

int bt_soldier::actionIdleWarDelay()
{
	if (on_enter) {
		stopAllAnimations();
		resetTimeAnimation();
		playAnimationIfNotPlaying(18);
	}

	/*TCompTransform* p_transform = player_transform;
	TCompTransform* m_transform = own_transform;
	XMVECTOR dir = XMVector3Normalize(p_transform->position - m_transform->position);
	look_direction = Physics.XMVECTORToPxVec3(dir);*/
	stopMovement();

	if (state_time > getAnimationDuration(18))
		return LEAVE;
	else
		return STAY;
}

//Play a taunter routine
int bt_soldier::actionTaunter()
{
	//Meter animacion
	if (on_enter) {
		playAnimationIfNotPlaying(17);
	}

	TCompTransform* p_transform = player_transform;
	TCompTransform* m_transform = own_transform;
	XMVECTOR dir = XMVector3Normalize(p_transform->position - m_transform->position);
	stopMovement();
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = Physics.XMVECTORToPxVec3(dir);

	if (state_time > getAnimationDuration(17))
		return LEAVE;
	else
		return STAY;
}

//Calculate if hurts or ragdoll, if ragdoll then clean all events (los events solo tocan su flag, excepto el ragdoll)
int bt_soldier::actionHurtEvent()
{
	if (on_enter) {
		stopAllAnimations();
		resetTimeAnimation();
		playAnimationIfNotPlaying(23);
	}

	TCompTransform* p_transform = player_transform;
	TCompTransform* m_transform = own_transform;
	XMVECTOR dir = XMVector3Normalize(p_transform->position - m_transform->position);
	stopMovement();
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = Physics.XMVECTORToPxVec3(dir);

	if (state_time > getAnimationDuration(23)) {
		//Call the iaManager method for warning the rest of the grandmas
		aimanager::get().warningPlayerFound(this);
		event_detected = false;
		tied_event = false;

		return LEAVE;
	}
	else
		return STAY;

}

//Keeps in falling state till ti
int bt_soldier::actionFallingEvent()
{
	return LEAVE;
}

//
int bt_soldier::actionGetAngry()
{
	is_angry = true;
	return LEAVE;
}

//Puts are_events var to false
int bt_soldier::actionNoevents()
{
	return LEAVE;
}

//
int bt_soldier::conditionTied()
{
	return false;
}

//
int bt_soldier::conditionis_ragdoll()
{
	return is_ragdoll;
}

//
int bt_soldier::conditionis_grounded()
{
	TCompCharacterController* me_controller = ((CEntity*)entity)->get<TCompCharacterController>();
	if (me_controller->OnGround()){
		return true;
	}
	else{
		return false;
	}
}

//
int bt_soldier::conditiontrue()
{
	return true;
}

//
int bt_soldier::conditionis_angry()
{
	return is_angry;
}

//Check if the player is close enought for an annoying attack
int bt_soldier::conditiontoo_close_attack()
{

	if ((V3DISTANCE(((TCompTransform*)own_transform)->position, ((TCompTransform*)player_transform)->position) < max_dist_close_attack) && ((timer - last_time) >= delta_time_close_attack)){
		too_close_attack = true;
		last_time = timer;
	}
	else{
		too_close_attack = false;
	}
	return too_close_attack;
}

//Check if is necesary a warcry
int bt_soldier::conditionhave_to_warcry()
{
	return have_to_warcry;
}

//Check if there player is not visible for any grandma (and reach the last position)
int bt_soldier::conditionplayer_lost()
{
	if (!player_out_navMesh){
		if ((last_time_player_saw) > max_time_player_lost){
			player_previously_lost = true;
			initial_attack = false;
			return true;
		}
	}
	else{
		player_out_navMesh = false;
		return true;
	}
	return false;
}

//check if the player is visible
int bt_soldier::conditionsee_player()
{
	if ((findPlayer()) && (player_previously_lost)){
		see_player = true;
		player_previously_lost = false;
	}
	else{
		see_player = false;
	}
	return see_player;
}

int bt_soldier::conditionLook_time(){
	if (time_searching_player <= max_time_player_search){
		lost_player = true;
	}
	else{
		lost_player = false;
	}
	return lost_player;
}

//Check the look for timer
int bt_soldier::conditionLook_for_timeout()
{
	if (time_searching_player > max_time_player_search){
		return true;
	}
	else{
		return false;
	}
	//return Look_for_timeout;
}

//Check if the role is attacker and is close enought
int bt_soldier::conditionis_attacker()
{
	TCompTransform* m_transform = own_transform;
	TCompTransform* p_transform = player_transform;

	float distance = V3DISTANCE(m_transform->position, p_transform->position);// + slot_position);
	if ((rol == role::ATTACKER) && (V3DISTANCE(m_transform->position, p_transform->position) <= 7.5f)){
		return true;
	}
	else{
		return false;
	}
}

//Check if the player is in range and cold down time passed
int bt_soldier::conditionnormal_attack()
{
	TCompTransform* m_transform = own_transform;
	TCompTransform* p_transform = player_transform;

	float random_time_attack = getRandomNumber(delta_time_close_attack - 1.5f, delta_time_close_attack);
	TCompPlayerController* player_controller = ((CEntity*)player)->get<TCompPlayerController>();
	if (((V3DISTANCE(m_transform->position, p_transform->position) < 2.5f) && (timer - last_time) >= random_time_attack) && (player_controller->canReceiveDamage())){
		last_time = timer;
		return true;
	}
	else{
		return false;
	}
}

//Init on false
int bt_soldier::conditionare_events()
{
	return event_detected;
	//return are_events;
}

//Check if is a hurt event
int bt_soldier::conditionhurt_event()
{
	return hurt_event;
}

//Check if is a falling event
int bt_soldier::conditionfalling_event()
{
	return false;
	//return falling_event;
}

//Check if is a tied event
int bt_soldier::conditiontied_event()
{
	//return false;
	return tied_event;
}

//Check if the role is taunter and is close enought
int bt_soldier::conditionis_taunter()
{
	float distance = V3DISTANCE(((TCompTransform*)own_transform)->position, ((TCompTransform*)player_transform)->position);
	if ((rol == role::TAUNTER) && (distance <= 4.5f)){
		return true;
	}
	else{
		return false;
	}
	//return is_taunter;
}

//
int bt_soldier::conditioninitial_attack()
{
	TCompTransform* m_transform = own_transform;
	TCompTransform* p_transform = player_transform;

	XMVECTOR attack_direction = (p_transform->position - m_transform->position);
	attack_direction = XMVector3Normalize(attack_direction);
	XMVECTOR front = XMVector3Normalize(m_transform->getFront());
	XMVECTOR dir = XMVector3AngleBetweenVectors(attack_direction, front);
	float rads = XMVectorGetX(dir);
	float angle_deg = rad2deg(rads);

	//CNav_mesh_manager::get().findPath(m_transform->position, p_transform->position, path);
	if (path.size() > 0){
		XMVECTOR attack_direction_path = (path[path.size() - 1] - m_transform->position);
		attack_direction_path = XMVector3Normalize(attack_direction_path);
		XMVECTOR front_path = XMVector3Normalize(m_transform->getFront());
		XMVECTOR dir_path = XMVector3AngleBetweenVectors(attack_direction_path, front_path);
		float rads_path = XMVectorGetX(dir_path);
		float angle_deg_path = rad2deg(rads_path);

		float distance = V3DISTANCE(m_transform->position, p_transform->position);	
	
		float distance_path = V3DISTANCE(p_transform->position, path[path.size() - 1]);
		float distance_path_enemy = V3DISTANCE(m_transform->position, path[path.size() - 1]);
		TCompPlayerController* player_controller = ((CEntity*)player)->get<TCompPlayerController>();
		if ((!initial_attack) && (distance_path_enemy < 7.5f) && (distance_path<1.0f) && (player_controller->canReceiveDamage())){
			if ((angle_deg < 30.f) && (angle_deg_path<30.f)){
				if (distance < 7.5f){
					XDEBUG("First attack angle: %f, attack_dir.x=%f, attack_dir.y=%f, attack_dir.z=%f, front.x=%f, front.y=%f, front.z=%f, angle_deg_path=%f", +angle_deg, XMVectorGetX(attack_direction), XMVectorGetY(attack_direction), XMVectorGetZ(attack_direction), XMVectorGetX(front), XMVectorGetY(front), XMVectorGetZ(front), angle_deg_path);
					return true;
				}else{
					initial_attack = true;
					return false;
				}
			}
			else{
				XDEBUG("First attack angle: %f, attack_dir.x=%f, attack_dir.y=%f, attack_dir.z=%f, front.x=%f, front.y=%f, front.z=%f, angle_deg_path=%f", +angle_deg, XMVectorGetX(attack_direction), XMVectorGetY(attack_direction), XMVectorGetZ(attack_direction), XMVectorGetX(front), XMVectorGetY(front), XMVectorGetZ(front), angle_deg_path);
				initial_attack = true;
				return false;
			}
		}
		else{
			initial_attack = true;
			return false;
		}
	}
	return false;
}

//Check if it is too far from the target position
int bt_soldier::conditionfar_from_target_pos()
{
	TCompTransform* m_transform = own_transform;
	TCompTransform* p_transform = player_transform;

	XMVECTOR target = p_transform->position;// + slot_position;

	float distance = V3DISTANCE(m_transform->position, target);
	if (distance > 2.f){
		return true;
	}
	else{
		return false;
	}
}


/* Funciones de Control de variables, llamadas por sensores */


// Sensor para detectar si el enemigo ve al player
void bt_soldier::playerViewedSensor(){
	if (!player_viewed_sensor){

		bool tri = ((TCompPlayerPosSensor*)player_pos_sensor)->playerInRange();
		if (((TCompPlayerPosSensor*)player_pos_sensor)->playerInRange()) {
			if (current != NULL){
				if ((current->getTypeInter() == EXTERNAL) || (current->getTypeInter() == BOTH)){
					is_angry = true;
					have_to_warcry = false;
					setCurrent(NULL);
					player_viewed_sensor = true;
				}
			}
		}
		else{
			player_viewed_sensor = false;
			have_to_warcry = false;
			is_angry = false;
		}
	}
	else{
		last_time_player_saw += CApp::get().delta_time;
	}
}

void bt_soldier::hurtSensor(float damage){

	if (!is_angry)
		have_to_warcry = true;
	is_angry = true;
	if (damage >= force_large_impact){
		TCompLife* life = ((CEntity*)entity)->get<TCompLife>();
		life->life = 0;
		is_ragdoll = true;
		TCompTransform* m_transform = own_transform;
		CHandle particle_entity = CLogicManager::get().instantiateParticleGroup(particle_name_dismemberment, m_transform->position, m_transform->rotation);
		setCurrent(NULL);

	}
	else if ((damage >= force_medium_impact) && (damage < force_large_impact)){
		is_ragdoll = true;
		setCurrent(NULL);
	}
	else if (damage < force_medium_impact){
		hurt_event = true;
	}
}

void bt_soldier::WarWarningSensor(XMVECTOR player_position){
	is_angry = true;
	have_to_warcry = false;
	player_detected_pos = player_position;
	setCurrent(NULL);
}

void bt_soldier::PlayerFoundSensor(){
	last_time_player_saw = 0;
	lost_player = false;
	setCurrent(NULL);
}

void bt_soldier::update(float elapsed){

	if (active){
		playerViewedSensor();
		findLostPlayer();
		if (findPlayer()){
			last_point_player_saw = ((TCompTransform*)player_transform)->position;
			last_time_player_saw = 0;
		}
		TCompRagdoll* m_ragdoll = enemy_ragdoll;
		if (m_ragdoll) {
			if (!m_ragdoll->isRagdollActive()) {
				if ((current == NULL) || (null_node)){
					resetBot();
					null_node = false;
				}
			}
		}
		this->recalc(elapsed);
	}
	else{
		resetBot();
	}
}

bool bt_soldier::trueEveryXSeconds(float time)
{
	static float counter = 0;
	counter += CApp::get().delta_time;
	if (counter >= time) {
		counter = 0;
		return true;
	}
	return false;
}

void bt_soldier::chasePoint(TCompTransform* own_position, XMVECTOR chase_point){
	physx::PxRaycastBuffer buf;
	Physics.raycastAll(own_position->position + XMVectorSet(0, 0.1f, 0, 0), own_position->getFront(), 1.f, buf);
	for (int i = 0; i < (int)buf.nbTouches; i++)
	{
		TCompCharacterController* character_cntrl = (TCompCharacterController*)character_controller;
		TCompRigidBody* own_rigid = character_cntrl->getRigidBody();
		if (buf.touches[i].actor != (own_rigid->rigidBody)) {
			jump = true;
		}
		else{
			jump = false;
		}
	}
	mov_direction = Physics.XMVECTORToPxVec3(own_position->getFront());
	XMVECTOR m_dir = XMVector3Normalize(chase_point - own_position->position);
	look_direction = Physics.XMVECTORToPxVec3(m_dir);
	((TCompCharacterController*)character_controller)->Move(mov_direction, false, jump, look_direction);
}

CHandle bt_soldier::getPlayerTransform(){
	return player_transform;
}

void bt_soldier::findLostPlayer(){
	if (lost_player){
		if (findPlayer()){
			lost_player = false;
			player_previously_lost = true;
			setCurrent(NULL);
		}
	}
}

bool bt_soldier::findPlayer(){
	if (((TCompPlayerPosSensor*)player_pos_sensor)->playerInRange()) {
		return true;
	}
	else{
		return false;
	}
}

bool bt_soldier::isAngry(){
	return is_angry;
}

void bt_soldier::setRol(int r){
	if (r == 1)
		rol = role::ATTACKER;
	else if (r == 2)
		rol = role::TAUNTER;
}

void bt_soldier::setAttackerSlot(int s){
	if (s == 1){
		slot = attacker_slots::NORTH;
	}
	else if (s == 2){
		slot = attacker_slots::EAST;
	}
	else if (s == 3){
		slot = attacker_slots::WEST;
	}
}

int bt_soldier::getAttackerSlot(){
	return slot;
}

int bt_soldier::getRol(){
	return rol;
}

float bt_soldier::getDistanceToPlayer(){
	if (own_transform.isValid())
		return V3DISTANCE(((TCompTransform*)own_transform)->position, ((TCompTransform*)player_transform)->position);
	else
		return 0.f;
}

int bt_soldier::getNearestSlot(bool free_north, bool free_east, bool free_west){
	float distanceToNorth = 1000.f;
	float distanceToEast = 1000.f;
	float distanceToWest = 1000.f;
	TCompTransform* p_transform = player_transform;
	TCompTransform* m_transform = own_transform;
	XMVECTOR left = XMVectorSet(-1, 0, 0, 0);
	XMVECTOR right = XMVectorSet(1, 0, 0, 0);
	XMVECTOR front = XMVectorSet(0, 0, 1, 0);

	if (free_north){
		XMVECTOR slot_positionNorth = front * max_distance_to_attack;
		XMVECTOR pos = (p_transform->position + slot_positionNorth);
		distanceToNorth = V3DISTANCE(pos, m_transform->position);
	}

	if (free_east){
		XMVECTOR slot_positionEast = left * max_distance_to_attack;
		XMVECTOR pos = (p_transform->position + slot_positionEast);
		distanceToEast = V3DISTANCE(pos, m_transform->position);
	}

	if (free_west){
		XMVECTOR slot_positionWest = right * max_distance_to_attack;
		XMVECTOR pos = (p_transform->position + slot_positionWest);
		distanceToWest = V3DISTANCE(pos, m_transform->position);
	}

	if ((distanceToNorth <= distanceToEast) && (distanceToNorth <= distanceToWest)){
		//devolvemos north
		return 0;
	}
	else if ((distanceToEast <= distanceToNorth) && (distanceToEast <= distanceToWest)){
		//devolvemos east
		return 1;
	}
	else if ((distanceToWest <= distanceToNorth) && (distanceToWest <= distanceToEast)){
		//devolvemos west
		return 2;
	}
	return -1;
}


void bt_soldier::drawdebug() {
	font.print3D(wander_target, "Destino");
	if (slot == attacker_slots::NORTH)
		font.print3D(wander_target + XMVectorSet(0.f, 0.5f, 0.f, 0.f), "NORTH");
	if (slot == attacker_slots::WEST)
		font.print3D(wander_target + XMVectorSet(0.f, 0.5f, 0.f, 0.f), "WEST");
	if (slot == attacker_slots::EAST)
		font.print3D(wander_target + XMVectorSet(0.f, 0.5f, 0.f, 0.f), "EAST");
}

void bt_soldier::stopAllAnimations() {
	TCompSkeleton* m_skeleton = enemy_skeleton;

	for (int i = 0; i < 20; ++i) {
		m_skeleton->model->getMixer()->clearCycle(i, 0.3f);
	}
}

void bt_soldier::playAnimationIfNotPlaying(int id) {
	TCompSkeleton* m_skeleton = enemy_skeleton;

	if (id != last_anim_id) {
		stopAnimation(last_anim_id);
		last_anim_id = id;
		m_skeleton->loopAnimation(id);
	}
}

void bt_soldier::stopAnimation(int id) {
	TCompSkeleton* m_skeleton = enemy_skeleton;
	m_skeleton->stopAnimation(id);
}

float bt_soldier::getAnimationDuration(int id) {
	TCompSkeleton* m_skeleton = enemy_skeleton;
	//float dur = m_skeleton->model->getMixer()->getAnimationVector()[id]->getCoreAnimation()->getDuration();
	float res = m_skeleton->model->getMixer()->getAnimationDuration();
	return res;
}

void bt_soldier::resetTimeAnimation(){
	TCompSkeleton* m_skeleton = enemy_skeleton;
	m_skeleton->resetAnimationTime();
}

void bt_soldier::setActive(bool act){
	active = act;
}

void bt_soldier::needleHitSensor(){
	if ((current) && ((current->getTypeInter() == EXTERNAL))){
		setCurrent(NULL);
		tied_event = true;
		event_detected = true;
	}
}

void bt_soldier::setIndRecastAABB(int ind){
	ind_recast_aabb = ind;
}

int bt_soldier::getIndRecastAABB(){
	return ind_recast_aabb;
}

void bt_soldier::stopMovement(){
	mov_direction = PxVec3(0, 0, 0);
	((TCompCharacterController*)character_controller)->Move(mov_direction, false, false, look_direction);
}

void bt_soldier::resetBot(){
	setCurrent(NULL);
	playAnimationIfNotPlaying(10);
	stopMovement();
	//mov_direction = PxVec3(0, 0, 0);
	//((TCompCharacterController*)character_controller)->Move(mov_direction, false, false, look_direction);
}
