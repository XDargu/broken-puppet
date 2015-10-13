#include "mcv_platform.h"
#include "comp_bt_soldier.h"
#include "comp_transform.h"
#include "comp_character_controller.h"
#include "comp_rigid_body.h"
#include "comp_sensor_needles.h"
#include "comp_sensor_tied.h"
#include "comp_player_position_sensor.h"
#include "comp_skeleton.h"
#include "comp_sensor_distance_player.h"
#include "nav_mesh_manager.h"
#include "../ai/aimanager.h"
#include "font\font.h"
#include "../audio/sound_manager.h"
#include "entity_manager.h"

TCompBtSoldier::TCompBtSoldier(){ }

TCompBtSoldier::~TCompBtSoldier(){
	delete m_ai_controller;
	m_ai_controller = nullptr;
}

TCompBtSoldier::TCompBtSoldier(bt_soldier* ai_controller) {

	m_ai_controller = new bt_soldier;
	m_ai_controller->SetEntity(CHandle(this).getOwner());
	aimanager::get().addBot(m_ai_controller);
}


void TCompBtSoldier::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	assertRequiredComponent<TCompTransform>(this);
	assertRequiredComponent<TCompCharacterController>(this);
	//assertRequiredComponent<TCompSensorNeedles>(this);
	assertRequiredComponent<TCompPlayerPosSensor>(this);
	//assertRequiredComponent<TCompSensorDistPlayer>(this);
	assertRequiredComponent<TCompSensorTied>(this);
	assertRequiredComponent<TCompSkeleton>(this);
	m_aabb = assertRequiredComponent<TCompAABB>(this);

	//int ind_recast_aabb = atts.getInt("RecastAABBInd", 0);


	m_ai_controller = new bt_soldier;
	m_ai_controller->SetEntity(CHandle(this).getOwner());
	aimanager::get().addBot(m_ai_controller);
}

void TCompBtSoldier::init(){
	m_ai_controller->create("enemy");
	TCompCharacterController* controller = getSibling<TCompCharacterController>(this);
	controller->moveSpeedMultiplier = 0.8f;
	controller->airSpeed = 0.6f;
	controller->jumpPower = 0.7f;

	//Asignación de la fuerza minima para hacer hacer saltar el callback de collisiones
	TCompRigidBody* rigidBody = getSibling<TCompRigidBody>(this);

	physx::PxReal threshold = 10000.f;
	rigidBody->rigidBody->setContactReportThreshold(threshold);

	TCompTransform* pos = assertRequiredComponent<TCompTransform>(this);
	TCompAABB* aabb = m_aabb;
	aabb->update(0);

	int ind_recast_aabb = CNav_mesh_manager::get().getIndexMyRecastAABB(m_aabb);
	m_ai_controller->setIndRecastAABB(ind_recast_aabb);

	player_entity = CEntityManager::get().getByName("Player");
	player_transform = ((CEntity*)player_entity)->get<TCompTransform>();

	footstep_counter = 0;

}

void TCompBtSoldier::update(float elapsed){
	if (m_ai_controller){
		m_ai_controller->update(elapsed);

		TCompTransform* trans = getSibling<TCompTransform>(this);
		TCompCharacterController* c_controller = getSibling<TCompCharacterController>(this);

		// Footsteps sound
		float surface_tag = CSoundManager::get().getMaterialTagValue(c_controller->last_material_tag);
		float surface_value = surface_tag;

		bool moving = m_ai_controller->isMoving();
		float run_speed_modifier = m_ai_controller->getRunSpeedModifier();

		if (moving) {
			footstep_counter += elapsed;

		float base_step = 1.f;
		float time_modifier = run_speed_modifier * 1.f; //* (1 / water_multiplier);

			if (footstep_counter >= time_modifier) {
				CSoundManager::SoundParameter params[] = {
					{ "Material", surface_value }
				};

				CSoundManager::get().playEvent("STEPS_GRANDMA", params, sizeof(params) / sizeof(CSoundManager::SoundParameter), trans->position);
				footstep_counter = 0.0f;
			}
		}
	}
}

