#include "mcv_platform.h"
#include "comp_player_controller.h"
#include "comp_life.h"
#include "comp_transform.h"
#include "comp_collider_capsule.h"
#include "comp_rigid_body.h"
#include "comp_character_controller.h"
#include "comp_skeleton.h"
#include "comp_ragdoll.h"
#include "../audio/sound_manager.h"
#include "io\iostatus.h"

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

	physx::PxReal threshold = 100.f;
	rigidBody->rigidBody->setContactReportThreshold(threshold);
}

/*float displacement;
float counter;
XMVECTOR prev_pos;*/

void TCompPlayerController::init() {

	/*
		Load sounds
	*/

	CSoundManager::get().addFX2DTrack("string_free_1.ogg", "string_free_1","throw");
	CSoundManager::get().addFX2DTrack("string_free_2.ogg", "string_free_2", "throw");
	CSoundManager::get().addFX2DTrack("string_free_3.ogg", "string_free_3", "throw");
	CSoundManager::get().addFX2DTrack("string_free_4.ogg", "string_free_4", "throw");
	CSoundManager::get().addFX2DTrack("string_free_5.ogg", "string_free_5", "throw");
	CSoundManager::get().addFX2DTrack("string_grab_6.ogg", "string_grab_6", "tense");
	CSoundManager::get().addFX2DTrack("string_grab_8.ogg", "string_grab_8", "tense");


	CSoundManager::get().addFX2DTrack("string_grab_9.ogg", "string_player_grab", "pull");
	CSoundManager::get().addFX2DTrack("string_grab_7.ogg", "string_player_grab2", "pull");

	fsm_player_legs.Init();
	fsm_player_torso.Init();
	
	TCompTransform* trans = assertRequiredComponent<TCompTransform>(this);

	/*prev_pos = trans->position;
	displacement = 0;
	counter = 0;*/

	needle_back1 = CEntityManager::get().getByName("NeedleCarrete1");
	needle_back2 = CEntityManager::get().getByName("NeedleCarrete2");

	float offset_size = 0.05f;
	float offset_rot_size = 0.2f;
	float offset_rot_size2 = 3.14f;
	offset_needle_back1 = XMVectorSet(getRandomNumber(-offset_size, offset_size), getRandomNumber(-offset_size, offset_size), 0, 0);
	offset_needle_back2 = XMVectorSet(getRandomNumber(-offset_size, offset_size), getRandomNumber(-offset_size, offset_size), 0, 0);

	offset_rot_needle_back1 = XMVectorSet(getRandomNumber(-offset_rot_size, offset_rot_size), getRandomNumber(-offset_rot_size, offset_rot_size), 0, 0);
	offset_rot_needle_back2 = XMVectorSet(getRandomNumber(-offset_rot_size, offset_rot_size), getRandomNumber(-offset_rot_size, offset_rot_size), 0, 0);
}

