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

		CEntity* e = CHandle(this).getOwner();
		TCompTransform* trans = e->get<TCompTransform>();
		TCompEnemyController* enemy_controller = e->get<TCompEnemyController>();

		assert(trans || fatal("TCompAiFsmBasic requieres a TCompTransform component"));
		assert(enemy_controller || fatal("TCompAiFsmBasic requieres a TCompEnemyController component"));

		ai_basic_enemy ai;
		m_ai_controller = ai;
		m_ai_controller.SetEntity(CHandle(this).getOwner());
	}

	void init(){
		
		m_ai_controller.Init();
	}

	void update(float elapsed){
		m_ai_controller.Recalc(elapsed);
	}


};
#endif
