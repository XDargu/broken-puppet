#include "mcv_platform.h"
#include "bt_grandma.h"
#include "../entity_manager.h"
#include "../components/all_components.h"
#include "utils.h"
#include "nav_mesh_manager.h"
#include "components\comp_skeleton.h"

//Constants
const int max_bf_posibilities = 7;
const float max_dist_reach_needle = 1.7f;
const float max_dist_close_attack = 1.7f;
const float max_time_player_lost = 2.f;
const float delta_time_close_attack = 1.3f;
const float distance_change_way_point = 0.3f;
const float force_large_impact = 10000.f;
const float force_medium_impact = 6500.f;

void bt_grandma::create(string s)
{
	name = s;
	createRoot("Root", PRIORITY, NULL, NULL);
	addChild("Root", "Ragdoll", SEQUENCE, (btcondition)&bt_grandma::conditionis_ragdoll, NULL);
	addChild("Ragdoll", "ActionRagdoll1", ACTION, NULL, (btaction)&bt_grandma::actionRagdoll);
	addChild("Ragdoll", "Awake", PRIORITY, NULL, NULL);
	addChild("Awake", "WakeUp", SEQUENCE, (btcondition)&bt_grandma::conditionTied, NULL);
	addChild("WakeUp", "GroundedTied", PRIORITY, NULL, NULL);
	addChild("GroundedTied", "ActionWakeUp2", ACTION, (btcondition)&bt_grandma::conditionis_grounded, (btaction)&bt_grandma::actionWakeUp);
	addChild("GroundedTied", "CutOwnSec", SEQUENCE, (btcondition)&bt_grandma::conditiontrue, NULL);
	addChild("CutOwnSec", "WaitTied", SEQUENCE, NULL, NULL);																				//Este estaba en undefined 
	addChild("WakeUp", "CutOwn3", ACTION, NULL, (btaction)&bt_grandma::actionCutOwn);
	addChild("Awake", "Grounded", PRIORITY, (btcondition)&bt_grandma::conditiontrue, NULL);
	addChild("Grounded", "ActionWakeUp4", ACTION, (btcondition)&bt_grandma::conditionis_grounded, (btaction)&bt_grandma::actionWakeUp);
	addChild("Grounded", "Leave5", ACTION, (btcondition)&bt_grandma::conditiontrue, (btaction)&bt_grandma::actionLeave);
	addChild("Ragdoll", "GetAngry6", ACTION, NULL, (btaction)&bt_grandma::actionGetAngry);
	addChild("Root", "events", PRIORITY, (btcondition)&bt_grandma::conditionare_events, NULL);
	addChild("events", "HurtEvent7", ACTION, (btcondition)&bt_grandma::conditionhurt_event, (btaction)&bt_grandma::actionHurtEvent);
	addChild("events", "FallingEvent8", ACTION, (btcondition)&bt_grandma::conditionfalling_event, (btaction)&bt_grandma::actionFallingEvent);
	addChild("events", "TiedEvent9", ACTION, (btcondition)&bt_grandma::conditiontied_event, (btaction)&bt_grandma::actionTiedEvent);
	addChild("events", "No events10", ACTION, (btcondition)&bt_grandma::conditiontrue, (btaction)&bt_grandma::actionNoevents);
	addChild("Root", "Angry", PRIORITY, (btcondition)&bt_grandma::conditionis_angry, NULL);
	addChild("Angry", "Warcry11", ACTION, (btcondition)&bt_grandma::conditionhave_to_warcry, (btaction)&bt_grandma::actionWarcry);
	addChild("Angry", "LookForPlayer", PRIORITY, (btcondition)&bt_grandma::conditionplayer_lost, NULL);
	addChild("LookForPlayer", "PlayerAlert12", ACTION, (btcondition)&bt_grandma::conditionsee_player, (btaction)&bt_grandma::actionPlayerAlert);
	addChild("LookForPlayer", "CalmDown13", ACTION, (btcondition)&bt_grandma::conditionLook_for_timeout, (btaction)&bt_grandma::actionCalmDown);
	addChild("LookForPlayer", "LookAround14", ACTION, (btcondition)&bt_grandma::conditiontrue, (btaction)&bt_grandma::actionLookAround);
	addChild("Angry", "TryAttack", SEQUENCE, (btcondition)&bt_grandma::conditiontrue, NULL);
	addChild("TryAttack", "SelectRole15", ACTION, NULL, (btaction)&bt_grandma::actionSelectRole);
	addChild("TryAttack", "ExecuteRole", PRIORITY, NULL, NULL);
	addChild("ExecuteRole", "AttackRoutine", PRIORITY, (btcondition)&bt_grandma::conditionis_attacker, NULL);
	addChild("AttackRoutine", "InitialAttack16", ACTION, (btcondition)&bt_grandma::conditioninitial_attack, (btaction)&bt_grandma::actionInitialAttack);
	addChild("AttackRoutine", "NormalAttack17", ACTION, (btcondition)&bt_grandma::conditionnormal_attack, (btaction)&bt_grandma::actionNormalAttack);
	addChild("AttackRoutine", "Situate18", ACTION, (btcondition)&bt_grandma::conditionfar_from_target_pos, (btaction)&bt_grandma::actionSituate);
	addChild("AttackRoutine", "IdleWa19r", ACTION, (btcondition)&bt_grandma::conditiontrue, (btaction)&bt_grandma::actionIdleWar);
	addChild("ExecuteRole", "Taunter", PRIORITY, (btcondition)&bt_grandma::conditionis_taunter, NULL);
	addChild("Taunter", "Situate20", ACTION, (btcondition)&bt_grandma::conditionfar_from_target_pos, (btaction)&bt_grandma::actionSituate);
	addChild("Taunter", "Taunter21", ACTION, NULL, (btaction)&bt_grandma::actionTaunter);
	
	addChild("ExecuteRole", "ChaseRoleDistance22", ACTION, (btcondition)&bt_grandma::conditiontrue, (btaction)&bt_grandma::actionChaseRoleDistance);
	
	addChild("Root", "Peacefull", PRIORITY, (btcondition)&bt_grandma::conditiontrue, NULL);
	addChild("Peacefull", "XSecAttack", SEQUENCE, (btcondition)&bt_grandma::conditiontoo_close_attack, NULL);
	addChild("XSecAttack", "TooCloseAttack23", ACTION, NULL, (btaction)&bt_grandma::actionTooCloseAttack);
	addChild("Peacefull", "TakeNeedle", SEQUENCE, (btcondition)&bt_grandma::conditionneedle_to_take, NULL);
	addChild("TakeNeedle", "XSecsNeedle", SEQUENCE, NULL, NULL);
	addChild("XSecsNeedle", "NeedleAppearsEvent24", ACTION, NULL, (btaction)&bt_grandma::actionNeedleAppearsEvent);
	addChild("TakeNeedle", "SelectNeedleToTake25", ACTION, NULL, (btaction)&bt_grandma::actionSelectNeedleToTake);
	addChild("TakeNeedle", "ChaseAndTakeNeedle", PRIORITY, NULL, NULL);
	addChild("ChaseAndTakeNeedle", "HowToCutAndTakeNeedle", PRIORITY, (btcondition)&bt_grandma::conditioncan_reach_needle, NULL);
	addChild("HowToCutAndTakeNeedle", "CutRope26", ACTION, (btcondition)&bt_grandma::conditionis_needle_tied, (btaction)&bt_grandma::actionCutRope);
	addChild("HowToCutAndTakeNeedle", "TakeNeedle27", ACTION, (btcondition)&bt_grandma::conditiontrue, (btaction)&bt_grandma::actionTakeNeedle);
	addChild("ChaseAndTakeNeedle", "ChaseNeedlePosition28", ACTION, (btcondition)&bt_grandma::conditiontrue, (btaction)&bt_grandma::actionChaseNeedlePosition);
	addChild("Peacefull", "FreeTime", RANDOM, (btcondition)&bt_grandma::conditiontrue, NULL);
	addChild("FreeTime", "Idle29", ACTION, EXTERNAL, NULL, (btaction)&bt_grandma::actionIdle, 70);
	addChild("FreeTime", "Wander30", SEQUENCE, EXTERNAL, NULL, NULL,30);

	addChild("Wander30", "SearchPoint", ACTION, EXTERNAL, NULL, (btaction)&bt_grandma::actionSearchPoint);
	addChild("Wander30", "ActionWander", ACTION, EXTERNAL, NULL, (btaction)&bt_grandma::actionWander);

	radius = 6.f;
	ind_path = 0;
	own_transform = ((CEntity*)entity)->get<TCompTransform>();
	center = ((TCompTransform*)own_transform)->position;
	character_controller = ((CEntity*)entity)->get<TCompCharacterController>();
	last_time_player_saw = 0;
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
	see_player = false;
	ropeRef = CHandle();
	player_detected_pos = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	m_sensor = ((CEntity*)entity)->get<TCompSensorNeedles>();
	player_pos_sensor = ((CEntity*)entity)->get<TCompPlayerPosSensor>();
	tied_sensor = ((CEntity*)entity)->get<TCompSensorTied>();
	player_transform = ((CEntity*)player)->get<TCompTransform>();
	lastNumNeedlesViewed = 0;
}

