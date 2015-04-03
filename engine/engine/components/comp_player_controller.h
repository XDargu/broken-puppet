#ifndef INC_COMP_PLAYER_CONTROLLER_H_
#define INC_COMP_PLAYER_CONTROLLER_H_

#include "base_component.h"
#include "handle/app_msgs.h"
#include "ai\fsm_player_legs.h"

struct TCompPlayerController : TBaseComponent {
public:
	FSMPlayerLegs fsm_player_legs;

	float hit_cold_down;
	float time_since_last_hit;

	TCompPlayerController() {}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		assertRequiredComponent<TCompLife>(this);
		assertRequiredComponent<TCompTransform>(this);
		assertRequiredComponent<TCompColliderCapsule>(this);
		assertRequiredComponent<TCompRigidBody>(this);
		assertRequiredComponent<TCompCharacterController>(this);

		fsm_player_legs.SetEntity(CHandle(this).getOwner());		
	}

	void init() {
		fsm_player_legs.Init();
		hit_cold_down = 1;
		time_since_last_hit = 0;
	}

	void update(float elapsed) {
		time_since_last_hit += elapsed;
	}

	void fixedUpdate(float elapsed) {
		fsm_player_legs.update(elapsed);
	}

	void actorHit(const TActorHit& msg) {

		if (time_since_last_hit >= hit_cold_down){
			dbg("Force recieved is  %f\n", msg.damage);
			if (msg.damage <= 10000.f){
				fsm_player_legs.last_hit = 2;
			}
			else{
				fsm_player_legs.last_hit = 10;
			}
			fsm_player_legs.EvaluateHit();
			time_since_last_hit = 0;
		}
	}

	void onAttackDamage(const TMsgAttackDamage& msg) {
		if (time_since_last_hit >= hit_cold_down){
			dbg("Damage recieved is  %f\n", msg.damage);
			fsm_player_legs.last_hit = 2;
			fsm_player_legs.EvaluateHit();
			time_since_last_hit = 0;
		}
	}
};

#endif
