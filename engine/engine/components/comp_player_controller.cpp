#include "mcv_platform.h"
#include "comp_player_controller.h"
#include "comp_life.h"
#include "comp_transform.h"
#include "comp_collider_capsule.h"
#include "comp_rigid_body.h"
#include "comp_character_controller.h"

void TCompPlayerController::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	assertRequiredComponent<TCompLife>(this);
	assertRequiredComponent<TCompTransform>(this);
	assertRequiredComponent<TCompColliderCapsule>(this);
	TCompRigidBody* rigidBody = assertRequiredComponent<TCompRigidBody>(this);
	assertRequiredComponent<TCompCharacterController>(this);

	fsm_player_legs.SetEntity(CHandle(this).getOwner());
	fsm_player_torso.SetEntity(CHandle(this).getOwner());
	fsm_player_legs.torso = &fsm_player_torso;
	fsm_player_torso.legs = &fsm_player_legs;

	hit_cool_down = 1;
	time_since_last_hit = 0;

	physx::PxReal threshold = 300.f;
	rigidBody->rigidBody->setContactReportThreshold(threshold);
}

/*float displacement;
float counter;
XMVECTOR prev_pos;*/

void TCompPlayerController::init() {
	fsm_player_legs.Init();
	fsm_player_torso.Init();
	
	TCompTransform* trans = assertRequiredComponent<TCompTransform>(this);

	/*prev_pos = trans->position;
	displacement = 0;
	counter = 0;*/
}

void TCompPlayerController::update(float elapsed) {
	time_since_last_hit += elapsed;

	//fsm_player_torso.update(elapsed);

	TCompTransform* trans = assertRequiredComponent<TCompTransform>(this);
	/*dbg((
		"X:" + std::to_string(XMVectorGetX(trans->position)) + ", "
		+ "Y:" + std::to_string(XMVectorGetY(trans->position)) + ", "
		+ "Z:" + std::to_string(XMVectorGetZ(trans->position)) + "\n"
		).c_str());*/

	
	/*displacement += V3DISTANCE(prev_pos, trans->position);
	counter += elapsed;
	if (counter >= 1) {
		XDEBUG("Displacement: %f", displacement);
		displacement = 0;
		counter = 0;
	}
	prev_pos = trans->position;*/
}

void TCompPlayerController::fixedUpdate(float elapsed) {
	fsm_player_legs.update(elapsed);
	fsm_player_torso.update(elapsed);
}

//unsigned int TCompPlayerController::getStringCount() {
	//return fsm_player_torso.getStringCount();
//}

void TCompPlayerController::actorHit(const TActorHit& msg) {

	if (time_since_last_hit >= hit_cool_down){
		dbg("Force recieved is  %f\n", msg.damage);
		fsm_player_legs.EvaluateHit(msg.damage);
		time_since_last_hit = 0;
	}
}

void TCompPlayerController::onAttackDamage(const TMsgAttackDamage& msg) {
	if (time_since_last_hit >= hit_cool_down){
		dbg("Damage recieved is  %f\n", msg.damage);
		//fsm_player_legs.last_hit = 2;
		fsm_player_legs.EvaluateHit(msg.damage);
		time_since_last_hit = 0;
	}
}
