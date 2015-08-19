#ifndef INC_COMP_PLAYER_CONTROLLER_H_
#define INC_COMP_PLAYER_CONTROLLER_H_

#include "base_component.h"
#include "handle/app_msgs.h"
#include "ai\fsm_player_legs.h"
#include "ai\fsm_player_torso.h"

struct TCompPlayerController : TBaseComponent {

	CHandle old_target_transform;
	CHandle needle_back1;
	CHandle needle_back2;
	XMVECTOR offset_needle_back1;
	XMVECTOR offset_needle_back2;
	XMVECTOR offset_rot_needle_back1;
	XMVECTOR offset_rot_needle_back2;
public:
	FSMPlayerLegs fsm_player_legs;
	FSMPlayerTorso fsm_player_torso;
	
	float hit_cool_down;
	float time_since_last_hit;

	TCompPlayerController() : old_target_transform(CHandle()) {}

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);

	void fixedUpdate(float elapsed);
	

	//unsigned int getStringCount();

	void actorHit(const TActorHit& msg);

	void onAttackDamage(const TMsgAttackDamage& msg);

	bool canThrow();
};

#endif