void TCompBtSoldier::actorHit(const TActorHit& msg) {
	dbg("Force recieved is  %f\n", msg.damage);
	m_ai_controller->hurtSensor(msg.damage);
}

void TCompBtSoldier::warWarning(const TWarWarning& msg) {
	m_ai_controller->WarWarningSensor(msg.player_position);
}

void TCompBtSoldier::notifyPlayerFound(const TPlayerFound& msg){
	m_ai_controller->PlayerFoundSensor();
}

/*void TCompBtSoldier::notifyPlayerTouch(const TPlayerTouch& msg){
m_ai_controller->PlayerTouchSensor(msg.touch);
}*/

void TCompBtSoldier::groundHit(const TGroundHit& msg) {
	dbg("ground hit recieved is  %f\n", msg.vel);
	if (msg.vel > -15.f){
		//m_fsm_basic_player.last_hit = 2;
	}
	else{
		//m_fsm_basic_player.last_hit = 10;
	}
}

void TCompBtSoldier::onRopeTensed(const TMsgRopeTensed& msg) {
	float damage = 0.f;
	if (msg.sqrRopeDistance < 12 * 12) {
		damage = 30000.f;
		XDEBUG("damage camino menor: %f", msg.sqrRopeDistance);

		//m_ai_controller->setRagdoll();
	}
	else {
		damage = 100000.f;
		CSoundManager& sound_m = CSoundManager::get();
		XDEBUG("damage camino mayor o igual: %f", msg.sqrRopeDistance);
		if (!CLogicManager::get().first_blood){
			//First dismemberment
			//Check if is the enemy really dead and play the sound
			//Play priority sound 
			if (sound_m.getNamedInstanceState("kath_expr_p") != FMOD_STUDIO_PLAYBACK_STATE::FMOD_STUDIO_PLAYBACK_PLAYING) {
				sound_m.stopNamedInstance("kath_expr", FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_IMMEDIATE);
				sound_m.playTalkEvent("KATH_KILL_LAUGH", ((TCompTransform*)player_transform)->position, "kath_expr_p");
				//XDEBUG("logitud joint: %f", djoint->joint->getDistance());
				CLogicManager::get().first_blood = true;
			}
		}
		else{
			//Not first blood so play random comments
			aimanager& aiManager = aimanager::get();
			if (aiManager.bots.size() <= 1){
				//check what tipe of enemy we just killed
				//We killed a soldier
				if (sound_m.getNamedInstanceState("kath_expr_p") != FMOD_STUDIO_PLAYBACK_STATE::FMOD_STUDIO_PLAYBACK_PLAYING) {
					sound_m.stopNamedInstance("kath_expr", FMOD_STUDIO_STOP_MODE::FMOD_STUDIO_STOP_IMMEDIATE);

					int rand = floor(getRandomNumber(0.0f, 2.99f));

					CSoundManager::SoundParameter params[] = {
						{ "frase", rand }
					};
					// Subtítulos
					std::string guid = "EXPR00-1";
					if (rand == 1) { guid = "EXPR01"; }
					if (rand == 2) { guid = "EXPR02"; }
					//Throw movement sound. 
					CSoundManager::get().playTalkEvent("KATH_SOLDIER_KILLED", params, sizeof(params) / sizeof(CSoundManager::SoundParameter), ((TCompTransform*)player_transform)->position, "kath_expr_p", guid);
				}
			}
		}
	}
	if (m_ai_controller)
		m_ai_controller->hurtSensor(damage);
	//m_ai_controller->setRagdoll();
}

void TCompBtSoldier::onNeedleHit(const TMsgNeedleHit& msg) {
	if (m_ai_controller)
		m_ai_controller->needleHitSensor();
}


void TCompBtSoldier::renderDebug3D() {
	TCompTransform* m_transform = ((CEntity*)CHandle(this).getOwner())->get<TCompTransform>();
	font.print3D(m_transform->position + XMVectorSet(0, 1, 0, 0), m_ai_controller->getCurrentNode().c_str());
	if (m_ai_controller)
		m_ai_controller->drawdebug();
}
