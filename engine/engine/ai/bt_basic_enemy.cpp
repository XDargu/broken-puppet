#include "mcv_platform.h"
#include "bt_basic_enemy.h"
#include "../entity_manager.h"
#include "../components/all_components.h"

using namespace DirectX;

//Constants
const int max_bf_posibilities = 7;
const float max_dist_reach_needle = 1.8f;
const float max_dist_close_attack = 1.7f;
const float max_time_player_lost = 2.f;
const float max_time_tied = 2.f;
const float max_distance_to_attack = 1.5f;
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

void bt_basic_enemy::create(string s)
{
	name = s;

	createRoot("root", PRIORITY, NULL, NULL);
	//addChild("prueba", "patrol", DECORATOR, DEC_LIMIT_TIMES, NULL, NULL);
	/*addChild("root", "patrol", SEQUENCE, BOTH,(btcondition)&bt_basic_enemy::conditionPatrol, NULL);
	addChild("patrol", "chase", ACTION, BOTH, NULL, (btaction)&bt_basic_enemy::actionChasePlayer);
	addChild("patrol", "initial_attack", DECORATOR, BOTH, CONDITIONAL_NODE, NULL, NULL);
	addChild("initial_attack", "initial_attack_action", ACTION, BOTH, NULL, (btaction)&bt_basic_enemy::actionInitialAttack);
	addChild("patrol", "idle_war", DECORATOR, BOTH, LOOP_UNTIL, NULL, NULL);
	addChild("idle_war", "idle_war_act", ACTION, BOTH, NULL, (btaction)&bt_basic_enemy::actionIdleWar);
	addChild("patrol", "attack", DECORATOR, BOTH, TIMER_NODE, NULL, NULL);
	addChild("attack", "attack_action", ACTION, BOTH, NULL, (btaction)&bt_basic_enemy::actionAttack);*/

	addChild("root", "wander", SEQUENCE, BOTH, (btcondition)&bt_basic_enemy::conditionWander, NULL);
	addChild("wander", "search_point", ACTION, BOTH, NULL, (btaction)&bt_basic_enemy::actionSearchPoint);
	addChild("wander", "chase_point", ACTION, BOTH, NULL, (btaction)&bt_basic_enemy::actionChasePoint);
	// Values
	player = CEntityManager::get().getByName("Player");
	character_controller = ((CEntity*)entity)->get<TCompUnityCharacterController>();

	wander_target = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->position;
	comp_mesh = ((CEntity*)entity)->get<TCompMesh>();
	comp_life = ((CEntity*)entity)->get<TCompLife>();

	attack_distance = 1.5f;
	view_distance = 5;
	out_of_reach_distance = 3;
	already_attacked = false;
	already_animated = false;
	initial_attack_done = false;
	player_viewed_sensor = false;

	currentNumNeedlesViewed = 0;
	lastNumNeedlesViewed = 0;
	needle_objective = new needle_rope;

	//inicializamos las condiciones de los timer_nodes
	initDecoratorCondition("attack", 2);
	//------------------------------------------------

}

bool bt_basic_enemy::conditionPatrol(){;
	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();
	TCompTransform* p_transform = ((CEntity*)player)->get<TCompTransform>();
	XMVECTOR mFront = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->getFront();
	float prueba = V3DISTANCE(m_transform->position, p_transform->position);
	if (V3DISTANCE(m_transform->position, p_transform->position) < view_distance)
	{

		// Check if player is visible
		physx::PxRaycastBuffer buf;
		Physics.raycastAll(m_transform->position + XMVectorSet(0, 1.5f, 0, 0), XMVector3Normalize(p_transform->position - m_transform->position), view_distance, buf);

		bool player_visible = false;
		if (buf.nbTouches > 1)
			if (std::strcmp(buf.touches[1].actor->getName(), "Player") == 0) {
				player_visible = true;
			}

		if (player_visible) {
			already_animated = false;
			return true;
		}
		else{
			return false;
		}
	}else{
		return false;
	}
}

