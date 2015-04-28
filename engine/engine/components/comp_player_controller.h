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

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		assertRequiredComponent<TCompLife>(this);
		assertRequiredComponent<TCompTransform>(this);
		assertRequiredComponent<TCompColliderCapsule>(this);
		assertRequiredComponent<TCompRigidBody>(this);
		assertRequiredComponent<TCompCharacterController>(this);

		fsm_player_legs.SetEntity(CHandle(this).getOwner());
		fsm_player_torso.SetEntity(CHandle(this).getOwner());
		fsm_player_legs.torso = &fsm_player_torso;

		hit_cool_down = 1;
		time_since_last_hit = 0;
	}

	void init() {
		fsm_player_legs.Init();
		fsm_player_torso.Init();
	}

	void update(float elapsed) {
		time_since_last_hit += elapsed;

		if (isKeyPressed('F')) {
			fsm_player_legs.ChangeState("fbp_Ragdoll");
		}
		fsm_player_torso.update(elapsed);
	}

	void fixedUpdate(float elapsed) {
		fsm_player_legs.update(elapsed);

	}

	unsigned int getStringCount() {
		return fsm_player_torso.getStringCount();
	}

	void actorHit(const TActorHit& msg) {

		if (time_since_last_hit >= hit_cool_down){
			dbg("Force recieved is  %f\n", msg.damage);
			fsm_player_legs.EvaluateHit(msg.damage);
			time_since_last_hit = 0;
		}
	}

	void onAttackDamage(const TMsgAttackDamage& msg) {
		if (time_since_last_hit >= hit_cool_down){
			dbg("Damage recieved is  %f\n", msg.damage);
			//fsm_player_legs.last_hit = 2;
			fsm_player_legs.EvaluateHit(msg.damage);
			time_since_last_hit = 0;
		}
	}
};

#endif
