#ifndef INC_COMP_AI_BT_H_
#define INC_COMP_AI_BT_H_

#include "base_component.h"

#include "../ai/bt_basic_enemy.h"
#include "../ai/fsm_basic_enemy.h"
#include "../ai/aimanager.h"

struct TCompAiBT : TBaseComponent {

	aicontroller* m_ai_controller;

	TCompAiBT(){ }
	TCompAiBT(bt_basic_enemy* ai_controller) {
		m_ai_controller = new bt_basic_enemy;
		m_ai_controller->SetEntity(CHandle(this).getOwner());
		aimanager::get().addBot(m_ai_controller);
	}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {

		assertRequiredComponent<TCompTransform>(this);
		assertRequiredComponent<TCompUnityCharacterController>(this);

		//m_ai_controller = new ai_basic_enemy;
		m_ai_controller = new bt_basic_enemy;
		m_ai_controller->SetEntity(CHandle(this).getOwner());
		aimanager::get().addBot(m_ai_controller);
	}

	void init(){
		m_ai_controller->create("enemy");
		TCompUnityCharacterController* controller = getSibling<TCompUnityCharacterController>(this);
		controller->moveSpeedMultiplier = 3;
	}

	void update(float elapsed){
		m_ai_controller->update(elapsed);
		//m_ai_controller->recalc(elapsed);
	}

	void actorHit(const TActorHit& msg) {
		dbg("Force recieved is  %f\n", msg.damage);
		//m_ai_controller->EvaluateHit(msg.damage);
	}

	void groundHit(const TGroundHit& msg) {
		dbg("ground hit recieved is  %f\n", msg.vel);
		if (msg.vel > -15.f){
			//m_fsm_basic_player.last_hit = 2;
		}
		else{
			//m_fsm_basic_player.last_hit = 10;
		}
	}

	void onRopeTensed(const TMsgRopeTensed& msg) {
		//m_ai_controller.ChangeState("aibe_Ragdoll");		
	}
};
#endif


