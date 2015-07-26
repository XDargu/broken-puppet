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
const float max_dist_reach_needle = 1.8f;
const float max_dist_close_attack = 1.7f;
const float max_time_player_lost = 2.f;
const float max_distance_to_attack = 1.f;
const float max_time_player_search = 7.f;
const float max_range_role = 7.f;
const float max_distance_taunter = 4.f;
const float delta_time_close_attack = 6.f;
const float distance_change_way_point = 0.55f;
const float force_large_impact = 500.f;
const float force_medium_impact = 100.f;
const float max_time_ragdoll = 3.f;
const float radius = 7.f;

const float walk_speed = 0.8f;
const float run_speed = 2.f;
const float run_angry_speed = 2.2f;


// Sensor
const float sensor_delay = 1.f;
float sensor_acum_soldier = 0.f;


void bt_soldier::create(string s)
{
	name = s;
	createRoot("Root", PRIORITY, NULL, NULL);
	addChild("Root", "Ragdoll", SEQUENCE, (btcondition)&bt_soldier::conditionis_ragdoll, NULL);
	addChild("Ragdoll", "ActionRagdoll1", ACTION, NULL, (btaction)&bt_soldier::actionRagdoll);
	addChild("Ragdoll", "Awake", PRIORITY, NULL, NULL);
	addChild("Awake", "WakeUp", SEQUENCE, (btcondition)&bt_soldier::conditionTied, NULL);

	addChild("Awake", "Grounded", PRIORITY, (btcondition)&bt_soldier::conditiontrue, NULL);
	addChild("Grounded", "ActionWakeUp4", ACTION, (btcondition)&bt_soldier::conditionis_grounded, (btaction)&bt_soldier::actionWakeUp);
	addChild("Grounded", "Leave5", ACTION, (btcondition)&bt_soldier::conditiontrue, (btaction)&bt_soldier::actionLeave);

	addChild("Root", "events", PRIORITY, (btcondition)&bt_soldier::conditionare_events, NULL);
	addChild("events", "HurtEvent7", ACTION, (btcondition)&bt_soldier::conditionhurt_event, (btaction)&bt_soldier::actionHurtEvent);
	addChild("events", "FallingEvent8", ACTION, (btcondition)&bt_soldier::conditionfalling_event, (btaction)&bt_soldier::actionFallingEvent);
	addChild("events", "No events10", ACTION, (btcondition)&bt_soldier::conditiontrue, (btaction)&bt_soldier::actionNoevents);
	addChild("Root", "Angry", PRIORITY, (btcondition)&bt_soldier::conditionis_angry, NULL);

	addChild("Angry", "Warcry11", ACTION, (btcondition)&bt_soldier::conditionhave_to_warcry, (btaction)&bt_soldier::actionWarcry);
	addChild("Angry", "LookForPlayer", PRIORITY, (btcondition)&bt_soldier::conditionplayer_lost, NULL);
	addChild("Angry", "PlayerAlert12", ACTION, (btcondition)&bt_soldier::conditionsee_player, (btaction)&bt_soldier::actionPlayerAlert);

	addChild("LookForPlayer", "CalmDown13", ACTION, (btcondition)&bt_soldier::conditionLook_for_timeout, (btaction)&bt_soldier::actionCalmDown);

	addChild("LookForPlayer", "LookAroundSequence", SEQUENCE, (btcondition)&bt_soldier::conditiontrue, NULL);
	addChild("LookAroundSequence", "SearchLastPoint", ACTION, NULL, (btaction)&bt_soldier::actionSearchArroundLastPoint);
	addChild("LookAroundSequence", "LookAround14", ACTION, NULL, (btaction)&bt_soldier::actionLookAround);

	addChild("Angry", "TryAttack", SEQUENCE, (btcondition)&bt_soldier::conditiontrue, NULL);
	addChild("TryAttack", "SelectRole15", ACTION, NULL, (btaction)&bt_soldier::actionSelectRole);
	addChild("TryAttack", "ExecuteRole", PRIORITY, NULL, NULL);
	addChild("ExecuteRole", "AttackRoutine", PRIORITY, (btcondition)&bt_soldier::conditionis_attacker, NULL);
	addChild("AttackRoutine", "InitialAttack16", ACTION, INTERNAL, (btcondition)&bt_soldier::conditioninitial_attack, (btaction)&bt_soldier::actionInitialAttack);
	addChild("AttackRoutine", "NormalAttack17", ACTION, INTERNAL, (btcondition)&bt_soldier::conditionnormal_attack, (btaction)&bt_soldier::actionNormalAttack);
	addChild("AttackRoutine", "Situate18", ACTION, (btcondition)&bt_soldier::conditionfar_from_target_pos, (btaction)&bt_soldier::actionSituate);
	addChild("AttackRoutine", "IdleWa19r", ACTION, (btcondition)&bt_soldier::conditiontrue, (btaction)&bt_soldier::actionIdleWar);
	addChild("ExecuteRole", "Taunter", PRIORITY, (btcondition)&bt_soldier::conditionis_taunter, NULL);
	addChild("Taunter", "Situate20", ACTION, (btcondition)&bt_soldier::conditionfar_from_target_pos, (btaction)&bt_soldier::actionSituate);
	addChild("Taunter", "Taunter21", ACTION, NULL, (btaction)&bt_soldier::actionTaunter);

	addChild("ExecuteRole", "ChaseRoleDistance22", ACTION, (btcondition)&bt_soldier::conditiontrue, (btaction)&bt_soldier::actionChaseRoleDistance);

	addChild("Root", "Peacefull", PRIORITY, (btcondition)&bt_soldier::conditiontrue, NULL);
	
	
	addChild("Peacefull", "FreeTime", RANDOM, (btcondition)&bt_soldier::conditiontrue, NULL);
	addChild("FreeTime", "Idle29", ACTION, EXTERNAL, NULL, (btaction)&bt_soldier::actionIdle, 70);
	addChild("FreeTime", "Wander30", SEQUENCE, EXTERNAL, NULL, NULL, 30);

	addChild("Wander30", "SearchPoint", ACTION, EXTERNAL, NULL, (btaction)&bt_soldier::actionSearchPoint);
	addChild("Wander30", "ActionWander", ACTION, EXTERNAL, NULL, (btaction)&bt_soldier::actionWander);


	type = aicontroller::types::soldier;
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
	needle_to_take = false;
	can_reach_needle = false;
	is_needle_tied = false;
	needle_is_valid = false;
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
	cut = false;
	attacked = false;

	player_out_navMesh = false;

	ropeRef = CHandle();
	player_detected_pos = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	previous_point_search = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	slot_position = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	m_sensor = ((CEntity*)entity)->get<TCompSensorNeedles>();
	player_pos_sensor = ((CEntity*)entity)->get<TCompPlayerPosSensor>();
	player_transform = ((CEntity*)player)->get<TCompTransform>();
	rol = role::UNASIGNATED;
	slot = attacker_slots::NO_SLOT;

	((TCompCharacterController*)character_controller)->lerpRotation = 0.1f;

	resetBot();
}

