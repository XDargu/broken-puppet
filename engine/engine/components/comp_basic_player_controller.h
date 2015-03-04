//*****************************************//
// ADD THIS COMPONENT AFTER 			   //
//    COMP MESH							   //
//    COMP UNITY CHARACTER CONTROLLER	   //
//*****************************************//

#ifndef INC_COMP_BASIC_PLAYER_CONTROLLER_H_
#define INC_COMP_BASIC_PLAYER_CONTROLLER_H_

#include "mcv_platform.h"
#include "base_component.h"
#include "../handle/app_msgs.h"
#include "../ai/fsm_basic_player.h"

struct TCompBasicPlayerController : TBaseComponent {     

public:


	fsm_basic_player m_fsm_basic_player;

	TCompBasicPlayerController(){ }
	TCompBasicPlayerController(fsm_basic_player n_fsm_basic_player) : m_fsm_basic_player(n_fsm_basic_player) { }

	CHandle m_comp_mesh;
	CHandle m_comp_unity_c_controller;
	CHandle m_entity_player_pivot;
	

	void loadFromAtts(MKeyValue &atts) {
	
		m_fsm_basic_player.SetEntity(CHandle(this).getOwner());

	}

	void init() {	
		m_fsm_basic_player.Init();
	}


	void update(float elapsed) {
		
	}

	void fixedUpdate(float elapsed) {
		m_fsm_basic_player.EvaluateHit();
		m_fsm_basic_player.Recalc(elapsed);
	}
};

#endif
