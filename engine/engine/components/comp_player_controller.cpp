#include "mcv_platform.h"
#include "comp_player_controller.h"
#include "comp_life.h"
#include "comp_name.h"
#include "comp_transform.h"
#include "comp_collider_capsule.h"
#include "comp_rigid_body.h"
#include "comp_character_controller.h"
#include "comp_skeleton.h"
#include "comp_ragdoll.h"
#include "../audio/sound_manager.h"
#include "io\iostatus.h"
#include "components\comp_particle_group.h"
#include "handle\prefabs_manager.h"

const string particles_hit_name = "ps_porcelain_hit";

TCompPlayerController::TCompPlayerController() : old_target_transform(CHandle()) {
	fsm_player_legs = new FSMPlayerLegs;
	fsm_player_torso = new FSMPlayerTorso;
}

TCompPlayerController::~TCompPlayerController() {
	delete fsm_player_legs;
	delete fsm_player_torso;
}

void TCompPlayerController::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	assertRequiredComponent<TCompLife>(this);
	assertRequiredComponent<TCompTransform>(this);
	assertRequiredComponent<TCompColliderCapsule>(this);
	TCompRigidBody* rigidBody = assertRequiredComponent<TCompRigidBody>(this);
	assertRequiredComponent<TCompCharacterController>(this);

	fsm_player_legs->SetEntity(CHandle(this).getOwner());
	fsm_player_torso->SetEntity(CHandle(this).getOwner());
	fsm_player_legs->torso = fsm_player_torso;
	fsm_player_torso->legs = fsm_player_legs;

	hit_cool_down = 0.5f;
	time_since_last_hit = 0;

	//55000.f
	physx::PxReal threshold = 55000.f;
	rigidBody->rigidBody->setContactReportThreshold(threshold);

	footsteps = CSoundManager::get().getInstance("STEPS_KATH");
	footstep_counter = 0;

	/*FMOD::Studio::ParameterInstance* surface = NULL;
	footsteps->getParameter("surface", &surface);
	FMOD_RESULT r = surface->setValue(0);

	FMOD::Studio::ParameterInstance* running = NULL;
	footsteps->getParameter("surface", &running);
	r = running->setValue(0);

	footsteps->start();*/
}

/*float displacement;
float counter;
XMVECTOR prev_pos;*/