//Se mantiene en modo ragdoll durante un tiempo
int bt_grandma::actionRagdoll()
{
	return LEAVE;
}

//Ejecuta la animacin de levantarse
int bt_grandma::actionWakeUp()
{
	return LEAVE;
}

//Corta todas las cuerdas a la que est atada
int bt_grandma::actionCutOwn()
{
	return LEAVE;
}

//
int bt_grandma::actionLeave()
{
	return LEAVE;
}

//Attack to the player when he is too close
int bt_grandma::actionTooCloseAttack()
{
	//Play close attack animation 
	((CEntity*)player)->sendMsg(TActorHit(((CEntity*)player), 5000.f));
	return LEAVE;
}

//Go to the needle position (leave if cant reach)
int bt_grandma::actionChaseNeedlePosition()
{
	if (needle_is_valid){
		CHandle target_needle = ((TCompSensorNeedles*)m_sensor)->getNeedleAsociatedSensor(entity);
		TCompTransform* n_transform = ((CEntity*)target_needle.getOwner())->get<TCompTransform>();

		if (on_enter){
			CNav_mesh_manager::get().findPath(((TCompTransform*)own_transform)->position, n_transform->position, path);
			if (path.size() > 0){
				if (V3DISTANCE((path[path.size() - 1]), n_transform->position)<max_dist_reach_needle-distance_change_way_point){
					ind_path = 0;
					return STAY;
				}else{
					return LEAVE;
				}
			}
		}else{
			if (path.size() > 0){
				if (ind_path < path.size()){
					if (V3DISTANCE((path[path.size() - 1]), n_transform->position) < max_dist_reach_needle - distance_change_way_point){
						chasePoint(((TCompTransform*)own_transform), path[ind_path]);
						if ((V3DISTANCE(((TCompTransform*)own_transform)->position, path[ind_path]) < distance_change_way_point)){
							ind_path++;
							return STAY;
						}
						else{
							return STAY;
						}
					}else{
						last_look_direction = look_direction;
						return LEAVE;
					}
				}else{
					last_look_direction = look_direction;
					return LEAVE;
				}
			}else{
				last_look_direction = look_direction;
				return LEAVE;
			}
		}
	}else{
		last_look_direction = look_direction;
		return LEAVE;
	}
}

