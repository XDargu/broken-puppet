#ifndef INC_COMP_PLAYER_CONTROLLER_H_
#define INC_COMP_PLAYER_CONTROLLER_H_

#include "base_component.h"
#include "handle/app_msgs.h"
#include "ai\fsm_player_legs.h"
#include "ai\fsm_player_torso.h"

struct TCompPlayerController : TBaseComponent {
public:
	FSMPlayerLegs fsm_player_legs;
	FSMPlayerTorso fsm_player_torso;
	
	float hit_cool_down;
	float time_since_last_hit;

	TCompPlayerController() {}

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);

	void fixedUpdate(float elapsed);
	

	unsigned int getStringCount();

	void actorHit(const TActorHit& msg);

	void onAttackDamage(const TMsgAttackDamage& msg);
};

#endif