void TCompPlayerController::init() {

	/*
		Load sounds
	*/

	/*CSoundManager::get().addFX2DTrack("string_free_1.ogg", "string_free_1","throw");
	CSoundManager::get().addFX2DTrack("string_free_2.ogg", "string_free_2", "throw");
	CSoundManager::get().addFX2DTrack("string_free_3.ogg", "string_free_3", "throw");
	CSoundManager::get().addFX2DTrack("string_free_4.ogg", "string_free_4", "throw");
	CSoundManager::get().addFX2DTrack("string_free_5.ogg", "string_free_5", "throw");
	CSoundManager::get().addFX2DTrack("string_grab_6.ogg", "string_grab_6", "tense");
	CSoundManager::get().addFX2DTrack("string_grab_8.ogg", "string_grab_8", "tense");


	CSoundManager::get().addFX2DTrack("string_grab_9.ogg", "string_player_grab", "pull");
	CSoundManager::get().addFX2DTrack("string_grab_7.ogg", "string_player_grab2", "pull");*/

	entity_player = (CHandle(this).getOwner());
	player_trans = (((CEntity*)entity_player)->get<TCompTransform>());

	fsm_player_legs->Init();
	fsm_player_torso->Init();
	
	TCompTransform* trans = assertRequiredComponent<TCompTransform>(this);

	/*prev_pos = trans->position;
	displacement = 0;
	counter = 0;*/

	// Create back needles
	CHandle pref_needle = prefabs_manager.getInstanceByName("player_back_needle");
	if (pref_needle.isValid()) {
		TCompName* name_needle = ((CEntity*)pref_needle)->get<TCompName>();
		if (name_needle) {
			strcpy(name_needle->name, "NeedleCarrete1");
		}
	}
	pref_needle = prefabs_manager.getInstanceByName("player_back_needle");
	if (pref_needle.isValid()) {
		TCompName* name_needle = ((CEntity*)pref_needle)->get<TCompName>();
		if (name_needle) {
			strcpy(name_needle->name, "NeedleCarrete2");
		}
	}

	needle_back1 = CEntityManager::get().getByName("NeedleCarrete1");
	needle_back2 = CEntityManager::get().getByName("NeedleCarrete2");

	float offset_size = 0.05f;
	float offset_rot_size = 0.2f;
	float offset_rot_size2 = 3.14f;
	offset_needle_back1 = XMVectorSet(getRandomNumber(-offset_size, offset_size), getRandomNumber(-offset_size, offset_size), 0, 0);
	offset_needle_back2 = XMVectorSet(getRandomNumber(-offset_size, offset_size), getRandomNumber(-offset_size, offset_size), 0, 0);

	offset_rot_needle_back1 = XMVectorSet(getRandomNumber(-offset_rot_size, offset_rot_size), getRandomNumber(-offset_rot_size, offset_rot_size), 0, 0);
	offset_rot_needle_back2 = XMVectorSet(getRandomNumber(-offset_rot_size, offset_rot_size), getRandomNumber(-offset_rot_size, offset_rot_size), 0, 0);

	// Create jump particle prefab
	CHandle pref_entity = prefabs_manager.getInstanceByName("player_jump_pref");
	if (pref_entity.isValid()) {
		TCompTransform* pref_trans = ((CEntity*)pref_entity)->get<TCompTransform>();
		if (pref_trans) {
			pref_trans->position = trans->position;
			pref_trans->init();
		}
	}

	entity_jump_dust = CEntityManager::get().getByName("PlayerParticleJumpDust");
}