//Select the priority needle
int bt_grandma::actionSelectNeedleToTake()
{
	bool sucess = (((TCompSensorNeedles*)m_sensor)->asociateGrandmaTargetNeedle(entity, max_dist_reach_needle, distance_change_way_point));
	if (sucess)
		needle_is_valid = true;
	return LEAVE;

}

//Cut the needles rope
int bt_grandma::actionCutRope()
{
	CHandle target_needle = ((TCompSensorNeedles*)m_sensor)->getNeedleAsociatedSensor(entity);
	CHandle target_rope = ((TCompSensorNeedles*)m_sensor)->getRopeAsociatedSensor(entity);

	((TCompSensorNeedles*)m_sensor)->removeNeedleRope(target_needle);
	CEntityManager::get().remove(CHandle(target_rope).getOwner());
	CEntityManager::get().remove(CHandle(target_needle).getOwner());
	needle_to_take = false;
	needle_is_valid = false;

	return LEAVE;

}

//Take the needle
int bt_grandma::actionTakeNeedle()
{
	CHandle target_needle = ((TCompSensorNeedles*)m_sensor)->getNeedleAsociatedSensor(entity);

	((TCompSensorNeedles*)m_sensor)->removeNeedleRope(target_needle);
	CEntityManager::get().remove(CHandle(target_needle).getOwner());
	needle_to_take = false;
	needle_is_valid = false;

	return LEAVE;
}

