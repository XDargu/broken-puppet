#include "mcv_platform.h"
#include "comp_bt_grandma.h"
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

TCompBtGrandma::TCompBtGrandma(){ }

TCompBtGrandma::~TCompBtGrandma(){
	delete m_ai_controller;
	m_ai_controller = nullptr;
}

TCompBtGrandma::TCompBtGrandma(bt_grandma* ai_controller) {

	m_ai_controller = new bt_grandma;
	m_ai_controller->SetEntity(CHandle(this).getOwner());
	aimanager::get().addBot(m_ai_controller);
}


void TCompBtGrandma::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	assertRequiredComponent<TCompTransform>(this);
	assertRequiredComponent<TCompCharacterController>(this);
	assertRequiredComponent<TCompSensorNeedles>(this);
	assertRequiredComponent<TCompPlayerPosSensor>(this);
	assertRequiredComponent<TCompSensorDistPlayer>(this);
	assertRequiredComponent<TCompSensorTied>(this);
	assertRequiredComponent<TCompSkeleton>(this);
	m_aabb = assertRequiredComponent<TCompAABB>(this);

	//int ind_recast_aabb = atts.getInt("RecastAABBInd", 0);
	

	m_ai_controller = new bt_grandma;
	m_ai_controller->SetEntity(CHandle(this).getOwner());
	aimanager::get().addBot(m_ai_controller);
}

void TCompBtGrandma::init(){
	m_ai_controller->create("enemy");
	TCompCharacterController* controller = getSibling<TCompCharacterController>(this);
	controller->moveSpeedMultiplier = 0.8f;
	controller->airSpeed = 0.6f;
	controller->jumpPower = 0.7f;

	//Asignación de la fuerza minima para hacer hacer saltar el callback de collisiones
	TCompRigidBody* rigidBody = getSibling<TCompRigidBody>(this);

	physx::PxReal threshold = 300.f;
	rigidBody->rigidBody->setContactReportThreshold(threshold);

	TCompTransform* pos = assertRequiredComponent<TCompTransform>(this);
	TCompAABB* aabb = m_aabb;
	aabb->update(0);	

	int ind_recast_aabb = CNav_mesh_manager::get().getIndexMyRecastAABB(m_aabb);
	m_ai_controller->setIndRecastAABB(ind_recast_aabb);

}

void TCompBtGrandma::update(float elapsed){
	m_ai_controller->update(elapsed);
}

void TCompBtGrandma::actorHit(const TActorHit& msg) {
	dbg("Force recieved is  %f\n", msg.damage);
	m_ai_controller->hurtSensor(msg.damage);
}

void TCompBtGrandma::warWarning(const TWarWarning& msg) {
	m_ai_controller->WarWarningSensor(msg.player_position);
}

void TCompBtGrandma::notifyPlayerFound(const TPlayerFound& msg){
	m_ai_controller->PlayerFoundSensor();
}

/*void TCompBtGrandma::notifyPlayerTouch(const TPlayerTouch& msg){
	m_ai_controller->PlayerTouchSensor(msg.touch);
}*/

void TCompBtGrandma::groundHit(const TGroundHit& msg) {
	dbg("ground hit recieved is  %f\n", msg.vel);
	if (msg.vel > -15.f){
		//m_fsm_basic_player.last_hit = 2;
	}
	else{
		//m_fsm_basic_player.last_hit = 10;
	}
}

void TCompBtGrandma::onRopeTensed(const TMsgRopeTensed& msg) {	
	m_ai_controller->setRagdoll();
}


void TCompBtGrandma::renderDebug3D() {
	TCompTransform* m_transform = ((CEntity*)CHandle(this).getOwner())->get<TCompTransform>();
	font.print3D(m_transform->position + XMVectorSet(0, 1, 0, 0), m_ai_controller->getCurrentNode().c_str());
	m_ai_controller->drawdebug();
}
