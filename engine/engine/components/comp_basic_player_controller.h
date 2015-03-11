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

	float hit_cold_down;
	float time_since_last_hit;
	

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
	
		m_fsm_basic_player.SetEntity(CHandle(this).getOwner());
		assertRequiredComponent<TCompLife>(this);

	}

	void init() {	
		m_fsm_basic_player.Init();
		hit_cold_down = 1;
		time_since_last_hit = 0;
	}


	void update(float elapsed) {
		//if (isKeyPressed('E'))m_fsm_basic_player.Dead();
		//if (isKeyPressed('Q'))m_fsm_basic_player.last_hit = 10;
		time_since_last_hit += elapsed;
	}

	void fixedUpdate(float elapsed) {	
		m_fsm_basic_player.Recalc(elapsed);
	}

	void actorHit(const TActorHit& msg) {

		if (time_since_last_hit >= hit_cold_down){
			dbg("Force recieved is  %f\n", msg.damage);
			if (msg.damage <= 10000.f){
				m_fsm_basic_player.last_hit = 2;
			}
			else{
				m_fsm_basic_player.last_hit = 10;
			}
			m_fsm_basic_player.EvaluateHit();
			time_since_last_hit = 0;
		}
	}

	void onAttackDamage(const TMsgAttackDamage& msg) {
		if (time_since_last_hit >= hit_cold_down){
			dbg("Damage recieved is  %f\n", msg.damage);
			m_fsm_basic_player.last_hit = 2;
			m_fsm_basic_player.EvaluateHit();
			time_since_last_hit = 0;
		}
	}

};

#endif