//Select the idle and play it
int bt_grandma::actionIdle()
{
	float aux_time = state_time;
	bool aux_on_enter = on_enter;

	//TCompSkeleton* skeleton = ((CEntity*)entity)->get<TCompSkeleton>();

	mov_direction = PxVec3(0, 0, 0);
	look_direction = last_look_direction;

	if (state_time >= 2){
		return LEAVE;
	}else{
		return STAY;
	}

}

//Select a point to go 
int bt_grandma::actionSearchPoint()
{	
	float aux_time = state_time;
	bool aux_on_enter = on_enter;

	rand_point = CNav_mesh_manager::get().getRandomNavMeshPoint(center, radius, ((TCompTransform*)own_transform)->position);

	mov_direction = PxVec3(0, 0, 0);
	look_direction = last_look_direction;

	return LEAVE;

}

//Chase the selected point
int bt_grandma::actionWander()
{
	float aux_time = state_time;
	bool aux_on_enter = on_enter;
	jump = false;
	
	//Tratamos de evitar cambios demasiado repentinos de ruta
	if (on_enter){
		((TCompSkeleton*)(((CEntity*)entity)->get<TCompSkeleton>()))->loopAnimation(1);
		CNav_mesh_manager::get().findPath(((TCompTransform*)own_transform)->position, rand_point, path);
		find_path_time = state_time;
		ind_path = 0;
	}else{
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
			}else{
				return STAY;
			}
		}
		else{
			last_look_direction = look_direction;
			return LEAVE;
		}
	}else{
		return LEAVE;
	}
}

//Makes a warcry
int bt_grandma::actionWarcry()
{
	aimanager::get().warningToClose(this, 20.f);
	have_to_warcry = false;
	return LEAVE;
}

//Alert to the other grandma about the player
int bt_grandma::actionPlayerAlert()
{
	//Call the iaManager method for warning the rest of the grandmas
	aimanager::get().warningPlayerFound(this);
	return LEAVE;
}

//Leave the angry state, go to peacefull
int bt_grandma::actionCalmDown()
{
	return LEAVE;
}

//look the player around the his last point
int bt_grandma::actionLookAround()
{
	return LEAVE;
}

//Takes a roll, attacker or taunter and a poisition to go
int bt_grandma::actionSelectRole()
{
	return LEAVE;
}

