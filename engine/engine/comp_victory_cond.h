#ifndef INC_COMP_VICTORY_COND_H_
#define INC_COMP_VICTORY_COND_H_

#include "base_component.h"

struct TCompVictoryCond : TBaseComponent {
public:

	TCompVictoryCond(){}

	void loadFromAtts(MKeyValue &atts) {
	}

	void init() {
	}

	void update(float elapsed) {

	}


	std::string toString() {
		return "Victory Condition";
	}

	void victory(const TVictoryCondition& msg) {
		dbg("Victory Condition\n");
		CEntity* e = CEntityManager::get().getByName("Player");
		if (e == msg.whoEnter){
			CApp::get().activateVictory();
		}
	}
};

#endif