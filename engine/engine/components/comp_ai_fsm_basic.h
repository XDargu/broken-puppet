#ifndef INC_COMP_AI_FSM_BASIC_H_
#define INC_COMP_AI_FSM_BASIC_H_

#include "base_component.h"
#include "../ai/aicontroller.h"
#include "../ai/ai_basic_enemy.h"

struct TCompAiFsmBasic : TBaseComponent {

	ai_basic_enemy m_ai_controller;

	TCompAiFsmBasic(){ }
	TCompAiFsmBasic(ai_basic_enemy ai_controller) : m_ai_controller(ai_controller) { }

	void loadFromAtts(MKeyValue &atts) {

		assertRequiredComponent<TCompTransform>(this);
		assertRequiredComponent<TCompUnityCharacterController>(this);
		

		m_ai_controller.SetEntity(CHandle(this).getOwner());
	}

	void init(){		
		m_ai_controller.Init();
		TCompUnityCharacterController* controller = getSibling<TCompUnityCharacterController>(this);
		controller->moveSpeedMultiplier = 3;
	}

	void update(float elapsed){
		m_ai_controller.Recalc(elapsed);
	}

	void actorHit(const TActorHit& msg) {
		dbg("Force recieved is  %f\n", msg.damage);
		m_ai_controller.EvaluateHit(msg.damage);
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
};
#endif