//Go to his position
int bt_grandma::actionChaseRoleDistance()
{
	wander_target = ((TCompTransform*)((CEntity*)player)->get<TCompTransform>())->position;
	if (on_enter){
		CNav_mesh_manager::get().findPath(((TCompTransform*)own_transform)->position, wander_target, path);
		ind_path = 0;
		if (path.size() > 0){
			return STAY;
		}else{
			return LEAVE;
		}
	}

	if (path.size() > 0){
		if (ind_path < path.size()){
			chasePoint(((TCompTransform*)own_transform), path[ind_path]);
			if ((V3DISTANCE(((TCompTransform*)own_transform)->position, path[ind_path]) < 0.4f)){
				ind_path++;
				return STAY;
			}else{
				return STAY;
			}
		}else{
			return LEAVE;
		}
	}else{
		return LEAVE;
	}
}

//First attack
int bt_grandma::actionInitialAttack()
{
	return LEAVE;
}

//Move step by step to the roll position (leave on reach or lost)
int bt_grandma::actionSituate()
{
	return LEAVE;
}

//
int bt_grandma::actionNormalAttack()
{
	return LEAVE;
}

//Play a Idle war animation
int bt_grandma::actionIdleWar()
{
	return LEAVE;
}

//Play a taunter routine
int bt_grandma::actionTaunter()
{
	return LEAVE;
}

//Calculate if hurts or ragdoll, if ragdoll then clean all events (los events solo tocan su flag, excepto el ragdoll)
int bt_grandma::actionHurtEvent()
{
	return LEAVE;
}

//
int bt_grandma::actionNeedleAppearsEvent()
{
	currentNumNeedlesViewed = (unsigned int)((TCompSensorNeedles*)m_sensor)->getNumNeedles(entity, max_dist_reach_needle, distance_change_way_point);//list_needles.size();
	if (currentNumNeedlesViewed > lastNumNeedlesViewed){
		if (current != NULL){
			if ((current->getTypeInter() == EXTERNAL) || (current->getTypeInter() == BOTH)){
				setCurrent(NULL);
				lastNumNeedlesViewed = currentNumNeedlesViewed;
				return LEAVE;
			}
		}
	}else{
		return LEAVE;
	}
}

int bt_grandma::actionTiedEvent()
{
	if (ropeRef == nullptr){
		tied_event = false;
		event_detected = false;
		return LEAVE;
	}else{
		//Plays the cut own string animation
		int dice = getRandomNumber(0, 10);
		if (dice < max_bf_posibilities){
			CEntityManager::get().remove(CHandle(ropeRef).getOwner());
			tied_event = false;
			event_detected = false;
			return LEAVE;
		}else{
			tied_event = false;
			event_detected = false;
			tied_succesfull = true;
			return LEAVE;
		}
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
	//return Tied;
}

//
int bt_grandma::conditionis_ragdoll()
{
	return is_ragdoll;
}

//
int bt_grandma::conditionis_grounded()
{
	return false;
	//return is_grounded;
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
		too_close_attack = true;
		last_time = timer;
	}else{
		too_close_attack = false;
	}
	return too_close_attack;
}

//Check if there is a needle to take
int bt_grandma::conditionneedle_to_take()
{
	currentNumNeedlesViewed = (unsigned int)((TCompSensorNeedles*)m_sensor)->getNumNeedles(entity, max_dist_reach_needle, distance_change_way_point);
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
		is_needle_tied=true;
	}else{
		is_needle_tied=false;
	}
	return is_needle_tied;
}

//Check if is necesary a warcry
int bt_grandma::conditionhave_to_warcry()
{
	return have_to_warcry;
}

//Check if there player is not visible for any grandma (and reach the last position)
int bt_grandma::conditionplayer_lost()
{
	if ((last_time_player_saw) > max_time_player_lost){
		return true;
	}
	return false;
}

//check if the player is visible
int bt_grandma::conditionsee_player()
{
	if (findPlayer()){
		see_player = true;
	}else{
		see_player = false;
	}
	return see_player;
}

//Check the look for timer
int bt_grandma::conditionLook_for_timeout()
{
	return false;
	//return Look_for_timeout;
}

//Check if the role is attacker and is close enought
int bt_grandma::conditionis_attacker()
{
	return false;
	//return is_attacker;
}