//Se mantiene en modo ragdoll durante un tiempo
int bt_soldier::actionRagdoll()
{
	TCompRagdoll* m_ragdoll = enemy_ragdoll;

	stopAllAnimations();

	if (m_ragdoll) {

		if (!m_ragdoll->isRagdollActive()) {
			m_ragdoll->setActive(true);

			TCompLife* m_life = ((CEntity*)entity)->get<TCompLife>();
			if (m_life->life <= 0) {
				TCompRagdoll* m_ragdoll = enemy_ragdoll;
				m_ragdoll->breakJoints();
				TCompTransform* m_transform = own_transform;
				TCompAABB* ragdoll_aabb = (TCompAABB*)((CEntity*)entity)->get<TCompAABB>();
				XMVECTOR min = XMVectorSet(-50, -50, -50, 0);
				XMVECTOR max = XMVectorSet(50, 50, 50, 0);

				CNav_mesh_manager::get().removeCapsule(((CEntity*)entity)->get<TCompColliderCapsule>());
				if (this->getRol() == role::ATTACKER)
					aimanager::get().RemoveEnemyAttacker(this);
				else
					aimanager::get().RemoveEnemyTaunt(this);

				//CEntityManager::get().remove(((CEntity*)entity)->get<TCompRigidBody>());
				//CEntityManager::get().remove(((CEntity*)entity)->get<TCompColliderCapsule>());

				CEntityManager::get().remove(((CEntity*)entity)->get<TCompCharacterController>());
				ragdoll_aabb->setIdentityMinMax(min, max);

				aimanager::get().removeBot(this->getId());

				CEntityManager::get().remove(((CEntity*)entity)->get<TCompBtSoldier>());

				TCompTransform* p_transform = player_transform;
				if (V3DISTANCE(p_transform->position, m_transform->position) < 10) {
					CEntity* camera = CEntityManager::get().getByName("PlayerCamera");
					TCompTransform* c_transform = camera->get<TCompTransform>();
					TCompCamera* c_camera = camera->get<TCompCamera>();
					if (c_transform->isInFov(m_transform->position, c_camera->getFov())) {
						CApp::get().slowMotion(3);
					}
				}

			}

		}
	}

	XMVECTOR spine_pos = ((TCompSkeleton*)enemy_skeleton)->getPositionOfBone(3);

	XMVECTOR pos_orig = Physics.PxVec3ToXMVECTOR(((TCompRigidBody*)enemy_rigid)->rigidBody->getGlobalPose().p);
	XMVECTOR pos_final = XMVectorLerp(pos_orig, spine_pos, 0.1f);

	((TCompRigidBody*)enemy_rigid)->rigidBody->setGlobalPose(
		physx::PxTransform(
		Physics.XMVECTORToPxVec3(pos_final),
		((TCompRigidBody*)enemy_rigid)->rigidBody->getGlobalPose().q
		)
		);

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
		m_skeleton->playAnimation(16);

	}

	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = last_look_direction;

	if (state_time > getAnimationDuration(16)) {
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

	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = last_look_direction;

	if (state_time >= getAnimationDuration(7)) {
		((CEntity*)player)->sendMsg(TActorHit(((CEntity*)player), 150.f));
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

	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = last_look_direction;


	if (state_time >= getAnimationDuration(18) + 1) {
		aimanager::get().warningToClose(this, 20.f, player_transform);
		have_to_warcry = false;
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
		playAnimationIfNotPlaying(17);
	}

	TCompTransform* p_transform = player_transform;
	TCompTransform* m_transform = own_transform;
	XMVECTOR dir = XMVector3Normalize(p_transform->position - m_transform->position);
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = Physics.XMVECTORToPxVec3(dir);

	if (state_time > getAnimationDuration(17)) {
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

	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = last_look_direction;

	return LEAVE;

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

	time_searching_player = 0;
	if ((V3DISTANCE(m_transform->position, p_transform->position))<4.f){
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

//bool is_reacheable = false;
//Go to his position
int bt_soldier::actionChaseRoleDistance()
{
	TCompTransform* m_transform = own_transform;
	TCompTransform* p_transform = player_transform;
	CNav_mesh_manager::get().findPath(m_transform->position, p_transform->position, path);
	if (on_enter) {
		if (path.size() > 0){
			playAnimationIfNotPlaying(15);

			TCompCharacterController* m_char_controller = character_controller;

			m_char_controller->moveSpeedMultiplier = run_angry_speed;
			m_char_controller->airSpeed = run_angry_speed * 0.8f;
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
	if (distance < 4.f) {
		return LEAVE;
	}

	if (path.size() > 0){
		if (ind_path < path.size()){
			chasePoint(m_transform, path[ind_path]);
			if ((V3DISTANCE(m_transform->position, path[ind_path]) < 0.4f)){
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
	if (on_enter) {
		initial_attack = true;
		playAnimationIfNotPlaying(11);
		attacked = false;
	}

	TCompTransform* p_transform = player_transform;
	TCompTransform* m_transform = own_transform;
	XMVECTOR dir = XMVector3Normalize(p_transform->position - m_transform->position);
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = Physics.XMVECTORToPxVec3(dir);

	if ((state_time > getAnimationDuration(11) / 5) && (!attacked)) {
		// Check if the attack reach the player
		float distance = XMVectorGetX(XMVector3Length(p_transform->position - m_transform->position));
		if (distance <= max_distance_to_attack * 2)
		{
			((CEntity*)player)->sendMsg(TActorHit(((CEntity*)player), 300.f));
		}
		attacked = true;
		return LEAVE;
	}
	else
		return STAY;
}

//Move step by step to the roll position (leave on reach or lost)
int bt_soldier::actionSituate()
{
	TCompTransform* m_transform = own_transform;
	TCompTransform* p_transform = player_transform;

	wander_target = p_transform->position;// + slot_position;
	CNav_mesh_manager::get().findPath(m_transform->position, wander_target, path);
	if (on_enter) {
		if (path.size() > 0){
			TCompCharacterController* m_char_controller = character_controller;

			m_char_controller->moveSpeedMultiplier = run_angry_speed;
			m_char_controller->airSpeed = run_angry_speed * 0.8f;
			playAnimationIfNotPlaying(15);
		}
		else{
			player_out_navMesh = true;
			playAnimationIfNotPlaying(0);
			return LEAVE;
		}
	}

	if (on_enter){
		ind_path = 0;
	}

	float distance = V3DISTANCE(m_transform->position, wander_target);
	if (distance < 1.5f) {
		return LEAVE;
	}

	CNav_mesh_manager::get().findPath(m_transform->position, wander_target, path);
	if (path.size() > 0){
		if (ind_path < path.size()){
			chasePoint(m_transform, path[ind_path]);
			XMVECTOR prueba = m_transform->position;
			if ((V3DISTANCE(m_transform->position, path[ind_path]) < 1.3f)){
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
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = Physics.XMVECTORToPxVec3(dir);

	if ((state_time  > getAnimationDuration(4)) && (!attacked)) {
		// Check if the attack reach the player
		float distance = XMVectorGetX(XMVector3Length(p_transform->position - m_transform->position));
		if (distance <= max_distance_to_attack * 2)
		{
			((CEntity*)player)->sendMsg(TActorHit(((CEntity*)player), 250.f));
		}
		attacked = true;
		return LEAVE;
	}
	else
		return STAY;
}

//Play a Idle war animation
int bt_soldier::actionIdleWar()
{
	if (on_enter) {
		playAnimationIfNotPlaying(10);
	}

	TCompTransform* p_transform = player_transform;
	TCompTransform* m_transform = own_transform;
	XMVECTOR dir = XMVector3Normalize(p_transform->position - m_transform->position);
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = Physics.XMVECTORToPxVec3(dir);

	if (state_time > getAnimationDuration(10))
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
		playAnimationIfNotPlaying(9);
	}

	TCompTransform* p_transform = player_transform;
	TCompTransform* m_transform = own_transform;
	XMVECTOR dir = XMVector3Normalize(p_transform->position - m_transform->position);
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = Physics.XMVECTORToPxVec3(dir);

	if (state_time > getAnimationDuration(9)) {
		//Call the iaManager method for warning the rest of the grandmas
		aimanager::get().warningPlayerFound(this);
		event_detected = false;
		return LEAVE;
	}
	else
		return STAY;
}

int bt_soldier::actionTiedEvent()
{

	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = last_look_direction;

	if (on_enter) {

		if (ropeRef == nullptr){
			tied_event = false;
			event_detected = false;
			return LEAVE;
		}
		else{
			//Plays the cut own string animation
			int dice = getRandomNumber(0, 10);
			if (dice < max_bf_posibilities){
				// Ninja animation
				playAnimationIfNotPlaying(11);
				tied_event = false;
				event_detected = false;
			}
			else{
				tied_event = false;
				event_detected = false;
				tied_succesfull = true;
				return LEAVE;
			}
		}

	}

	if (state_time < getAnimationDuration(11)){
		return STAY;
	}
	else{
		CEntityManager::get().remove(CHandle(ropeRef).getOwner());
		tied_event = false;
		event_detected = false;
		is_angry = true;
		have_to_warcry = true;
		return LEAVE;
	}

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
	//Podría quitar see_player
	if ((findPlayer()) && (player_previously_lost)){
		see_player = true;
		player_previously_lost = false;
	}
	else{
		see_player = false;
	}
	return see_player;
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
	if ((rol == role::ATTACKER) && (V3DISTANCE(m_transform->position, p_transform->position) <= 4.5f)){
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

	if (/*(player_touch)||*/((V3DISTANCE(m_transform->position, p_transform->position) < 2.5f) && (timer - last_time) >= delta_time_close_attack)){
		last_time = timer;
		//player_touch = false;
		return true;
	}
	else{
		return false;
	}
	//return normal_attack;
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

	if ((!initial_attack) && ((V3DISTANCE(m_transform->position, p_transform->position) < 2.f))){
		return true;
	}
	else{
		return false;
	}
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
			is_angry=false;
		}
	}
	else{
		last_time_player_saw += CApp::get().delta_time;
	}
}

void bt_soldier::tiedSensor(){
	((TCompSensorTied*)tied_sensor)->keepTied();
	if (!tied_succesfull){
		if (!tied_event){
			if (((TCompSensorTied*)tied_sensor)->getTiedState()){
				ropeRef = (TCompRope*)((TCompSensorTied*)tied_sensor)->getRopeRef();
				setCurrent(NULL);
				tied_event = true;
				event_detected = true;
			}
		}
	}
	else{
		if (!((TCompSensorTied*)tied_sensor)->getTiedState()){
			tied_succesfull = false;
			tied_event = false;
			event_detected = false;
		}
	}
}

void bt_soldier::hurtSensor(float damage){

	if (!is_angry)
		have_to_warcry = true;
	is_angry = true;
	if (damage >= force_large_impact){
		TCompLife* life = ((CEntity*)entity)->get<TCompLife>();
		life->life = 0;
		stopAllAnimations();
		is_ragdoll = true;
		setCurrent(NULL);

	}
	else if ((damage >= force_medium_impact) && (damage < force_large_impact)){
		stopAllAnimations();
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
	setCurrent(NULL);
}

void bt_soldier::update(float elapsed){

	if (active){
		sensor_acum_soldier += elapsed;

		if (sensor_delay <= sensor_acum_soldier)
		{
			sensor_acum_soldier = 0;
		}


		playerViewedSensor();
		//tiedSensor();
		if (findPlayer()){
			last_point_player_saw = ((TCompTransform*)player_transform)->position;
			last_time_player_saw = 0;
		}
		/*else{
		float prueba = V3DISTANCE(((TCompTransform*)own_transform)->position, ((TCompTransform*)player_transform)->position);
		if ((is_angry) && (V3DISTANCE(((TCompTransform*)own_transform)->position, ((TCompTransform*)player_transform)->position)>radius)){
		if (rol == role::ATTACKER)
		aimanager::get().RemoveEnemyAttacker(this);
		}
		}*/
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
	look_direction = Physics.XMVECTORToPxVec3(chase_point - own_position->position);
	((TCompCharacterController*)character_controller)->Move(mov_direction, false, jump, look_direction);
}

CHandle bt_soldier::getPlayerTransform(){
	return player_transform;
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

	float res = m_skeleton->model->getMixer()->getAnimationDuration();
	return res;
}

void bt_soldier::setActive(bool act){
	active = act;
}

void bt_soldier::setIndRecastAABB(int ind){
	ind_recast_aabb = ind;
}

int bt_soldier::getIndRecastAABB(){
	return ind_recast_aabb;
}

void bt_soldier::resetBot(){
	setCurrent(NULL);
	playAnimationIfNotPlaying(10);
	mov_direction = PxVec3(0, 0, 0);
	((TCompCharacterController*)character_controller)->Move(mov_direction, false, false, look_direction);
}

void bt_soldier::setType(aicontroller::types kind){
	type = kind;
};

aicontroller::types bt_soldier::getType(){
	return type;
};