void TCompPlayerController::update(float elapsed) {
	time_since_last_hit += elapsed;

	fsm_player_torso.update(elapsed);	

	CIOStatus& io = CIOStatus::get();
	if (io.becomesReleased(CIOStatus::R)) {
		fsm_player_legs.ChangeState("fbp_Ragdoll");
	}	

	TCompTransform* trans = assertRequiredComponent<TCompTransform>(this);
	TCompRigidBody* rigid = assertRequiredComponent<TCompRigidBody>(this);
	TCompCharacterController* c_controller = assertRequiredComponent<TCompCharacterController>(this);
	TCompSkeleton* skeleton = assertRequiredComponent<TCompSkeleton>(this);

	/*dbg((
		"X:" + std::to_string(XMVectorGetX(trans->position)) + ", "
		+ "Y:" + std::to_string(XMVectorGetY(trans->position)) + ", "
		+ "Z:" + std::to_string(XMVectorGetZ(trans->position)) + "\n"
		).c_str());*/

	float water_level = CApp::get().water_level;
	float atten = 0.2f;
	float water_multiplier = 1;

	if (rigid->rigidBody->getGlobalPose().p.y < water_level - atten)  {
		float proportion = min(1, (water_level - rigid->rigidBody->getGlobalPose().p.y) / atten);
		water_multiplier = 1 - (proportion * 0.5f);
	}

	//c_controller->jumpPower = 10.2 / water_multiplier;
	c_controller->gravityMultiplier = 48 * water_multiplier;
	skeleton->model->getMixer()->setTimeFactor(water_multiplier);

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

	// Back needles
	if (needle_back1.isValid() && needle_back2.isValid()) {
		TCompSkeleton* skel = assertRequiredComponent<TCompSkeleton>(this);
		XMVECTOR back_pos = skel->getPositionOfBone(58);
		XMVECTOR back_rot = skel->getRotationOfBone(58);

		CEntity* needle_back1_entity = needle_back1;
		CEntity* needle_back2_entity = needle_back2;

		TCompTransform* needle_back1_t = needle_back1_entity->get<TCompTransform>();
		TCompTransform* needle_back2_t = needle_back2_entity->get<TCompTransform>();

		// Bone transform
		TTransform bone_trans = TTransform(back_pos, back_rot, XMVectorSet(1, 1, 1, 0));

		bone_trans.rotation = XMQuaternionMultiply(bone_trans.rotation, XMQuaternionRotationAxis(bone_trans.getUp(), deg2rad(90)));
		bone_trans.position += bone_trans.getFront() * -0.15;

		// Here we have the bone_trans transform with the neutral position of a needle
		// Now, add some random rotation/poasition to the needles

		needle_back1_t->rotation = XMQuaternionMultiply(XMQuaternionRotationAxis(bone_trans.getLeft(), XMVectorGetX(offset_rot_needle_back1)), XMQuaternionMultiply(XMQuaternionRotationAxis(bone_trans.getFront(), XMVectorGetY(offset_rot_needle_back1)), bone_trans.rotation));
		needle_back2_t->rotation = XMQuaternionMultiply(XMQuaternionRotationAxis(bone_trans.getLeft(), XMVectorGetX(offset_rot_needle_back2)), XMQuaternionMultiply(XMQuaternionRotationAxis(bone_trans.getFront(), XMVectorGetY(offset_rot_needle_back2)), bone_trans.rotation));

		needle_back1_t->position = bone_trans.position + bone_trans.getLeft() * XMVectorGetX(offset_needle_back1) + bone_trans.getUp() * XMVectorGetY(offset_needle_back1);
		needle_back2_t->position = bone_trans.position + bone_trans.getLeft() * XMVectorGetX(offset_needle_back2) + bone_trans.getUp() * XMVectorGetY(offset_needle_back2);
	}
	
}

void TCompPlayerController::fixedUpdate(float elapsed) {
	fsm_player_legs.update(elapsed);
}

//unsigned int TCompPlayerController::getStringCount() {
	//return fsm_player_torso.getStringCount();
//}

void TCompPlayerController::actorHit(const TActorHit& msg) {
	CHandle player_handle = CHandle(this).getOwner();
	CEntity* player_entity = (CEntity*)player_handle;
	TCompRagdoll* p_ragdoll = player_entity->get<TCompRagdoll>();
	TCompPlayerController* p_controller = player_entity->get<TCompPlayerController>();
	if ((!(p_ragdoll->isRagdollActive()) || (p_controller->fsm_player_legs.getCurrentNode() == "fbp_WakeUp"))){
		if (time_since_last_hit >= hit_cool_down){
			dbg("Force recieved is  %f\n", msg.damage);
			fsm_player_legs.EvaluateHit(msg.damage);
			time_since_last_hit = 0;
		}
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


bool TCompPlayerController::canThrow() {
	return fsm_player_torso.canThrow();
}