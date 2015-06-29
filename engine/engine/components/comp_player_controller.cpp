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

	CEntity* camera_entity = CEntityManager::get().getByName("PlayerCamera");
	TCompTransform* camera_transform = camera_entity->get<TCompTransform>();

	PxRaycastBuffer hit;
	Physics.raycast(camera_transform->position, camera_transform->getFront(), 1000, hit);

	if (hit.hasBlock) {
		CHandle target_entity(hit.block.actor->userData);
		if (target_entity.isValid()) {
			if (!(((CEntity*)target_entity)->hasTag("player"))) {
				TCompTransform* target_transform = ((CEntity*)target_entity)->get<TCompTransform>();
				if (!(CHandle(target_transform) == old_target_transform)) {
					if (old_target_transform.isValid()) {
						TCompTransform* old_t_transform = old_target_transform;
						if (old_t_transform->getType() == 80)
							old_t_transform->setType(1);
						if (((TCompTransform*)old_target_transform)->getType() == 90)
							((TCompTransform*)old_target_transform)->setType(0.95);
					}
				}
				if (target_transform->getType() == 100) {
					target_transform->setType(0.8);
					old_target_transform = target_transform;
				}
				if (target_transform->getType() == 95) {
					target_transform->setType(0.9);
					old_target_transform = target_transform;
				}
			}
		}
	}
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
