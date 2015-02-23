#ifndef INC_COMP_AI_FSM_BASIC_H_
#define INC_COMP_AI_FSM_BASIC_H_

#include "base_component.h"
#include "../ai/aicontroller.h"
#include "../ai/ai_basic_wander.h"

struct TCompAiFsmBasic : TBaseComponent {

	ai_basic_wander m_ai_controller;

	TCompAiFsmBasic(){ }
	TCompAiFsmBasic(ai_basic_wander ai_controller) : m_ai_controller(ai_controller) { }

	void loadFromAtts(MKeyValue &atts) {
		ai_basic_wander ai;
		m_ai_controller = ai;
	}

	void init(){
		m_ai_controller.SetEntity(CHandle(this).getOwner());
		m_ai_controller.Init();
	}

	void update(float elapsed){
		m_ai_controller.Recalc(elapsed);
	}


};
#endif