int bt_basic_enemy::actionIdleWar(){
	bool time_out = false;
	bool player_moved = false;
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_IdleWar");

	XMVECTOR mPosition = ((TCompTransform*)((CEntity*)entity)->get<TCompTransform>())->position;
	XMVECTOR pPosition = ((TCompTransform*)((CEntity*)player)->get<TCompTransform>())->position;

	((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, Physics.XMVECTORToPxVec3(pPosition - mPosition));

	if ((V3DISTANCE(mPosition, pPosition) > out_of_reach_distance))
	{
		already_animated = false;
		player_moved = true;
	}
	// Stay until next Second
	if ((player_moved) || (trueEveryXSeconds(2)))
	{
		setDecoratorCondition("idle_war", false);
		return LEAVE;
	}else{
		return STAY;
	}
}

int bt_basic_enemy::actionAttack(){
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_Attack");

	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();
	TCompTransform* p_transform = ((CEntity*)player)->get<TCompTransform>();

	if (V3DISTANCE(m_transform->position, p_transform->position) < attack_distance)
	{
		physx::PxVec3 front = Physics.XMVECTORToPxVec3(m_transform->getFront());
		((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, front);

		if (!already_attacked) {
			((CEntity*)player)->sendMsg(TMsgAttackDamage(entity, 10));
			already_attacked = true;
		}

		setDecoratorCondition("attack", true);
		if (getDecoratorCondition("attack")){
			already_attacked = false;
			already_animated = false;
			initDecoratorCondition("idle_war", 0);
			return LEAVE;
		}
		else{
			return STAY;
		}

	}else{
		return LEAVE;
	}

    
}

int bt_basic_enemy::actionInitialAttack(){
	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();
	TCompTransform* p_transform = ((CEntity*)player)->get<TCompTransform>();

	if (V3DISTANCE(m_transform->position, p_transform->position) < attack_distance)
	{
		already_animated = false;
		if (!initial_attack_done) {
			setDecoratorCondition("initial_attack", true);
			initial_attack_done = true;
			((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_Attack");
			TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();

			physx::PxVec3 front = Physics.XMVECTORToPxVec3(m_transform->getFront());
			((TCompUnityCharacterController*)character_controller)->Move(physx::PxVec3(0, 0, 0), false, false, front);
			if (!already_attacked) {
				((CEntity*)player)->sendMsg(TMsgAttackDamage(entity, 10));
				already_attacked = true;
				return STAY;
			}
		}else{
			// Stay until next Second
			if (trueEveryXSeconds(2))
			{
				already_attacked = false;
				already_animated = false;
				initial_attack_done = true;
				return LEAVE;
			}
			else{
				return STAY;
			}
		}
	}else{
		return LEAVE;
	}

	return STAY;
}

int bt_basic_enemy::actionChasePlayer(){
	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();
	TCompTransform* p_transform = ((CEntity*)player)->get<TCompTransform>();

	if ((V3DISTANCE(m_transform->position, p_transform->position) < view_distance) && (V3DISTANCE(m_transform->position, p_transform->position) > attack_distance)){

		((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_Run");

		// Go to the wander_target
		physx::PxVec3 front = Physics.XMVECTORToPxVec3(m_transform->getFront());
		((TCompUnityCharacterController*)character_controller)->Move(front, false, false, Physics.XMVECTORToPxVec3(p_transform->position - m_transform->position));
	
		return STAY;
	}else{
		return LEAVE;
	}
}

bool bt_basic_enemy::conditionWander(){
	player_viewed_sensor = false;
	return true;
}

int bt_basic_enemy::actionSearchPoint(){
	// If wander target reached, set other target

	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();

	int x = getRandomNumber(0, 20) - 10;
	int z = getRandomNumber(0, 20) - 10;
	wander_target = m_transform->position + XMVectorSet(x * 2.0f, 0, z * 2.0f, 0);

	return LEAVE;
}

int bt_basic_enemy::actionChasePoint(){
	// Go to the wander_target
	((TCompMesh*)comp_mesh)->mesh = mesh_manager.getByName("Soldado_MS1_Walk");
	TCompTransform* m_transform = ((CEntity*)entity)->get<TCompTransform>();
	physx::PxVec3 front = Physics.XMVECTORToPxVec3(m_transform->getFront());
	((TCompUnityCharacterController*)character_controller)->Move(front, false, false, Physics.XMVECTORToPxVec3(wander_target - m_transform->position));
	// Check if wall in front of me, then set other target
	physx::PxRaycastBuffer buf;
	Physics.raycastAll(m_transform->position + XMVectorSet(0, 0.1f, 0, 0), XMVector3Normalize(wander_target - m_transform->position), 1.f, buf);

	bool can_go = true;

	for (int i = 0; i < (int)buf.nbTouches; i++)
	{
		if (buf.touches[i].actor != ((TCompUnityCharacterController*)character_controller)->enemy_rigidbody) {
			can_go = false;
		}
	}

	if ((V3DISTANCE(m_transform->position, wander_target) < 3)||(!can_go)){
		return LEAVE;
	}else{
		return STAY;
	}
}

float state_time_bt = 0.f;

void bt_basic_enemy::EvaluateHit(float damage) {
	if (damage <= 10000.f){
		//ChangeState("aibe_Ragdoll");
	}
	else{
		((TCompLife*)comp_life)->life = 0;
		//ChangeState("aibe_Ragdoll");
	}
}

bool bt_basic_enemy::trueEverySecond()
{
	static float counter = 0;
	counter += CApp::get().delta_time;
	if (counter >= 1) {
		counter = 0;
		return true;
	}
	return false;
}

bool bt_basic_enemy::trueEveryXSeconds(float time)
{
	static float counter = 0;
	counter += CApp::get().delta_time;
	if (counter >= time) {
		counter = 0;
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------------------------------------------

void bt_basic_enemy::update(float elapsed){
	playerViewedSensor();
	//needleViewedSensor();
	this->recalc(elapsed);
}

string bt_basic_enemy::getCurrentNode(){
	if (current == NULL)
		return "Root";
	else
		return current->getName();
}

// Sensor para detectar si el enemigo ve al player
void bt_basic_enemy::playerViewedSensor(){
	if (!player_viewed_sensor){

		TCompPlayerPosSensor* p_sensor = ((CEntity*)entity)->get<TCompPlayerPosSensor>();
		bool tri=p_sensor->playerInRange();
		if (p_sensor->playerInRange()) {
			if ((!current->isRoot()) && ((current->getTypeInter() == INTERNAL) || (current->getTypeInter() == BOTH))){
				setCurrent(NULL);
				player_viewed_sensor = true;
			}
		}
	}
}

// Sensor para detectar si el enemigo ve alguna aguja
void bt_basic_enemy::needleViewedSensor(){
	/*NOTA: Las abuelas solo deben tener en cuenta los avisos de este sensor ni no tienen ya asignada
	una aguja objetivo. De no ser así, iran a por todas las agujas nuevas*/

	//componente sensor de agujas del enemigo
	TCompSensorNeedles* m_sensor = ((CEntity*)entity)->get<TCompSensorNeedles>();
	//std::vector<needle_rope>* needle_vector = new std::vector<needle_rope>;
	//le pedimos que nos diga las agujas que el enemigo tiene en su rango
	//std::vector<TCompNeedle*> list_needles = m_sensor->getNeedlesInRange();
	//m_sensor->getNeedlesInRange(needle_vector);
	//if (!needle_vector->empty()){
	//almacenamos el numero de agujas en rango para comprobar variaciones
	currentNumNeedlesViewed = (unsigned int)m_sensor->getNumNeedles(entity, max_dist_reach_needle, distance_change_way_point);//list_needles.size();
	if (currentNumNeedlesViewed != lastNumNeedlesViewed){
		//Si hay variacion reseteamos comprobamos si el nodo es interrumpible
		//Hay que excluir el nodo root, puesto que no incluye niveles de interrupción
		if ((!current->isRoot()) && (current->getTypeInter() == EXTERNAL) || (current->getTypeInter() == BOTH)){
			//m_sensor->asociateGrandmaTargetNeedle(entity);
			setCurrent(NULL);
		}
	}
	lastNumNeedlesViewed = currentNumNeedlesViewed;
	//}
}

void bt_basic_enemy::setId(unsigned int id){
	my_id = id;
}

unsigned int bt_basic_enemy::getInt(){
	return my_id;
}

CHandle bt_basic_enemy::getTransform(){
	CHandle transform = ((CEntity*)entity)->get<TCompTransform>();
	return transform;
}

void bt_basic_enemy::setRol(int r){
	if (r == 1)
		rol = role::ATTACKER;
	else if (r == 2)
		rol = role::TAUNTER;
}

void bt_basic_enemy::setAttackerSlot(int s){
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

int bt_basic_enemy::getAttackerSlot(){
	return slot;
}

int bt_basic_enemy::getRol(){
	return rol;
}

float bt_basic_enemy::getDistanceToPlayer(){
	if (own_transform.isValid())
		return V3DISTANCE(((TCompTransform*)own_transform)->position, ((TCompTransform*)player_transform)->position);
	else
		return 0.f;
}

int bt_basic_enemy::getNearestSlot(bool free_north, bool free_east, bool free_west){
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
