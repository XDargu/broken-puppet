#include "mcv_platform.h"
#include "bt_grandma.h"
#include "../entity_manager.h"
#include "../components/all_components.h"
#include "utils.h"
#include "nav_mesh_manager.h"
#include "components\comp_skeleton.h"
#include "font\font.h"

//Constants
const int max_bf_posibilities = 7;
const float max_dist_reach_needle = 2.15f;
const float max_dist_to_needle = 2.25f;
const float max_dist_close_attack = 1.7f;
const float max_time_player_lost = 2.f;
const float max_time_tied = 2.f;
const float max_distance_to_attack = 1.5f;
const float max_time_player_search = 7.f;
const float max_range_role = 7.f;
const float max_distance_taunter = 4.f;
const float delta_time_close_attack = 3.5f;
const float distance_change_way_point = 0.35f;
const float force_large_impact = 60000.f;
const float force_medium_impact = 25000.f;
const float max_time_ragdoll = 3.f;
const float radius = 7.f;

const float walk_speed = 0.8f;
const float run_speed = 2.2f;
const float run_angry_speed = 4.f;
const float sensor_delay = 1.f;
const string particle_name_dismemberment = "ps_wood_hit";
const string particle_name_initial_hit = "ps_attack2";


void bt_grandma::create(string s)
{
	name = s;
	createRoot("Root", PRIORITY, NULL, NULL);
	addChild("Root", "Ragdoll", SEQUENCE, INTERNAL, (btcondition)&bt_grandma::conditionis_ragdoll, NULL);
	addChild("Ragdoll", "ActionRagdoll1", ACTION, INTERNAL, NULL, (btaction)&bt_grandma::actionRagdoll);
	addChild("Ragdoll", "Awake", PRIORITY, INTERNAL, NULL, NULL);
	addChild("Awake", "WakeUp", SEQUENCE, INTERNAL, (btcondition)&bt_grandma::conditiontied_event, NULL);
	addChild("Awake", "GroundedTied", PRIORITY, NULL, NULL);
	addChild("GroundedTied", "ActionWakeUp2", ACTION, INTERNAL, (btcondition)&bt_grandma::conditionis_grounded, (btaction)&bt_grandma::actionWakeUp);
	addChild("GroundedTied", "CutOwnSec", SEQUENCE, (btcondition)&bt_grandma::conditiontrue, NULL);
	
	
	//addChild("CutOwnSec", "WaitTied", SEQUENCE, NULL, NULL);																				//Este estaba en undefined 
	addChild("CutOwnSec", "actionWaitTied", ACTION, NULL, (btaction)&bt_grandma::actionWaitSec);
	addChild("CutOwnSec", "actionCutTied", ACTION, NULL, (btaction)&bt_grandma::actionCutOwn);
	
	
	addChild("WakeUp", "CutOwn3", ACTION, NULL, (btaction)&bt_grandma::actionCutOwn);
	addChild("Awake", "Grounded", PRIORITY, (btcondition)&bt_grandma::conditiontrue, NULL);
	addChild("Grounded", "ActionWakeUp4", ACTION, (btcondition)&bt_grandma::conditionis_grounded, (btaction)&bt_grandma::actionWakeUp);
	addChild("Grounded", "Leave5", ACTION, (btcondition)&bt_grandma::conditiontrue, (btaction)&bt_grandma::actionLeave);
	addChild("Ragdoll", "GetAngry6", ACTION, NULL, (btaction)&bt_grandma::actionGetAngry);
	addChild("Root", "events", PRIORITY, (btcondition)&bt_grandma::conditionare_events, NULL);
	addChild("events", "HurtEvent7", ACTION, EXTERNAL,  (btcondition)&bt_grandma::conditionhurt_event, (btaction)&bt_grandma::actionHurtEvent);
	//addChild("events", "NeedleHit", ACTION, (btcondition)&bt_grandma::conditionneedle_hit_event, (btaction)&bt_grandma::actionNeedleHit);
	addChild("events", "FallingEvent8", ACTION, (btcondition)&bt_grandma::conditionfalling_event, (btaction)&bt_grandma::actionFallingEvent);


	addChild("events", "TiedEvent9", SEQUENCE, (btcondition)&bt_grandma::conditiontied_event, NULL);
	//addChild("TiedEvent9", "TiedHit", ACTION, EXTERNAL, NULL, (btaction)&bt_grandma::actionHurtEvent);
	addChild("TiedEvent9", "TiedBreakDown", ACTION, NULL, (btaction)&bt_grandma::actionTiedEvent);
	//	addChild("events", "TiedEvent9", SEQUENCE, (btcondition)&bt_grandma::conditiontied_event, (btaction)&bt_grandma::actionTiedEvent);

	addChild("events", "No events10", ACTION, (btcondition)&bt_grandma::conditiontrue, (btaction)&bt_grandma::actionNoevents);
	addChild("Root", "Angry", PRIORITY, (btcondition)&bt_grandma::conditionis_angry, NULL);

	addChild("Angry", "Warcry11", ACTION, INTERNAL, (btcondition)&bt_grandma::conditionhave_to_warcry, (btaction)&bt_grandma::actionWarcry);
	addChild("Angry", "LookForPlayer", PRIORITY, (btcondition)&bt_grandma::conditionplayer_lost, NULL);
	addChild("Angry", "PlayerAlert12", ACTION, EXTERNAL, (btcondition)&bt_grandma::conditionsee_player, (btaction)&bt_grandma::actionPlayerAlert);

	addChild("LookForPlayer", "LookAroundPriority", PRIORITY, EXTERNAL, (btcondition)&bt_grandma::conditiontrue, NULL);
	addChild("LookAroundPriority", "LookAroundSequence", SEQUENCE, EXTERNAL,(btcondition)&bt_grandma::conditionLook_time, NULL);
	addChild("LookAroundPriority", "CalmDown13", ACTION, EXTERNAL,(btcondition)&bt_grandma::conditiontrue, (btaction)&bt_grandma::actionCalmDown);
	addChild("LookAroundSequence", "SearchLastPoint", ACTION, EXTERNAL, NULL, (btaction)&bt_grandma::actionSearchArroundLastPoint);
	addChild("LookAroundSequence", "LookAround14", ACTION, EXTERNAL, NULL, (btaction)&bt_grandma::actionLookAround);
	addChild("LookAroundSequence", "LookingForPlayer", ACTION, EXTERNAL, NULL, (btaction)&bt_grandma::actionLookingFor);


	addChild("Angry", "TryAttack", SEQUENCE, (btcondition)&bt_grandma::conditiontrue, NULL);
	addChild("TryAttack", "SelectRole15", ACTION, NULL, (btaction)&bt_grandma::actionSelectRole);
	addChild("TryAttack", "ExecuteRole", PRIORITY, NULL, NULL);
	addChild("ExecuteRole", "AttackRoutine", PRIORITY, (btcondition)&bt_grandma::conditionis_attacker, NULL);
	addChild("AttackRoutine", "InitialAttack16", ACTION, INTERNAL, (btcondition)&bt_grandma::conditioninitial_attack, (btaction)&bt_grandma::actionInitialAttack);
	addChild("AttackRoutine", "NormalAttack17", ACTION, INTERNAL, (btcondition)&bt_grandma::conditionnormal_attack, (btaction)&bt_grandma::actionNormalAttack);
	addChild("AttackRoutine", "Situate18", ACTION, EXTERNAL, (btcondition)&bt_grandma::conditionfar_from_target_pos, (btaction)&bt_grandma::actionSituate);
	addChild("AttackRoutine", "IdleWa19r", ACTION, EXTERNAL, (btcondition)&bt_grandma::conditiontrue, (btaction)&bt_grandma::actionIdleWar);
	addChild("ExecuteRole", "Taunter", PRIORITY, (btcondition)&bt_grandma::conditionis_taunter, NULL);
	addChild("Taunter", "Situate20", ACTION, EXTERNAL, (btcondition)&bt_grandma::conditionfar_from_target_pos, (btaction)&bt_grandma::actionSituate);
	addChild("Taunter", "Taunter21", ACTION, EXTERNAL, NULL, (btaction)&bt_grandma::actionTaunter);

	addChild("ExecuteRole", "ChaseRoleDistance22", ACTION, EXTERNAL, (btcondition)&bt_grandma::conditiontrue, (btaction)&bt_grandma::actionChaseRoleDistance);

	addChild("Root", "Peacefull", PRIORITY, (btcondition)&bt_grandma::conditiontrue, NULL);
	addChild("Peacefull", "XSecAttack", SEQUENCE, (btcondition)&bt_grandma::conditiontoo_close_attack, NULL);
	addChild("XSecAttack", "TooCloseAttack23", ACTION, INTERNAL, NULL, (btaction)&bt_grandma::actionTooCloseAttack);
	addChild("Peacefull", "TakeNeedle", SEQUENCE, EXTERNAL, (btcondition)&bt_grandma::conditionneedle_to_take, NULL);
	addChild("TakeNeedle", "XSecsNeedle", SEQUENCE, EXTERNAL, NULL, NULL);
	addChild("XSecsNeedle", "NeedleAppearsEvent24", ACTION, EXTERNAL, NULL, (btaction)&bt_grandma::actionNeedleAppearsEvent);
	addChild("TakeNeedle", "SelectNeedleToTake25", ACTION, EXTERNAL, NULL, (btaction)&bt_grandma::actionSelectNeedleToTake);
	addChild("TakeNeedle", "ChaseAndTakeNeedle", PRIORITY, EXTERNAL, NULL, NULL);
	addChild("ChaseAndTakeNeedle", "HowToCutAndTakeNeedle", PRIORITY, EXTERNAL, (btcondition)&bt_grandma::conditioncan_reach_needle, NULL);
	addChild("HowToCutAndTakeNeedle", "CutRope26", ACTION, EXTERNAL, (btcondition)&bt_grandma::conditionis_needle_tied, (btaction)&bt_grandma::actionCutRope);
	addChild("HowToCutAndTakeNeedle", "TakeNeedle27", ACTION, EXTERNAL, (btcondition)&bt_grandma::conditiontrue, (btaction)&bt_grandma::actionTakeNeedle);
	addChild("ChaseAndTakeNeedle", "ChaseNeedlePosition28", ACTION, EXTERNAL, (btcondition)&bt_grandma::conditiontrue, (btaction)&bt_grandma::actionChaseNeedlePosition);
	
	addChild("Peacefull", "FreeTime", RANDOM, (btcondition)&bt_grandma::conditiontrue, NULL);
	addChild("FreeTime", "Idle29", ACTION, EXTERNAL, NULL, (btaction)&bt_grandma::actionIdle, 70);
	addChild("FreeTime", "Wander30", SEQUENCE, EXTERNAL, NULL, NULL, 30);

	addChild("Wander30", "SearchPoint", ACTION, EXTERNAL, NULL, (btaction)&bt_grandma::actionSearchPoint);
	addChild("Wander30", "ActionWander", ACTION, EXTERNAL, NULL, (btaction)&bt_grandma::actionWander);

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
	needle_to_take = false;
	needle_is_valid = false;
	is_angry = false;
	have_to_warcry = false;
	is_ragdoll = false;
	hurt_event = false;
	needle_hit = false;
	player_viewed_sensor = false;
	player_previously_lost = false;
	initial_attack = false;
	see_player = false;
	player_cant_reach = false;
	cut_animation_done = false;
	take_animation_done = false;
	active = false;
	lost_player = false;

	null_node = false;
	player_out_navMesh=false;
	cut = false;

	ropeRef = CHandle();
	player_detected_pos = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	previous_point_search = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	slot_position = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	m_sensor = ((CEntity*)entity)->get<TCompSensorNeedles>();
	player_pos_sensor = ((CEntity*)entity)->get<TCompPlayerPosSensor>();
	tied_sensor = ((CEntity*)entity)->get<TCompSensorTied>();
	player_transform = ((CEntity*)player)->get<TCompTransform>();
	rol = role::UNASIGNATED;
	lastNumNeedlesViewed = 0;
	currentNumNeedlesViewed = 0;
	sensor_acum = 0.f;
	sensor_const = 1.f;
	attacked = false;
	is_reacheable = false;

	((TCompCharacterController*)character_controller)->lerpRotation = 0.08f;

	resetBot();
}