void TCompPlayerController::update(float elapsed) {
	CHandle player_handle = CHandle(this).getOwner();
	CEntity* player_entity = (CEntity*)player_handle;
	TCompRagdoll* p_ragdoll = player_entity->get<TCompRagdoll>();

	bool can_receive_hit = canReceiveDamage();

	if (can_receive_hit) {
		time_since_last_hit += elapsed;
	}

	fsm_player_torso->update(elapsed);

	CIOStatus& io = CIOStatus::get();
	if (io.becomesReleased(CIOStatus::R)) {
		fsm_player_legs->ChangeState("fbp_Ragdoll");
	}	

	TCompTransform* trans = player_entity->get<TCompTransform>();
	TCompRigidBody* rigid = player_entity->get<TCompRigidBody>();
	TCompCharacterController* c_controller = player_entity->get<TCompCharacterController>();
	TCompSkeleton* skeleton = player_entity->get<TCompSkeleton>();

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

	PxActor* hit_actor = nullptr;
	PxVec3 actor_position;
	PxVec3 actor_normal;

	fsm_player_torso->getThrowingData(hit_actor, actor_position, actor_normal);

	if (hit_actor != nullptr) {
		CHandle target_entity(hit_actor->userData);
		if (target_entity.isValid()) {
			if (!(((CEntity*)target_entity)->hasTag("player"))) {
				TCompTransform* target_transform = ((CEntity*)target_entity)->get<TCompTransform>();
				if (!(CHandle(target_transform) == old_target_transform)) {
					if (old_target_transform.isValid()) {
						TCompTransform* old_t_transform = old_target_transform;
						if (old_t_transform->getType() == 80)
							old_t_transform->setType(1);
						if (((TCompTransform*)old_target_transform)->getType() == 90)
							((TCompTransform*)old_target_transform)->setType(0.95f);
					}
				}
				if (target_transform->getType() == 100) {
					target_transform->setType(0.8f);
					old_target_transform = target_transform;
				}
				if (target_transform->getType() == 95) {
					target_transform->setType(0.9f);
					old_target_transform = target_transform;
				}
			}
		}
	}

	/*
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
							((TCompTransform*)old_target_transform)->setType(0.95f);
					}
				}
				if (target_transform->getType() == 100) {
					target_transform->setType(0.8f);
					old_target_transform = target_transform;
				}
				if (target_transform->getType() == 95) {
					target_transform->setType(0.9f);
					old_target_transform = target_transform;
				}
			}
		}
	}*/
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
		TCompSkeleton* skel = player_entity->get<TCompSkeleton>();
		XMVECTOR back_pos = skel->getPositionOfBone(58);
		XMVECTOR back_rot = skel->getRotationOfBone(58);

		CEntity* needle_back1_entity = needle_back1;
		CEntity* needle_back2_entity = needle_back2;

		TCompTransform* needle_back1_t = needle_back1_entity->get<TCompTransform>();
		TCompTransform* needle_back2_t = needle_back2_entity->get<TCompTransform>();

		// Bone transform
		TTransform bone_trans = TTransform(back_pos, back_rot, XMVectorSet(1, 1, 1, 0));

		bone_trans.rotation = XMQuaternionMultiply(bone_trans.rotation, XMQuaternionRotationAxis(bone_trans.getUp(), deg2rad(90)));
		bone_trans.position += bone_trans.getFront() * -0.15f;

		// Here we have the bone_trans transform with the neutral position of a needle
		// Now, add some random rotation/poasition to the needles

		needle_back1_t->rotation = XMQuaternionMultiply(XMQuaternionRotationAxis(bone_trans.getLeft(), XMVectorGetX(offset_rot_needle_back1)), XMQuaternionMultiply(XMQuaternionRotationAxis(bone_trans.getFront(), XMVectorGetY(offset_rot_needle_back1)), bone_trans.rotation));
		needle_back2_t->rotation = XMQuaternionMultiply(XMQuaternionRotationAxis(bone_trans.getLeft(), XMVectorGetX(offset_rot_needle_back2)), XMQuaternionMultiply(XMQuaternionRotationAxis(bone_trans.getFront(), XMVectorGetY(offset_rot_needle_back2)), bone_trans.rotation));

		needle_back1_t->position = bone_trans.position + bone_trans.getLeft() * XMVectorGetX(offset_needle_back1) + bone_trans.getUp() * XMVectorGetY(offset_needle_back1);
		needle_back2_t->position = bone_trans.position + bone_trans.getLeft() * XMVectorGetX(offset_needle_back2) + bone_trans.getUp() * XMVectorGetY(offset_needle_back2);
	}
	
	// Particles
	/*if (entity_jump_dust.isValid()) {
		TCompTransform* pg_trans = ((CEntity*)entity_jump_dust)->get<TCompTransform>();
		pg_trans->position = trans->position;
		pg_trans->lookAt(pg_trans->position + trans->getUp(), XMVectorSet(0, 1, 0, 0));
	}*/

	// Footsteps sound

	float surface_tag = CSoundManager::get().getMaterialTagValue(c_controller->last_material_tag);
	float surface_value = surface_tag;

	bool moving = fsm_player_legs->isMoving();	
	bool running_value = fsm_player_legs->isRunning();

	if (moving) {
		footstep_counter += elapsed;

		float base_step = 0.5f;
		float time_modifier = (running_value ? 0.75f : 1) * (1 / water_multiplier);

		if (footstep_counter >= base_step * time_modifier) {
			CSoundManager::SoundParameter params[] = {
				{ "Material", surface_value }
			};

			CSoundManager::get().playEvent("STEPS_KATH", params, sizeof(params) / sizeof(CSoundManager::SoundParameter), trans->position);
			footstep_counter = 0.0f;
		}
	}
	
	
	
	/*FMOD::Studio::ParameterInstance* surface = NULL;
	footsteps->getParameter("surface", &surface);
	FMOD_RESULT r = surface->setValue(surface_value);

	float running_value = io.isPressed(CIOStatus::RUN);
	FMOD::Studio::ParameterInstance* running = NULL;
	footsteps->getParameter("running", &running);
	r = running->setValue(running_value);

	// 3D Attributes
	FMOD_3D_ATTRIBUTES attributes = { { 0 } };
	attributes.position = CSoundManager::get().XMVECTORtoFmod(trans->position);
	r = footsteps->set3DAttributes(&attributes);*/
}

