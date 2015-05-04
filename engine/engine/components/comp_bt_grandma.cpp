#include "mcv_platform.h"
#include "comp_bt_grandma.h"
#include "comp_transform.h"
#include "comp_character_controller.h"
#include "comp_sensor_needles.h"
#include "comp_player_position_sensor.h"
#include "../ai/aimanager.h"

aicontroller* m_ai_controller;

TCompBtGrandma::TCompBtGrandma(){ }
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

	m_ai_controller = new bt_grandma;
	m_ai_controller->SetEntity(CHandle(this).getOwner());
	aimanager::get().addBot(m_ai_controller);
}

void TCompBtGrandma::init(){
	m_ai_controller->create("enemy");
	TCompCharacterController* controller = getSibling<TCompCharacterController>(this);
	controller->moveSpeedMultiplier = 1.0f;
	controller->jumpPower = 0.7f;
}

void TCompBtGrandma::update(float elapsed){
	m_ai_controller->update(elapsed);
}

void TCompBtGrandma::actorHit(const TActorHit& msg) {
	dbg("Force recieved is  %f\n", msg.damage);
	//m_ai_controller.EvaluateHit(msg.damage);
}

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
	//m_ai_controller.ChangeState("aibe_Ragdoll");		
}