//Se mantiene en modo ragdoll durante un tiempo
int bt_grandma::actionRagdoll()
{
	TCompRagdoll* m_ragdoll = enemy_ragdoll;
	TCompLife* m_life = ((CEntity*)entity)->get<TCompLife>();

	if (on_enter) {
		stopAllAnimations();

		if (!m_ragdoll->isRagdollActive()) {
			m_ragdoll->setActive(true);

			if (m_life->life <= 0) {
				TCompRagdoll* m_ragdoll = enemy_ragdoll;
				TCompTransform* m_transform = own_transform;
				m_ragdoll->breakJoints();
				TCompAABB* ragdoll_aabb = (TCompAABB*)((CEntity*)entity)->get<TCompAABB>();
				XMVECTOR min = XMVectorSet(-50, -50, -50, 0);
				XMVECTOR max = XMVectorSet(50, 50, 50, 0);

				//Si esta atada, eliminamos el hilo antes de matar al enemigo
				/*if (((TCompSensorTied*)tied_sensor)->getTiedState()){
					if (ropeRef.isValid())
					CEntityManager::get().remove(CHandle(ropeRef).getOwner());
					}*/

				//CNav_mesh_manager::get().removeCapsule(((CEntity*)entity)->get<TCompColliderCapsule>());
				if (this->getRol() == role::ATTACKER)
					aimanager::get().RemoveEnemyAttacker(this);
				else
					aimanager::get().RemoveEnemyTaunt(this);

				//CEntityManager::get().remove(((CEntity*)entity)->get<TCompRigidBody>());
				//CEntityManager::get().remove(((CEntity*)entity)->get<TCompColliderCapsule>());

				CEntityManager::get().remove(((CEntity*)entity)->get<TCompCharacterController>());
				ragdoll_aabb->setIdentityMinMax(min, max);

				aimanager::get().removeBot(this->getId());
				//aimanager::get().removeGrandma(this->getId());

				CEntityManager::get().remove(((CEntity*)entity)->get<TCompBtGrandma>());

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

	//if (m_life->life <= 0) {
		//pos_final = XMVectorSet(10000, 10000, 10000, 0);
	//}

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
int bt_grandma::actionWakeUp()
{
	if (on_enter) {
		is_ragdoll = false;
		TCompRagdoll* m_ragdoll = enemy_ragdoll;
		TCompSkeleton* m_skeleton = enemy_skeleton;
		m_ragdoll->setActive(false);
		m_skeleton->playAnimation(17);

	}

	stopMovement();
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = last_look_direction;

	if (state_time > 1.8f) {
		return LEAVE;
	}
	else
		return STAY;

}


int bt_grandma::actionWaitSec()
{
	if (state_time <= max_time_tied)
		return STAY;
	else{
	    return LEAVE;
	}
}

//Corta todas las cuerdas a la que est atada
int bt_grandma::actionCutOwn()
{
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = last_look_direction;
	TCompRagdoll* m_ragdoll = enemy_ragdoll;

	if (on_enter) {
		if (!ropeRef.isValid()){
			tied_event = false;
			event_detected = false;
			return LEAVE;
		}else{
			// Ninja animation
			stopMovement();
			m_ragdoll->setActive(false);
			tied_event = false;
			event_detected = false;
			stopAllAnimations();
			resetTimeAnimation();
			playAnimationIfNotPlaying(12);
		}

	}

	if (state_time < getAnimationDuration(12)){
		return STAY;
	}else{
		TCompRope* rope = (TCompRope*)ropeRef;
		if (ropeRef.isValid()) {
			/*if (rope->joint_aux.getOwner().isValid())
				CEntityManager::get().remove(rope->joint_aux.getOwner());
			if (CHandle(ropeRef).getOwner().isValid())
				CEntityManager::get().remove(CHandle(ropeRef).getOwner());*/
			CRope_manager::get().removeString(ropeRef);
		}
		tied_event = false;
		event_detected = false;
		is_angry = true;
		have_to_warcry = true;
		is_ragdoll = false;
		return LEAVE;
	}
}

//
int bt_grandma::actionLeave()
{
	return LEAVE;
}

//Attack to the player when he is too close
int bt_grandma::actionTooCloseAttack()
{
	TCompTransform* p_transform = (TCompTransform*)player_transform;
	TCompTransform* m_transform = (TCompTransform*)own_transform;

	if (on_enter) {
		//Play close attack animation 
		if (m_transform->isInLeft(p_transform->position)){
			stopAllAnimations();
			resetTimeAnimation();
			playAnimationIfNotPlaying(3);
		}
		else{
			stopAllAnimations();
			resetTimeAnimation();
			playAnimationIfNotPlaying(4);
		}

	}

	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = last_look_direction;

	if (state_time >= getAnimationDuration(3)) {
		float distance = XMVectorGetX(XMVector3Length(p_transform->position - m_transform->position));
		if (distance <= max_distance_to_attack * 2){
			((CEntity*)player)->sendMsg(TActorHit(((CEntity*)player), 61000.f, false));
		}
		return LEAVE;
	}
	else
	{
		return STAY;
	}

}

//Go to the needle position (leave if cant reach)
int bt_grandma::actionChaseNeedlePosition()
{
	if (needle_is_valid){
		CHandle target_needle = ((TCompSensorNeedles*)m_sensor)->getNeedleAsociatedSensor(entity);

		if (target_needle.isValid()){
			TCompTransform* n_transform = ((CEntity*)target_needle.getOwner())->get<TCompTransform>();

			if (on_enter){
				time_searching_needle = 0.f;
				distance_old = 100.f;
				TCompCharacterController* m_char_controller = character_controller;

				m_char_controller->moveSpeedMultiplier = run_speed+0.3f;
				((TCompCharacterController*)character_controller)->lerpRotation = 0.15f;
				//m_char_controller->airSpeed = run_speed * 0.8f;

				stopAllAnimations();
				resetTimeAnimation();
				playAnimationIfNotPlaying(15);

				CNav_mesh_manager::get().findPath(((TCompTransform*)own_transform)->position, n_transform->position, path);
				if (path.size() > 0){
					if (V3DISTANCE((path[path.size() - 1]), n_transform->position) < max_dist_reach_needle){
						ind_path = 0;
						return STAY;
					}
					else{
						return LEAVE;
					}
				}
			}
			else{
				if (path.size() > 0){
					if (ind_path < path.size()){
						if (distance_new >= distance_old){
							time_searching_needle += CApp::get().delta_time;
						}
						if ((time_searching_needle > 15.f) || (state_time > 20.f)){
							time_searching_needle = 100.f;
							Citem_manager::get().removeNeedle(target_needle);
							return LEAVE;
						}
						if (V3DISTANCE((path[path.size() - 1]), n_transform->position) < max_dist_reach_needle){
							chasePoint(((TCompTransform*)own_transform), path[ind_path]);
							distance_new = V3DISTANCE(((TCompTransform*)own_transform)->position, path[ind_path]);
							if (distance_new < 1.0f){
								distance_old = distance_new;
								ind_path++;
								return STAY;
							}
							else{
								distance_old = distance_new;
								return STAY;
							}
						}
						else{
							//last_look_direction = look_direction;
							return LEAVE;
						}
					}
					else{
						//last_look_direction = look_direction;
						return LEAVE;
					}
				}
				else{
					//last_look_direction = look_direction;
					return LEAVE;
				}
			}
		}
		else{
			//last_look_direction = look_direction;
			return LEAVE;
		}
		return LEAVE;
	}
	needle_to_take = false;
	needle_is_valid = false;
	return LEAVE;
}

//Select the priority needle
int bt_grandma::actionSelectNeedleToTake()
{
	//bool sucess = (((TCompSensorNeedles*)m_sensor)->asociateGrandmaTargetNeedle(entity, max_dist_reach_needle));
	//if (sucess)
		needle_is_valid = true;
	return LEAVE;

}

//Cut the needles rope
int bt_grandma::actionCutRope()
{
	if (on_enter){
		cut = false;
		cut_animation_done = false;
		take_animation_done = false;
	}
	
	if ((!cut) && (!cut_animation_done)){
		stopAllAnimations();
		resetTimeAnimation();
		playAnimationIfNotPlaying(9);
		cut_animation_done = true;
	}

	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = last_look_direction;
	//stopMovement();

	float duration_cut = 1.0f;
	float duration_get_needle = 1.0f;

	CHandle target_needle = ((TCompSensorNeedles*)m_sensor)->getNeedleAsociatedSensor(entity);

	if (target_needle.isValid()){
		TCompTransform* n_transform = ((CEntity*)target_needle.getOwner())->get<TCompTransform>();
		TCompTransform* m_transform = own_transform;
		XMVECTOR dir = XMVector3Normalize(n_transform->position - m_transform->position);
		mov_direction = PxVec3(0, 0, 0);
		look_direction = Physics.XMVECTORToPxVec3(dir);

		stopMovement();

		// Exe the logic of cut the rope
		if ((state_time >= getAnimationDuration(9) * 0.7f) && (!cut)){
			CHandle target_rope = ((TCompSensorNeedles*)m_sensor)->getRopeAsociatedSensor(entity);
			if (target_rope.isValid()){
				CRope_manager::get().removeString(target_rope);
				//CEntityManager::get().remove(CHandle(target_rope).getOwner());
				cut = true;
			}
		}

		// Finish the animation
		if (state_time >= getAnimationDuration(9)) {
			if (!take_animation_done){
				stopAllAnimations();
				resetTimeAnimation();
				playAnimationIfNotPlaying(8);
				take_animation_done = true;
			}

			// Exe the logic of taking a needle
			if ((state_time >= ((getAnimationDuration(9) + getAnimationDuration(8))*0.6f))){ //&& !animation_done){			
				//CHandle target_needle = ((TCompSensorNeedles*)m_sensor)->getNeedleAsociatedSensor(entity);
				//if (target_needle.isValid()){
					((TCompSensorNeedles*)m_sensor)->removeNeedleRope(target_needle);
					if (CHandle(target_needle).getOwner().isValid()){
						CEntityManager::get().remove(CHandle(target_needle).getOwner());
						//animation_done = true;
					}
				//}
			}

			// When the animation finish, leave state and clean bools
			if (state_time >= getAnimationDuration(9) + getAnimationDuration(8)) {
				needle_to_take = false;
				needle_is_valid = false;
				cut = false;
				return LEAVE;
			}
		}
		return STAY;
	}

	return LEAVE;
}

//Take the needle
int bt_grandma::actionTakeNeedle()
{
	if (on_enter) {
		stopAllAnimations();
		resetTimeAnimation();
		playAnimationIfNotPlaying(8);
	}

	CHandle target_needle = ((TCompSensorNeedles*)m_sensor)->getNeedleAsociatedSensor(entity);

	if (target_needle.isValid()){
		TCompTransform* n_transform = ((CEntity*)target_needle.getOwner())->get<TCompTransform>();
		TCompTransform* m_transform = own_transform;
		XMVECTOR dir = XMVector3Normalize(n_transform->position - m_transform->position);
		
		look_direction = Physics.XMVECTORToPxVec3(dir);

		stopMovement();

		if (state_time >= getAnimationDuration(8)) {

			((TCompSensorNeedles*)m_sensor)->removeNeedleRope(target_needle);
			if (CHandle(target_needle).getOwner().isValid()){
				CEntityManager::get().remove(CHandle(target_needle).getOwner());
				needle_to_take = false;
				needle_is_valid = false;
			}
			return LEAVE;
		}
		else {
			return STAY;
		}
	}
	return LEAVE;
}

//Select the idle and play it
int bt_grandma::actionIdle()
{
	//TCompSkeleton* skeleton = ((CEntity*)entity)->get<TCompSkeleton>();

	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = last_look_direction;
	int anim = 0; 
	if (on_enter) {
		anim = getRandomNumber(0, 1);
		//stopAllAnimations();
		resetTimeAnimation();
		playAnimationIfNotPlaying(anim);
	}
	stopMovement();
	if (state_time >= 2){
		return LEAVE;

	}
	else{
		return STAY;
	}

}

//Select a point to go 
int bt_grandma::actionSearchPoint()
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
int bt_grandma::actionWander()
{
	if (on_enter) {
		stopAllAnimations();
		resetTimeAnimation();
		playAnimationIfNotPlaying(2);

		TCompCharacterController* m_char_controller = character_controller;

		m_char_controller->moveSpeedMultiplier = walk_speed;
		((TCompCharacterController*)character_controller)->lerpRotation = 0.11f;
		//m_char_controller->airSpeed = walk_speed * 0.8f;

	}

	jump = false;
	//Tratamos de evitar cambios demasiado repentinos de ruta
	if (on_enter){
		((TCompSkeleton*)(((CEntity*)entity)->get<TCompSkeleton>()))->loopAnimation(2);
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
		}else{
			//last_look_direction = look_direction;
			return LEAVE;
		}
	}
	else{
		return LEAVE;
	}
}

//Makes a warcry
int bt_grandma::actionWarcry()
{
	if (on_enter) {
		stopAllAnimations();
		resetTimeAnimation();
		playAnimationIfNotPlaying(19);
	}

	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = last_look_direction;
	stopMovement();


	if (state_time >= getAnimationDuration(19)) {
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
int bt_grandma::actionPlayerAlert()
{
	if (on_enter) {
		resetTimeAnimation();
		playAnimationIfNotPlaying(18);
	}

	TCompTransform* p_transform = player_transform;
	TCompTransform* m_transform = own_transform;
	XMVECTOR dir = XMVector3Normalize(p_transform->position - m_transform->position);
	mov_direction = PxVec3(0, 0, 0);
	look_direction = Physics.XMVECTORToPxVec3(dir);
	((TCompCharacterController*)character_controller)->Move(mov_direction, false, jump, look_direction);

	if (state_time > getAnimationDuration(18)) {
		//Call the iaManager method for warning the rest of the grandmas
		aimanager::get().warningToClose(this, 10.f, player_transform);
		return LEAVE;
	}
	else
		return STAY;
}

//Leave the angry state, go to peacefull
int bt_grandma::actionCalmDown()
{
	is_angry = false;
	time_searching_player = 0;
	return LEAVE;
}

//Search random point around the last place where the player was saw
int bt_grandma::actionSearchArroundLastPoint()
{
	rand_point = CNav_mesh_manager::get().getRandomNavMeshPoint(last_point_player_saw, radius, ((TCompTransform*)own_transform)->position);

	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = last_look_direction;
	stopMovement();

	return LEAVE;

}

//plays the looking for player animation
int bt_grandma::actionLookingFor(){
	if (on_enter) {
		stopAllAnimations();
		resetTimeAnimation();
		playAnimationIfNotPlaying(21);
	}

	time_searching_player += CApp::get().delta_time;
	stopMovement();

	if (state_time > getAnimationDuration(21)*2.f)
		return LEAVE;
	else
		return STAY;
}

//look the player around the his last point
int bt_grandma::actionLookAround()
{
	float aux_time = state_time;
	bool aux_on_enter = on_enter;
	jump = false;
	time_searching_player += CApp::get().delta_time;
	//Tratamos de evitar cambios demasiado repentinos de ruta
	if (on_enter){

		TCompCharacterController* m_char_controller = character_controller;

		m_char_controller->moveSpeedMultiplier = run_speed;
		((TCompCharacterController*)character_controller)->lerpRotation = 0.13f;
		//m_char_controller->airSpeed = run_speed * 0.8f;
		stopAllAnimations();
		resetTimeAnimation();
		playAnimationIfNotPlaying(15);

		CNav_mesh_manager::get().findPath(((TCompTransform*)own_transform)->position, rand_point, path);
		find_path_time = state_time;
		ind_path = 0;

		//Aqui tendría que borrar el enemigo de los slots del aiManager para su posterior re-asignacion
		aimanager::get().RemoveEnemyAttacker(this);
		//----------------------------------------------------------------------------------------------
	}
	/*else{
		if ((state_time - find_path_time) > 1.f){
			CNav_mesh_manager::get().findPath(((TCompTransform*)own_transform)->position, rand_point, path);
			find_path_time = state_time;
		}
	}*/

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
			last_look_direction = look_direction;
			return LEAVE;
		}
	}
	else{
		return LEAVE;
	}
}

//Takes a roll, attacker or taunter and a poisition to go
int bt_grandma::actionSelectRole()
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


//Go to his position
int bt_grandma::actionChaseRoleDistance()
{
	TCompTransform* m_transform = own_transform;
	TCompTransform* p_transform = player_transform;
	CNav_mesh_manager::get().findPath(m_transform->position, p_transform->position, path);
	if (on_enter) {
		if (path.size() > 0){

			float distance = V3DISTANCE(p_transform->position, path[path.size() - 1]);
			if (distance>2.f){
				player_out_navMesh = true;
				playAnimationIfNotPlaying(0);
				return LEAVE;
			}
			stopAllAnimations();
			resetTimeAnimation();
			playAnimationIfNotPlaying(16);

			TCompCharacterController* m_char_controller = character_controller;

			m_char_controller->moveSpeedMultiplier = run_angry_speed;
			m_char_controller->airSpeed = run_angry_speed * 0.8f;
			((TCompCharacterController*)character_controller)->lerpRotation = 0.11f;
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

	//Go to situate
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
int bt_grandma::actionInitialAttack()
{
	TCompTransform* p_transform = player_transform;
	TCompTransform* m_transform = own_transform;
	XMVECTOR dir = XMVector3Normalize(p_transform->position - m_transform->position);
	//m_char_controller->airSpeed = run_angry_speed * 0.8f;

	if (on_enter) {
		((TCompCharacterController*)character_controller)->moveSpeedMultiplier = 2.7f;
		initial_attack = true;
		stopAllAnimations();
		resetTimeAnimation();
		playAnimationIfNotPlaying(20);
		attacked = false;
		mov_direction = Physics.XMVECTORToPxVec3(dir);
		((TCompCharacterController*)character_controller)->Move(mov_direction, false, jump, mov_direction);
	}

	float attack_time = 0.466;
	float distance = XMVectorGetX(XMVector3Length(p_transform->position - m_transform->position));

	// Check if the attack reach the player at the given time
	if (state_time > attack_time && attacked == false && distance <= 2.1f) {
		
		XMVECTOR attack_direction_path = (p_transform->position - m_transform->position);
		attack_direction_path = XMVector3Normalize(attack_direction_path);
		XMVECTOR front_path = XMVector3Normalize(m_transform->getFront());
		XMVECTOR dir_path = XMVector3AngleBetweenVectors(attack_direction_path, front_path);
		float rads_path = XMVectorGetX(dir_path);
		float angle_deg_path = rad2deg(rads_path);
		if (angle_deg_path < 40.f){
			XMVECTOR particles_pos = p_transform->position - attack_direction_path * 0.5f + XMVectorSet(0, 1, 0, 0);
			CHandle particle_entity = CLogicManager::get().instantiateParticleGroupOneShot(particle_name_initial_hit, particles_pos);
			((CEntity*)player)->sendMsg(TActorHit(((CEntity*)player), 61000.f, false));
			attacked = true;

			// Sound
			CSoundManager::get().playEvent("GRANDMA_HIT", m_transform->position);
		}
		else{
			last_time = timer;
			attacked = true;

			// MISS
			CSoundManager::get().playEvent("GRANDMA_MISS", m_transform->position);
			attacked = true;
		}
			
	}

	if (state_time >= getAnimationDuration(20)){
		last_time = timer;
		return LEAVE;
	}
	else{
		return STAY;
	}
}

//Move step by step to the roll position (leave on reach or lost)
int bt_grandma::actionSituate()
{
	TCompTransform* m_transform = own_transform;
	TCompTransform* p_transform = player_transform;

	wander_target = p_transform->position;// + slot_position;
	CNav_mesh_manager::get().findPath(m_transform->position, wander_target, path);
	if (on_enter) {
		if (path.size() > 0){
			TCompCharacterController* m_char_controller = character_controller;

			/*float distance = V3DISTANCE(p_transform->position, path[path.size() - 1]);
			if (distance>2.f){
				player_out_navMesh = true;
				playAnimationIfNotPlaying(0);
				return LEAVE;
			}*/

			m_char_controller->moveSpeedMultiplier = run_angry_speed;
			m_char_controller->airSpeed = run_angry_speed * 0.8f;
			stopAllAnimations();
			resetTimeAnimation();
			playAnimationIfNotPlaying(16);
		}else{
			player_out_navMesh = true;
			playAnimationIfNotPlaying(0);
			return LEAVE;
		}
	}

	if (on_enter){
		ind_path = 0;
	}

	float distance_path = V3DISTANCE(p_transform->position, path[path.size() - 1]);
	if (distance_path>2.f){
		player_out_navMesh = true;
		playAnimationIfNotPlaying(0);
		return LEAVE;
	}

	float distance = V3DISTANCE(m_transform->position, wander_target);
	if (!initial_attack){
		if (distance < 3.4f){
			return LEAVE;
		}
	}else{
		if (distance < 1.5f){
			return LEAVE;
		}
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
int bt_grandma::actionNormalAttack()
{
	if (on_enter) {
		attacked = false;
		int anim = getRandomNumber(5, 7);
		stopAllAnimations();
		resetTimeAnimation();
		playAnimationIfNotPlaying(anim);
	}

	TCompTransform* p_transform = player_transform;
	TCompTransform* m_transform = own_transform;
	XMVECTOR dir = XMVector3Normalize(p_transform->position - m_transform->position);
	look_direction = Physics.XMVECTORToPxVec3(dir);
	stopMovement();
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = Physics.XMVECTORToPxVec3(dir);

	// Anim attack 1 & 2 has the first attack at 0.5 seconds, the attack 3 animation at 0.33
	float attack_time = 0.5f;
	if (last_anim_id == 7)
		attack_time = 0.33f;

	if (state_time >= attack_time && attacked == false) {

		// Check if the attack reach the player
		float distance = XMVectorGetX(XMVector3Length(p_transform->position - m_transform->position));
		if (distance <= max_distance_to_attack)
		{
			// Impact particle
			XMVECTOR particles_pos = p_transform->position - dir * 0.5f + XMVectorSet(0, 1, 0, 0);
			CHandle particle_entity = CLogicManager::get().instantiateParticleGroupOneShot(particle_name_initial_hit, particles_pos);

			((CEntity*)player)->sendMsg(TActorHit(((CEntity*)player), 61000.f, false));
			attacked = true;

			// Sound
			CSoundManager::get().playEvent("GRANDMA_HIT", m_transform->position);
		}
		else {
			// MISS
			CSoundManager::get().playEvent("GRANDMA_MISS", m_transform->position);
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
int bt_grandma::actionIdleWar()
{

	if (on_enter) {
		stopAllAnimations();
		resetTimeAnimation();
		playAnimationIfNotPlaying(11);
	}

	TCompTransform* p_transform = player_transform;
	TCompTransform* m_transform = own_transform;
	XMVECTOR dir = XMVector3Normalize(p_transform->position - m_transform->position);
	look_direction = Physics.XMVECTORToPxVec3(dir);
	stopMovement();
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = Physics.XMVECTORToPxVec3(dir);

	if (state_time > getAnimationDuration(11))
		return LEAVE;
	else
		return STAY;
}

//Play a taunter routine
int bt_grandma::actionTaunter()
{
	//Meter animacion
	if (on_enter) {
		stopAllAnimations();
		resetTimeAnimation();
		playAnimationIfNotPlaying(18);
	}

	TCompTransform* p_transform = player_transform;
	TCompTransform* m_transform = own_transform;
	XMVECTOR dir = XMVector3Normalize(p_transform->position - m_transform->position);
	stopMovement();
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = Physics.XMVECTORToPxVec3(dir);

	if (state_time > getAnimationDuration(18))
		return LEAVE;
	else
		return STAY;
}

//Calculate if hurts or ragdoll, if ragdoll then clean all events (los events solo tocan su flag, excepto el ragdoll)
int bt_grandma::actionHurtEvent()
{
if (on_enter) {
		playAnimationIfNotPlaying(10);
	}

	stopMovement();
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = Physics.XMVECTORToPxVec3(dir);

	if (state_time > getAnimationDuration(10)) {
		//Call the iaManager method for warning the rest of the grandmas
		aimanager::get().warningPlayerFound(this);
		event_detected = false;
		return LEAVE;
	}
	else
		return STAY;

}

int bt_grandma::actionNeedleHit(){
	if (on_enter) {
		stopAllAnimations();
		resetTimeAnimation();
		playAnimationIfNotPlaying(10);
	}

	TCompTransform* p_transform = player_transform;
	TCompTransform* m_transform = own_transform;
	XMVECTOR dir = XMVector3Normalize(p_transform->position - m_transform->position);
	stopMovement();
	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = Physics.XMVECTORToPxVec3(dir);

	if (state_time > getAnimationDuration(10)) {
		//Call the iaManager method for warning the rest of the grandmas
		aimanager::get().warningPlayerFound(this);
		event_detected = false;
		return LEAVE;
	}
	else
		return STAY;
}

//
int bt_grandma::actionNeedleAppearsEvent()
{
	currentNumNeedlesViewed = (unsigned int)((TCompSensorNeedles*)m_sensor)->getNumNeedles(entity, max_dist_reach_needle);//list_needles.size();
	if (currentNumNeedlesViewed > lastNumNeedlesViewed){
		if (current != NULL){
			if ((current->getTypeInter() == EXTERNAL) || (current->getTypeInter() == BOTH)){
				setCurrent(NULL);
				lastNumNeedlesViewed = currentNumNeedlesViewed;
				return LEAVE;
			}
		}else{
			lastNumNeedlesViewed = currentNumNeedlesViewed;
			return LEAVE;
		}
	}
	else{
		return LEAVE;
	}
	return LEAVE;
}

int bt_grandma::actionTiedEvent()
{

	//mov_direction = PxVec3(0, 0, 0);
	//look_direction = last_look_direction;
	stopMovement();

	if (on_enter) {

		if (!ropeRef.isValid()){
			tied_event = false;
			event_detected = false;
			return LEAVE;
		}
		else{
			//Plays the cut own string animation
			//int dice = getRandomNumber(0, 10);
			//if (dice < max_bf_posibilities){
				// Ninja animation
			stopAllAnimations();
			resetTimeAnimation();
			playAnimationIfNotPlaying(12);
				//tied_event = false;
				//event_detected = false;
			/*}
			else{
				tied_event = false;
				event_detected = false;
				tied_succesfull = true;
				return LEAVE;*/
			//}
		}

	}

	if (state_time >= getAnimationDuration(12)/5){
		if (ropeRef.isValid()){
			TCompRope* rope = (TCompRope*)ropeRef;
			CRope_manager::get().removeString(ropeRef);
		}

		if (state_time >= getAnimationDuration(12)){
			tied_event = false;
			event_detected = false;
			if (!is_angry){
				is_angry = true;
				have_to_warcry = true;
			}
			else{
				is_angry = true;
				have_to_warcry = false;
			}
			return LEAVE;
		}else{
			return STAY;
		}
	}
	else{
		return STAY;
	}
}

//Keeps in falling state till ti
int bt_grandma::actionFallingEvent()
{
	return LEAVE;
}

//
int bt_grandma::actionGetAngry()
{
	is_angry = true;
	return LEAVE;
}

//Puts are_events var to false
int bt_grandma::actionNoevents()
{
	return LEAVE;
}

//
int bt_grandma::conditionTied()
{
	return false;
}

//
int bt_grandma::conditionis_ragdoll()
{
	return is_ragdoll;
}

//
int bt_grandma::conditionis_grounded()
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
int bt_grandma::conditiontrue()
{
	return true;
}

//
int bt_grandma::conditionis_angry()
{
	return is_angry;
}

//Check if the player is close enought for an annoying attack
int bt_grandma::conditiontoo_close_attack()
{

	if ((V3DISTANCE(((TCompTransform*)own_transform)->position, ((TCompTransform*)player_transform)->position) < max_dist_close_attack) && ((timer - last_time) >= delta_time_close_attack)){
		last_time = timer;
		return true;
	}
	else{
		return false;
	}
	return false;
}

//Check if there is a needle to take
int bt_grandma::conditionneedle_to_take()
{
	//currentNumNeedlesViewed = (unsigned int)((TCompSensorNeedles*)m_sensor)->getNumNeedles(entity, max_dist_reach_needle);
	if (currentNumNeedlesViewed > 0){
		needle_to_take = true;
	}else{
		needle_to_take = false;
	}

	return needle_to_take;
}

//
int bt_grandma::conditionis_needle_tied()
{
	CHandle target_rope = ((TCompSensorNeedles*)m_sensor)->getRopeAsociatedSensor(entity);
	if (target_rope.isValid()){
		return true;
	}
	else{
		return false;
	}
	return false;
}

//Check if is necesary a warcry
int bt_grandma::conditionhave_to_warcry()
{
	return have_to_warcry;
}

//Check if there player is not visible for any grandma (and reach the last position)
int bt_grandma::conditionplayer_lost()
{

	if (!player_out_navMesh){
		if ((last_time_player_saw) > max_time_player_lost){
			player_previously_lost = true;
			initial_attack = false;
			return true;
		}
	}else{
		player_out_navMesh = false;
		return true;
	}
	return false;

}

//check if the player is visible
int bt_grandma::conditionsee_player()
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
int bt_grandma::conditionLook_for_timeout()
{
	if (time_searching_player > max_time_player_search){
		return true;
	}
	else{
		return false;
	}

	//return Look_for_timeout;
}

int bt_grandma::conditionLook_time(){
	if (time_searching_player <= max_time_player_search){
		lost_player=true;
	}
	else{
		lost_player=false;
	}
	return lost_player;
}

//Check if the role is attacker and is close enought
int bt_grandma::conditionis_attacker()
{
	TCompTransform* m_transform = own_transform;
	TCompTransform* p_transform = player_transform;

	float distance = V3DISTANCE(m_transform->position, p_transform->position); //+ slot_position);
	if ((rol == role::ATTACKER) && (V3DISTANCE(m_transform->position, p_transform->position) <= 4.5f)){
		return true;
	}
	else{
		return false;
	}
}

//Check if the player is in range and cold down time passed
int bt_grandma::conditionnormal_attack()
{
	TCompTransform* m_transform = own_transform;
	TCompTransform* p_transform = player_transform;

	float random_time_attack = getRandomNumber(delta_time_close_attack - 2.8f, delta_time_close_attack);
	if (((V3DISTANCE(m_transform->position, p_transform->position) < 2.5f) && (timer - last_time) >= random_time_attack)){
		last_time = timer;
		return true;
	}
	else{
		return false;
	}
}

//Init on false
int bt_grandma::conditionare_events()
{
	return event_detected;
	//return are_events;
}

//Check if is a hurt event
int bt_grandma::conditionhurt_event()
{
	return hurt_event;
}

//Check if is a needle hit event
int bt_grandma::conditionneedle_hit_event(){
	return needle_hit;
}

//Check if is a falling event
int bt_grandma::conditionfalling_event()
{
	return false;
	//return falling_event;
}

//Check if is a tied event
int bt_grandma::conditiontied_event()
{
	//return false;
	return tied_event;
}

//Check if can reach the selected needle
int bt_grandma::conditioncan_reach_needle()
{
	//XASSERT(needle_objective->needleRef.isValid(), "Invalid needle");
	if (needle_is_valid){
		CHandle target_needle = ((TCompSensorNeedles*)m_sensor)->getNeedleAsociatedSensor(entity);
		if (target_needle.isValid()){
			TCompTransform* e_transform = ((CEntity*)target_needle.getOwner())->get<TCompTransform>();

			wander_target = e_transform->position;

			float distance_prueba = V3DISTANCE(wander_target, ((TCompTransform*)own_transform)->position);

			if (V3DISTANCE(wander_target, ((TCompTransform*)own_transform)->position) <= max_dist_to_needle){
				return true;
			}
			else{
				return false;
			}
		}else{
			return false;
		}
	}
	else{
		return false;
	}

	return false;
}

//Check if the role is taunter and is close enought
int bt_grandma::conditionis_taunter()
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
int bt_grandma::conditioninitial_attack()
{
	TCompTransform* m_transform = own_transform;
	TCompTransform* p_transform = player_transform;

	if (path.size() > 0) {

		XMVECTOR attack_direction_path = (path[path.size() - 1] - m_transform->position);
		attack_direction_path = XMVector3Normalize(attack_direction_path);
		XMVECTOR front_path = XMVector3Normalize(m_transform->getFront());
		XMVECTOR dir_path = XMVector3AngleBetweenVectors(attack_direction_path, front_path);
		float rads_path = XMVectorGetX(dir_path);
		float angle_deg_path = rad2deg(rads_path);

		float distance = V3DISTANCE(m_transform->position, p_transform->position);
		if ((!initial_attack) && ((distance < 3.4f))){
			if (angle_deg_path < 30.f)
				return true;
			else{
				initial_attack = true;
				return false;
			}
		}
		else{
			return false;
		}
	}
	return false;
}

//Check if it is too far from the target position
int bt_grandma::conditionfar_from_target_pos()
{
	TCompTransform* m_transform = own_transform;
	TCompTransform* p_transform = player_transform;

	XMVECTOR target = p_transform->position; //+ slot_position;

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
void bt_grandma::playerViewedSensor(){
	if (!player_viewed_sensor){

		bool tri = ((TCompPlayerPosSensor*)player_pos_sensor)->playerInRange();
		if (((TCompPlayerPosSensor*)player_pos_sensor)->playerInRange()) {
			if (current != NULL){
				if ((current->getTypeInter() == EXTERNAL) || (current->getTypeInter() == BOTH)){
					setCurrent(NULL);
					player_viewed_sensor = true;
				}
			}
		}
		else{
			player_viewed_sensor = false;
		}
	}
	else{
		last_time_player_saw += CApp::get().delta_time;
	}
}

// Sensor para detectar si el enemigo ve alguna aguja
void bt_grandma::needleViewedSensor(){

	//--------------------------------------------------------------------------------------------------
	/*NOTA: Debería solo ejecutar tanto este sensor como el de player position mientras no haya eventos*/
	//--------------------------------------------------------------------------------------------------

	//componente sensor de agujas del enemigo
	//m_sensor->getNeedlesInRange();

	//if (!needle_to_take){
	currentNumNeedlesViewed = (unsigned int)((TCompSensorNeedles*)m_sensor)->getNumNeedles(entity, max_dist_reach_needle);//list_needles.size();
	//dbg("currentNumNeedlesViewed: %u", currentNumNeedlesViewed);
	//dbg("lastNumNeedlesViewed: %u", lastNumNeedlesViewed);
	if (currentNumNeedlesViewed > lastNumNeedlesViewed){
		//Si hay variacion reseteamos comprobamos si el nodo es interrumpible
		//Hay que excluir el nodo root, puesto que no incluye niveles de interrupción
		if (current != NULL){
			if ((current->getTypeInter() == EXTERNAL) || (current->getTypeInter() == BOTH)){
				//TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();
				//m_sensor->asociateGrandmaTargetNeedle(entity);
				//needle_to_take = true;
				setCurrent(NULL);
			}
		}else{
			setCurrent(NULL);
		}
	}
	lastNumNeedlesViewed = currentNumNeedlesViewed;
	//}
	//}
}

void bt_grandma::tiedSensor(){
	((TCompSensorTied*)tied_sensor)->keepTied();
	if (tied_sensor.isValid()){
		if (!tied_event){
			if (((TCompSensorTied*)tied_sensor)->getTiedState()){
				ropeRef = (TCompRope*)((TCompSensorTied*)tied_sensor)->getRopeRef();
				if ((current) && ((current->getTypeInter() == EXTERNAL))){
					setCurrent(NULL);
					tied_event = true;
					event_detected = true;
				}
			}
		}
	}
}

void bt_grandma::hurtSensor(float damage){

	if (damage >= force_large_impact){
		TCompLife* life = ((CEntity*)entity)->get<TCompLife>();
		life->life = 0;
		stopAllAnimations();
		is_ragdoll = true;
		TCompTransform* m_transform = own_transform;
		CHandle particle_entity = CLogicManager::get().instantiateParticleGroup(particle_name_dismemberment, m_transform->position, m_transform->rotation);
		TCompParticleGroup* pg = ((CEntity*)particle_entity)->get<TCompParticleGroup>();
		pg->destroy_on_death = true;
		setCurrent(NULL);
		((TCompSensorNeedles*)m_sensor)->desAsociateNeedle(entity);
	}
	else if ((damage >= force_medium_impact) && (damage < force_large_impact)){
		stopAllAnimations();
		is_ragdoll = true;
		setCurrent(NULL);
		BeAngry();
	}
	else if (damage < force_medium_impact){
		hurt_event = true;
		BeAngry();
	}
}

void bt_grandma::BeAngry(){
	if (!is_angry){
		have_to_warcry = true;
		is_angry = true;
	}
}

void bt_grandma::WarWarningSensor(XMVECTOR player_position){
	is_angry = true;
	have_to_warcry = false;
	last_time_player_saw = 0;
	lost_player = false;
	player_detected_pos = player_position;
	setCurrent(NULL);
}

void bt_grandma::PlayerFoundSensor(){

	last_time_player_saw = 0;
	lost_player = false;
	//is_angry = true;
	setCurrent(NULL);
}
/*void bt_grandma::PlayerTouchSensor(bool touch){
	player_touch = touch;
	//setCurrent(NULL);
}*/

void bt_grandma::update(float elapsed){

	if (active){
		//dbg("grandma_pr: %u", entity.asUnsigned());

		sensor_acum += elapsed;

		//dbg("sensor_acum: %f", sensor_acum);
		//dbg("sensor_delay: %f", sensor_delay);
		if (sensor_delay <= sensor_acum)
		{
			needleViewedSensor();
			sensor_acum = 0;
		}


		playerViewedSensor();
		findLostPlayer();
		tiedSensor();
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
	}else{
		resetBot();
	}
}

bool bt_grandma::trueEveryXSeconds(float time)
{
	static float counter = 0;
	counter += CApp::get().delta_time;
	if (counter >= time) {
		counter = 0;
		return true;
	}
	return false;
}

void bt_grandma::chasePoint(TCompTransform* own_position, XMVECTOR chase_point){
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

CHandle bt_grandma::getPlayerTransform(){
	return player_transform;
}

void bt_grandma::findLostPlayer(){
	if (lost_player){
		if (findPlayer()){
			lost_player = false;
			player_previously_lost = true;
			setCurrent(NULL);
		}
	}
}

bool bt_grandma::findPlayer(){
	if (((TCompPlayerPosSensor*)player_pos_sensor)->playerInRange()) {
		return true;
	}
	else{
		return false;
	}
}

bool bt_grandma::isAngry(){
	return is_angry;
}

void bt_grandma::setRol(int r){
	if (r == 1)
		rol = role::ATTACKER;
	else if (r == 2)
		rol = role::TAUNTER;
}

void bt_grandma::setAttackerSlot(int s){
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

int bt_grandma::getAttackerSlot(){
	return slot;
}

int bt_grandma::getRol(){
	return rol;
}

float bt_grandma::getDistanceToPlayer(){
	if (own_transform.isValid())
		return V3DISTANCE(((TCompTransform*)own_transform)->position, ((TCompTransform*)player_transform)->position);
	else
		return 0.f;
}

int bt_grandma::getNearestSlot(bool free_north, bool free_east, bool free_west){
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


void bt_grandma::drawdebug() {
	font.print3D(wander_target, "Destino");
	if (slot == attacker_slots::NORTH)
		font.print3D(wander_target + XMVectorSet(0.f, 0.5f, 0.f, 0.f), "NORTH");
	if (slot == attacker_slots::WEST)
		font.print3D(wander_target + XMVectorSet(0.f, 0.5f, 0.f, 0.f), "WEST");
	if (slot == attacker_slots::EAST)
		font.print3D(wander_target + XMVectorSet(0.f, 0.5f, 0.f, 0.f), "EAST");
}

void bt_grandma::stopAllAnimations() {
	TCompSkeleton* m_skeleton = enemy_skeleton;

	for (int i = 0; i < 50; ++i) {
		m_skeleton->model->getMixer()->clearCycle(i, 0.3f);
	}
}

void bt_grandma::resetTimeAnimation(){
	TCompSkeleton* m_skeleton = enemy_skeleton;
	m_skeleton->resetAnimationTime();
}

void bt_grandma::playAnimationIfNotPlaying(int id) {
	TCompSkeleton* m_skeleton = enemy_skeleton;

	if (id != last_anim_id) {
		stopAnimation(last_anim_id);
		last_anim_id = id;
		m_skeleton->loopAnimation(id);
	}
}

void bt_grandma::stopAnimation(int id) {
	TCompSkeleton* m_skeleton = enemy_skeleton;
	m_skeleton->stopAnimation(id);
}

float bt_grandma::getAnimationDuration(int id) {
	TCompSkeleton* m_skeleton = enemy_skeleton;

	float res = m_skeleton->model->getMixer()->getAnimationDuration();
	return res;
}

void bt_grandma::setActive(bool act){
	active = act;
}

void bt_grandma::setIndRecastAABB(int ind){
	ind_recast_aabb = ind;
}

int bt_grandma::getIndRecastAABB(){
	return ind_recast_aabb;
}

void bt_grandma::stopMovement(){
	mov_direction = PxVec3(0, 0, 0);
	((TCompCharacterController*)character_controller)->Move(mov_direction, false, false, look_direction);
}

void bt_grandma::resetBot(){
	setCurrent(NULL);
	playAnimationIfNotPlaying(0);
	stopMovement();
	//mov_direction = PxVec3(0, 0, 0);
	//((TCompCharacterController*)character_controller)->Move(mov_direction, false, false, look_direction);
}