//
int bt_grandma::conditionnormal_attack()
{
	return false;
	//return normal_attack;
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

//Check if is a falling event
int bt_grandma::conditionfalling_event()
{
	return false;
	//return falling_event;
}

//Check if is a tied event
int bt_grandma::conditiontied_event()
{
	return tied_event;
}

//Check if can reach the selected needle
int bt_grandma::conditioncan_reach_needle()
{
	//XASSERT(needle_objective->needleRef.isValid(), "Invalid needle");
	if (needle_is_valid){
		CHandle target_needle = ((TCompSensorNeedles*)m_sensor)->getNeedleAsociatedSensor(entity);
		XASSERT(target_needle.isValid(), "Invalid owner");
		TCompTransform* e_transform = ((CEntity*)target_needle.getOwner())->get<TCompTransform>();

		wander_target = e_transform->position;

		float distance_prueba = V3DISTANCE(wander_target, ((TCompTransform*)own_transform)->position);

		if (V3DISTANCE(wander_target, ((TCompTransform*)own_transform)->position) <= max_dist_reach_needle){
			can_reach_needle = true;
		}
		else{
			can_reach_needle = false;
		}
	}else{
		can_reach_needle = false;
	}

	return can_reach_needle;
}

//Check if the role is taunter and is close enought
int bt_grandma::conditionis_taunter()
{
	return false;
	//return is_taunter;
}

//
int bt_grandma::conditioninitial_attack()
{
	return false;
	//return initial_attack;
}

//Check if it is too far from the target position
int bt_grandma::conditionfar_from_target_pos()
{
	return false;
	//return far_from_target_pos;
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
	}else{
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
	currentNumNeedlesViewed = (unsigned int)((TCompSensorNeedles*)m_sensor)->getNumNeedles(entity, max_dist_reach_needle, distance_change_way_point);//list_needles.size();
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
			}
		}
		lastNumNeedlesViewed = currentNumNeedlesViewed;
	//}
	//}
}

void bt_grandma::tiedSensor(){
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
	}else{
		if (!((TCompSensorTied*)tied_sensor)->getTiedState()){
			tied_succesfull = false;
			tied_event = false;
			event_detected = false;
		}
	}
}

void bt_grandma::hurtSensor(float damage){

	if (is_angry)
		have_to_warcry = true;
	is_angry = true;
	if (damage >= force_large_impact){
	}else if ((damage >= force_medium_impact) && (damage < force_large_impact)){
		is_ragdoll = true;
	}else if (damage < force_medium_impact){
		hurt_event = true;
	}
}

void bt_grandma::WarWarningSensor(XMVECTOR player_position){
	is_angry = true;
	have_to_warcry = false;
	player_detected_pos = player_position;
	setCurrent(NULL);
}

void bt_grandma::PlayerFoundSensor(){
	see_player = true;
	last_time_player_saw = 0;
	setCurrent(NULL);
}

void bt_grandma::update(float elapsed){
	playerViewedSensor();
	needleViewedSensor();	
	tiedSensor();
	if ((findPlayer()) || (see_player)){
		last_time_player_saw = 0;
	}
	((TCompCharacterController*)character_controller)->Move(mov_direction, false, jump, look_direction);
	this->recalc(elapsed);
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
		}else{
			jump = false;
		}
	}
	mov_direction = Physics.XMVECTORToPxVec3(own_position->getFront());
	look_direction = Physics.XMVECTORToPxVec3(chase_point - own_position->position);
}

void bt_grandma::setId(unsigned int id){
	my_id = id;
}

unsigned int bt_grandma::getId(){
	return my_id;
}

CHandle bt_grandma::getPlayerTransform(){
	return player_transform;
}

bool bt_grandma::findPlayer(){
	if (((TCompPlayerPosSensor*)player_pos_sensor)->playerInRange()) {
		return true;
	}else{
		return false;
	}
}

bool bt_grandma::isAngry(){
	return is_angry;
}