void TCompPlayerController::fixedUpdate(float elapsed) {
	fsm_player_legs->update(elapsed);
}

//unsigned int TCompPlayerController::getStringCount() {
	//return fsm_player_torso.getStringCount();
//}

void TCompPlayerController::actorHit(const TActorHit& msg) {
	bool can_receive_hit = canReceiveDamage();

	if (can_receive_hit){
		if (time_since_last_hit >= hit_cool_down){
			dbg("Force recieved is  %f\n", msg.damage);
			fsm_player_legs->EvaluateHit(msg.damage);
			time_since_last_hit = 0;

			//Porcelain hit when player is hurt		
			XMVECTOR particles_pos = ((TCompTransform*)player_trans)->position  + XMVectorSet(0, 1.2f, 0, 0);
			CHandle particle_entity = CLogicManager::get().instantiateParticleGroupOneShot(particles_hit_name, particles_pos);

			// Boss
			if (msg.is_boss){
				bossImpact(msg.who);
			}			
			
			
		}
	}
}

void TCompPlayerController::onAttackDamage(const TMsgAttackDamage& msg) {
}


bool TCompPlayerController::canThrow() {
	return fsm_player_torso->canThrow();
}


void TCompPlayerController::bossImpact(CHandle boss){
	/**/
	// hit the player
	CHandle m_player = (CHandle(this).getOwner());
	CHandle m_player_rigid = (((CEntity*)m_player)->get<TCompRigidBody>());
	CHandle m_player_ragdoll = (((CEntity*)m_player)->get<TCompRagdoll>());
	if (m_player_rigid.isValid() && m_player_ragdoll.isValid()){

		TCompTransform* player_comp_trans = (((CEntity*)m_player)->get<TCompTransform>());
		PxVec3 player_pos = Physics.XMVECTORToPxVec3(player_comp_trans->position);

		TCompTransform* enemy_comp_trans = ((CEntity*)boss)->get<TCompTransform>();
		PxVec3 force_dir = (player_pos - Physics.XMVECTORToPxVec3(enemy_comp_trans->position));
		PxVec3 up_aux = PxVec3(0, 30, 0);
		force_dir = (force_dir + up_aux).getNormalized();
		//force_dir = PxVec3(0, 10, 0);

		//((PxRigidDynamic*)((TCompRagdoll*)m_player_ragdoll)->getBoneRigid(1))->setLinearVelocity(force_dir * 2);

		fsm_player_legs->ragdoll_force = force_dir * 10;
	}
	// hurt the player
	/**/
}

bool TCompPlayerController::canReceiveDamage() {
	CHandle player_handle = CHandle(this).getOwner();
	CEntity* player_entity = (CEntity*)player_handle;
	TCompRagdoll* p_ragdoll = player_entity->get<TCompRagdoll>();

	bool can_receive_hit = !p_ragdoll->isRagdollActive();
	can_receive_hit &= fsm_player_legs->getCurrentNode() != "fbp_WakeUp";
	can_receive_hit &= fsm_player_legs->getCurrentNode() != "fbp_Hurt";
	can_receive_hit &= fsm_player_legs->getCurrentNode() != "fbp_WakeUpTeleport";
	can_receive_hit &= fsm_player_legs->getCurrentNode() != "fbp_Dead";

	return can_receive_hit;
}