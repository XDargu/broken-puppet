#include "mcv_platform.h"
#include "comp_ai_boss.h"
#include "comp_ai_substitute.h"
#include "entity_manager.h"
#include "comp_rigid_body.h"
#include "comp_transform.h"
#include "comp_skeleton.h"
#include "comp_needle.h"
#include "comp_ragdoll.h"
#include "comp_render.h"
#include "comp_rope.h"
#include "comp_explosion.h"
#include "comp_distance_joint.h"
#include "comp_point_light.h"
#include "item_manager.h"
#include "io\iostatus.h"
#include "handle\prefabs_manager.h"
#include "rope_manager.h"
#include "audio\sound_manager.h"

TCompAiBoss::TCompAiBoss() {
	m_fsm_boss = new fsm_boss;
}

TCompAiBoss::~TCompAiBoss() {
	delete m_fsm_boss;
}

void TCompAiBoss::loadFromAtts(const std::string& elem, MKeyValue &atts){
	L_hitch_joint = nullptr;
	R_hitch_joint = nullptr;
	can_break_hitch = false;
	death_time = false;
	proximity_distance = 14.f;
	calculate_break_point = false;
	heart_opened = false;
	boss_creation_delay = 0.f;
	last_created_pos = XMVectorSet(0, 0, 0, 0);
	last_random_pos = XMVectorSet(0, 0, 0, 0);
	safe_raining = false;
	safe_debris_amount = 0;
};

void TCompAiBoss::init(){

	mPlayer = CEntityManager::get().getByName("Player");
	if (mPlayer.isValid()){
		player_trans = ((CEntity*)mPlayer)->get<TCompTransform>();
	}

	mBoss = CEntityManager::get().getByName("Boss");

	point_offset = PxVec3(0, -6, 0);
	distance_to_point = 12;

	activate = false;
	shoot = false;
	attack2Active = false;
	attack2Time = 0;

	move_things = 0;
	debris_creation_delay = 0;
	debris_created = 0;

	is_death = false;

	hitchs_opened = false;
	force = 24;
	R_hitch_joint = nullptr;

	m_fsm_boss->entity = mBoss;
	m_fsm_boss->Init();

	/**********************************************************
	RIGHT ARM, HITCH, LITH
	/**********************************************************/
	comp_skeleton = ((CEntity*)mBoss)->get<TCompSkeleton>();
	TCompSkeleton* skeleton = comp_skeleton;

	// Right Arm hitch
	std::string rname = "boss/enganche_R";
	R_hitch = prefabs_manager.getInstanceByName(rname.c_str());
	TCompRigidBody* R_hitch_rigid = ((CEntity*)R_hitch)->get<TCompRigidBody>();
	R_hitch_rigid->init();

	TCompTransform* R_hitch_trans = ((CEntity*)R_hitch)->get<TCompTransform>();
	if (R_hitch_trans) R_hitch_trans->setType(0);
	PxRigidDynamic*  R_hitch_px_rigid = R_hitch_rigid->rigidBody;

	PxQuat r_rot = Physics.XMVECTORToPxQuat(skeleton->getRotationOfBone(48));
	PxVec3 r_pos = Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(48));

	PxTransform r_bone_trans = PxTransform(r_pos, r_rot);

	R_hitch_joint = PxFixedJointCreate(
		*Physics.gPhysicsSDK
		, R_hitch_px_rigid
		, PxTransform(PxVec3(0, 0, 0))
		, NULL
		, r_bone_trans
		);

	R_hitch_joint->setLocalPose(PxJointActorIndex::eACTOR1, r_bone_trans);
	R_hitch_px_rigid->setGlobalPose(r_bone_trans);

	// R_light
	R_hitch_light = prefabs_manager.getInstanceByName("boss/enganche_luz_R");
	TCompTransform* R_light_trans = ((CEntity*)R_hitch_light)->get<TCompTransform>();

	// Follow
	if (R_light_trans){
		R_light_trans->setType(0);
		R_light_trans->position = R_hitch_trans->position;
	}

	// Hacerlas visibles
	if (R_hitch_light.isValid()){
		((TCompRender*)((CEntity*)R_hitch_light)->get<TCompRender>())->active = false;
	}

	// Apagar la luz
	TCompPointLight* R_point_light = ((CEntity*)R_hitch_light)->get<TCompPointLight>();
	if (R_point_light){
		R_point_light->radius = 0.00001f;
	}

	/**********************************************************
	LEFT ARM, HITCH, LITH
	/**********************************************************/
	std::string lname = "boss/enganche_L";
	L_hitch = prefabs_manager.getInstanceByName(lname.c_str());
	TCompRigidBody* L_hitch_rigid = ((CEntity*)L_hitch)->get<TCompRigidBody>();
	L_hitch_rigid->init();
	PxRigidDynamic*  L_hitch_px_rigid = L_hitch_rigid->rigidBody;

	TCompTransform* L_hitch_trans = ((CEntity*)L_hitch)->get<TCompTransform>();
	if (L_hitch_trans) L_hitch_trans->setType(0);

	PxQuat l_rot = Physics.XMVECTORToPxQuat(skeleton->getRotationOfBone(23));
	PxVec3 l_pos = Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(23));

	PxTransform l_bone_trans = PxTransform(l_pos, l_rot);

	L_hitch_joint = PxFixedJointCreate(
		*Physics.gPhysicsSDK
		, L_hitch_px_rigid
		, PxTransform(PxVec3(0, 0, 0))
		, NULL
		, l_bone_trans
		);

	L_hitch_joint->setLocalPose(PxJointActorIndex::eACTOR1, l_bone_trans);
	L_hitch_px_rigid->setGlobalPose(l_bone_trans);
	/**/

	// L_light
	L_hitch_light = prefabs_manager.getInstanceByName("boss/enganche_luz_L");
	TCompTransform* L_light_trans = ((CEntity*)L_hitch_light)->get<TCompTransform>();
	// Follow
	if (L_light_trans){
		L_light_trans->setType(0);
		L_light_trans->position = L_hitch_trans->position;
	}
	// Hacerlas visibles
	if (L_hitch_light.isValid()){
		((TCompRender*)((CEntity*)L_hitch_light)->get<TCompRender>())->active = false;
	}
	// Apagar la luz
	TCompPointLight* L_point_light = ((CEntity*)L_hitch_light)->get<TCompPointLight>();
	if (L_point_light){
		L_point_light->radius = 0.00001f;
	}

	/**********************************************************
	HEART
	/**********************************************************/
	std::string hname = "boss/heart";
	H_hitch = prefabs_manager.getInstanceByName(hname.c_str());
	TCompRigidBody* H_hitch_rigid = ((CEntity*)H_hitch)->get<TCompRigidBody>();
	H_hitch_rigid->init();

	TCompTransform* H_hitch_trans = ((CEntity*)H_hitch)->get<TCompTransform>();
	if (H_hitch_trans) H_hitch_trans->setType(0);
	PxRigidDynamic*  H_hitch_px_rigid = H_hitch_rigid->rigidBody;

	PxQuat h_rot = Physics.XMVECTORToPxQuat(skeleton->getRotationOfBone(72));
	PxVec3 h_pos = Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(72));

	PxTransform h_bone_trans = PxTransform(h_pos, h_rot);

	H_hitch_joint = PxFixedJointCreate(
		*Physics.gPhysicsSDK
		, H_hitch_px_rigid
		, PxTransform(PxVec3(0, 0, 0))
		, NULL
		, h_bone_trans
		);

	H_hitch_joint->setLocalPose(PxJointActorIndex::eACTOR1, h_bone_trans);
	H_hitch_px_rigid->setGlobalPose(h_bone_trans);

	// Heart_light
	H_hitch_light = prefabs_manager.getInstanceByName("boss/heart_luz");
	TCompTransform* H_light_trans = ((CEntity*)H_hitch_light)->get<TCompTransform>();
	// Follow
	if (H_light_trans){
		H_light_trans->setType(0);
		H_light_trans->position = H_hitch_trans->position;
	}
	// Hacerlas visibles
	if (H_hitch_light.isValid()){
		((TCompRender*)((CEntity*)H_hitch_light)->get<TCompRender>())->active = false;
	}
	// Apagar la luz
	TCompPointLight* H_point_light = ((CEntity*)H_hitch_light)->get<TCompPointLight>();
	if (H_point_light){
		H_point_light->radius = 0.00001f;
		original_ligh_color = H_point_light->color;
	}

	// Creating a collider to avoid problems in the initial rain	
	CHandle anti_rain = prefabs_manager.getInstanceByName("boss/collider_anti_rain");
	if (anti_rain.isValid()){
		TCompTransform* anti_rain_trans = ((CEntity*)anti_rain)->get<TCompTransform>();
		if (anti_rain_trans)
			anti_rain_trans->teleport(XMVectorSet(10000, 10000, 10000, 10000));		
	}



	/***********************************************************
	/***************** Creating the substitute *****************
	************************************************************/

	substitute = CEntityManager::get().getByName("Substitute");
	if (substitute.isValid()){
		TCompTransform* substitute_trans = ((CEntity*)substitute)->get<TCompTransform>();
		TCompRigidBody* substitute_rigid = ((CEntity*)substitute)->get<TCompRigidBody>();

		// Get an initial posisition
		XMVECTOR chair_pos = XMVectorSet(0, 0, 0, 0);
		XMVECTOR chair_rotation = XMVectorSet(0, 0, 0, 1);

		// Get the chair position
		CHandle chair = CEntityManager::get().getByName("boss_trono_sustituta");
		if (chair.isValid()){
			TCompTransform* chair_trans = ((CEntity*)chair)->get<TCompTransform>();			
			
			if (chair_trans){
				chair_pos = chair_trans->position;
				chair_rotation = chair_trans->rotation;
			}
			if (substitute_rigid){
				PxTransform aux_pose = substitute_rigid->rigidBody->getGlobalPose();
				aux_pose.q = Physics.XMVECTORToPxQuat(chair_rotation);
				substitute_rigid->rigidBody->setGlobalPose(aux_pose);
			}
		}
		
		if (substitute_trans){
			substitute_trans->teleport(chair_pos);
			substitute_trans->rotation = chair_rotation;
		}
			
	}		

	/***********************************************************
	***************** Creating the broken floor ****************
	************************************************************/

	// Make the floor disapear
	CHandle floor = CEntityManager::get().getByName("tapa_hueco_boss");
	if (floor.isValid()){
		TCompTransform* floor_trans = ((CEntity*)floor)->get<TCompTransform>();
		CHandle broken_floor = prefabs_manager.getInstanceByName("boss/tapa_boss_rota");
		if (broken_floor.isValid() && (floor_trans)){
			TCompTransform* broken_floor_trans = ((CEntity*)broken_floor)->get<TCompTransform>();

			if (broken_floor_trans){
				broken_floor_trans->setType(0);
				broken_floor_trans->position = floor_trans->position;

				TCompRender* floor_broken_render = ((CEntity*)broken_floor)->get<TCompRender>();
				if (floor_broken_render)
					floor_broken_render->active = false;
			}
		}
	}
}

void TCompAiBoss::update(float elapsed){

	CIOStatus& io = CIOStatus::get();

	if (CIOStatus::get().becomesPressed(CIOStatus::H)){
		stun();
	}
	if (CIOStatus::get().becomesPressed(CIOStatus::J)){
		m_fsm_boss->HeartHit();
	}
	if (CIOStatus::get().becomesPressed(CIOStatus::L)){
		objToStun();
	}
	if (CIOStatus::get().becomesPressed(CIOStatus::K)){
		safe_raining = true;
		safe_debris_amount = 20;
	}

	if (safe_raining){
		safe_raining = safeRain(elapsed, safe_debris_amount);
	}

	if (m_fsm_boss->can_proximity && (mBoss.isValid()) && (mPlayer.isValid())){
		boss_trans = ((CEntity*)mBoss)->get<TCompTransform>();

		if ((boss_trans.isValid()) && (player_trans.isValid())){
			if (V3DISTANCE(((TCompTransform*)boss_trans)->position, ((TCompTransform*)player_trans)->position) < proximity_distance){
				m_fsm_boss->ChangeState("fbp_Proximity");
			}
		}
	}
	/**/
	comp_skeleton = ((CEntity*)mBoss)->get<TCompSkeleton>();
	TCompSkeleton* skeleton = comp_skeleton;
	/**/


	/**********************************************************
	RIGHT ARM, FOLLOW
	/**********************************************************/
	TCompRigidBody* R_hitch_rigid = ((CEntity*)R_hitch)->get<TCompRigidBody>();
	PxRigidDynamic*  R_hitch_px_rigid = R_hitch_rigid->rigidBody;

	PxQuat r_rot = Physics.XMVECTORToPxQuat(skeleton->getRotationOfBone(48));
	PxVec3 r_pos = Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(48));
	PxTransform r_bone_trans = PxTransform(r_pos, r_rot);

	R_hitch_joint->setLocalPose(PxJointActorIndex::eACTOR1, r_bone_trans);
	R_hitch_px_rigid->setGlobalPose(r_bone_trans);

	// Follow
	TCompTransform* R_light_trans = ((CEntity*)R_hitch_light)->get<TCompTransform>();
	if (R_light_trans){
		R_light_trans->setType(0);
		R_light_trans->position = Physics.PxVec3ToXMVECTOR(r_pos);
		R_light_trans->rotation = Physics.PxQuatToXMVECTOR(r_rot);
	}

	/**********************************************************
	LEFT ARM, FOLLOW
	/**********************************************************/
	TCompRigidBody* L_hitch_rigid = ((CEntity*)L_hitch)->get<TCompRigidBody>();
	PxRigidDynamic*  L_hitch_px_rigid = L_hitch_rigid->rigidBody;

	PxQuat l_rot = Physics.XMVECTORToPxQuat(skeleton->getRotationOfBone(23));
	PxVec3 l_pos = Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(23));
	PxTransform l_bone_trans = PxTransform(l_pos, l_rot);

	L_hitch_joint->setLocalPose(PxJointActorIndex::eACTOR1, l_bone_trans);
	L_hitch_px_rigid->setGlobalPose(l_bone_trans);

	// Follow
	TCompTransform* L_light_trans = ((CEntity*)L_hitch_light)->get<TCompTransform>();
	if (L_light_trans){
		L_light_trans->setType(0);
		L_light_trans->position = Physics.PxVec3ToXMVECTOR(l_pos);
		L_light_trans->rotation = Physics.PxQuatToXMVECTOR(l_rot);
	}


	/**********************************************************
	HEART FOLLOW
	/**********************************************************/
	if (!is_death){
		/**/
		TCompRigidBody* H_hitch_rigid = ((CEntity*)H_hitch)->get<TCompRigidBody>();
		PxRigidDynamic*  H_hitch_px_rigid = H_hitch_rigid->rigidBody;

		PxQuat h_rot = Physics.XMVECTORToPxQuat(skeleton->getRotationOfBone(72));
		PxVec3 h_pos = Physics.XMVECTORToPxVec3(skeleton->getPositionOfBone(72));
		PxTransform h_bone_trans = PxTransform(h_pos, h_rot);

		H_hitch_joint->setLocalPose(PxJointActorIndex::eACTOR1, h_bone_trans);
		H_hitch_px_rigid->setGlobalPose(h_bone_trans);

		// light
		TCompTransform* H_light_trans = ((CEntity*)H_hitch_light)->get<TCompTransform>();
		if (H_light_trans){
			H_light_trans->setType(0);
			H_light_trans->position = Physics.PxVec3ToXMVECTOR(h_pos);
			H_light_trans->rotation = Physics.PxQuatToXMVECTOR(h_rot);
		}
	}



	if ((m_fsm_boss->getState() == "fbp_Stunned1") && (!can_break_hitch)) {
		can_break_hitch = true;

		TCompTransform* R_hitch_trans = ((CEntity*)R_hitch)->get<TCompTransform>();
		if (R_hitch_trans) R_hitch_trans->setType(1);
		TCompTransform* L_hitch_trans = ((CEntity*)L_hitch)->get<TCompTransform>();
		if (L_hitch_trans) L_hitch_trans->setType(1);
	}
	else if ((m_fsm_boss->getState() != "fbp_Stunned1") && (can_break_hitch)) {
		can_break_hitch = false;

		TCompTransform* R_hitch_trans = ((CEntity*)R_hitch)->get<TCompTransform>();
		if (R_hitch_trans) R_hitch_trans->setType(0);
		TCompTransform* L_hitch_trans = ((CEntity*)L_hitch)->get<TCompTransform>();
		if (L_hitch_trans) L_hitch_trans->setType(0);
	}
	else if (m_fsm_boss->getState() == "fbp_FinalState"){
		openHeart(elapsed);

		if (!death_time) {
			death_time = true;
			TCompTransform* H_hitch_trans = ((CEntity*)H_hitch)->get<TCompTransform>();
			if (H_hitch_trans) H_hitch_trans->setType(1);
		}
	}

	// Check the hitch
	if (m_fsm_boss->getState() == "fbp_Stunned1")
	{
		openLight(elapsed);
	}
	else{
		closeLight();
		hitchs_opened = false;
	}

	// Destroy heart
	if (is_death){
		brokeHeart();
	}
}

void TCompAiBoss::fixedUpdate(float elapsed){

	m_fsm_boss->update(elapsed);

}


void TCompAiBoss::breakHitch(CHandle m_hitch){
	if ((m_hitch == R_hitch) && (can_break_hitch)) {
		if (m_fsm_boss->EvaluateHit(1)){
			TCompTransform* boss_trans = ((CEntity*)mBoss)->get<TCompTransform>();
			CSoundManager::get().playEvent("BOSS_ARM", boss_trans->position);
			//CApp::get().slowMotion(1.0f);
		}
	}
	if ((m_hitch == L_hitch) && (can_break_hitch)) {
		if (m_fsm_boss->EvaluateHit(0)){
			TCompTransform* boss_trans = ((CEntity*)mBoss)->get<TCompTransform>();
			CSoundManager::get().playEvent("BOSS_ARM", boss_trans->position);
			//CApp::get().slowMotion(1.0f);
		}
	}
	if ((m_hitch == H_hitch)) {
		m_fsm_boss->HeartHit();
		H_hitch_joint->setBreakForce(0, 0);
		is_death = true;
		if (H_hitch_light.isValid()){
			((TCompRender*)((CEntity*)H_hitch_light)->get<TCompRender>())->active = false;
		}
	}
}

void TCompAiBoss::stun(){
	if (m_fsm_boss->HeadHit()){
		TCompTransform* R_light_trans = ((CEntity*)R_hitch_light)->get<TCompTransform>();
		if (R_light_trans){
			R_light_trans->scale = XMVectorSetX(R_light_trans->scale, 0.01f);
		}
	}
}

bool TCompAiBoss::openLight(float elapsed){

	float scale_target = 0.3f;

	TCompTransform* R_light_trans = ((CEntity*)R_hitch_light)->get<TCompTransform>();
	TCompTransform* L_light_trans = ((CEntity*)L_hitch_light)->get<TCompTransform>();

	if (!hitchs_opened){
		if ((R_hitch_light.isValid()) && (R_hitch_light.isValid())){
			
			TCompPointLight* R_point_light = ((CEntity*)R_hitch_light)->get<TCompPointLight>();
			TCompPointLight* L_point_light = ((CEntity*)L_hitch_light)->get<TCompPointLight>();

			TCompRender* R_point_render = ((CEntity*)R_hitch_light)->get<TCompRender>();
			TCompRender* L_point_render = ((CEntity*)L_hitch_light)->get<TCompRender>();

			if (    (R_light_trans) && (L_light_trans)
				&& (R_point_render) && (L_point_render))
			{
				if (m_fsm_boss->has_right){
					R_light_trans->scale = XMVectorSetX(R_light_trans->scale, 0);
					((TCompRender*)((CEntity*)R_hitch_light)->get<TCompRender>())->active = true;
					R_light_trans->setType(0);
					R_point_light->radius = 3.f;
				}
				if (m_fsm_boss->has_left){
					L_light_trans->scale = XMVectorSetX(L_light_trans->scale, 0);
					((TCompRender*)((CEntity*)L_hitch_light)->get<TCompRender>())->active = true;
					L_light_trans->setType(0);
					L_point_light->radius = 3.f;
				}								
			}
		}
	}

	float aux_actual_scale = 0;
	float aux_new_scale = 0;

	// Scale lerp
	aux_actual_scale = XMVectorGetX(R_light_trans->scale);
	if ((aux_actual_scale <= scale_target) && (m_fsm_boss->has_right)){
		aux_new_scale = lerp(aux_actual_scale, scale_target, 0.2f) * elapsed;
		R_light_trans->scale = XMVectorSetX(R_light_trans->scale, aux_new_scale + aux_actual_scale);
	}
	
	// Scale lerp
	aux_actual_scale = XMVectorGetX(L_light_trans->scale);
	if ((aux_actual_scale <= scale_target) && (m_fsm_boss->has_left)){
		aux_new_scale = lerp(aux_actual_scale, scale_target, 0.2f) * elapsed;
		L_light_trans->scale = XMVectorSetX(L_light_trans->scale, aux_new_scale + aux_actual_scale);
	}

	hitchs_opened = true;
	return true;
}

void TCompAiBoss::closeLight(){
	if (hitchs_opened){
		if (R_hitch_light.isValid()){
			((TCompRender*)((CEntity*)R_hitch_light)->get<TCompRender>())->active = false;

			TCompPointLight* R_point_light = ((CEntity*)R_hitch_light)->get<TCompPointLight>();
			if (R_point_light){
				R_point_light->radius = 0.0001f;
			}
		}
		if (L_hitch_light.isValid()){
			((TCompRender*)((CEntity*)L_hitch_light)->get<TCompRender>())->active = false;

			TCompPointLight* L_point_light = ((CEntity*)L_hitch_light)->get<TCompPointLight>();
			if (L_point_light){
				L_point_light->radius = 0.0001f;
			}
		}
	}
}

void TCompAiBoss::openHeart(float elapsed){

	float scale_target = 1.f;

	TCompTransform* H_light_trans = ((CEntity*)H_hitch_light)->get<TCompTransform>();

	if (!heart_opened){
		if ((H_hitch_light.isValid())){

			TCompPointLight* H_point_light = ((CEntity*)H_hitch_light)->get<TCompPointLight>();
			TCompRender* H_point_render = ((CEntity*)H_hitch_light)->get<TCompRender>();

			if ((H_light_trans)
				&& (H_point_render))
			{
				H_light_trans->scale = XMVectorSetX(H_light_trans->scale, 0);
				((TCompRender*)((CEntity*)H_hitch_light)->get<TCompRender>())->active = true;
				H_light_trans->setType(0);
				H_point_light->radius = 3.f;

			}
		}
	}

	float aux_actual_scale = 0;
	float aux_new_scale = 0;

	// Scale lerp
	aux_actual_scale = XMVectorGetX(H_light_trans->scale);
	if ((aux_actual_scale <= scale_target)){
		aux_new_scale = lerp(aux_actual_scale, scale_target, 0.02f) * elapsed;
		H_light_trans->scale = XMVectorSetX(H_light_trans->scale, aux_new_scale + aux_actual_scale);
	}

	heart_opened = true;
}

void TCompAiBoss::initBoss(){
	XMVECTOR aux_pos = XMVectorSet(0, 1, 0, 0);

	CHandle tapa_suelo = CEntityManager::get().getByName("tapa_hueco_boss");
	if (tapa_suelo.isValid()){
		TCompTransform* tapa_suelo_trans = ((CEntity*)tapa_suelo)->get<TCompTransform>();
		if (tapa_suelo_trans)
			aux_pos = tapa_suelo_trans->position;
	}

	CSoundManager::get().playEvent("BOSS_RISE_UP", aux_pos);

	// Change the state to: RiseUp
	m_fsm_boss->lua_boss_init = true;
}

void TCompAiBoss::initialRain(int debris_amount){
	safe_raining = true;
	safe_debris_amount = debris_amount;
}

bool TCompAiBoss::safeRain(float elapsed,int debris_amount){

	float debris_respawn_time = 0.05f;
	float bomb_respawn_time = 1.f;
	bool active = true;

	if (debris_created <= debris_amount){

		debris_creation_delay += elapsed;

		if (debris_creation_delay >= debris_respawn_time){
			debris_creation_delay = 0;

			TCompTransform* enemy_comp_trans = ((CEntity*)mBoss)->get<TCompTransform>();

			XMVECTOR aux_boss_pos = enemy_comp_trans->position;
			
			int rnd_angle = getRandomNumber(1, 360);
			XMVECTOR random_rotation = XMQuaternionRotationRollPitchYaw(rnd_angle, rnd_angle, rnd_angle);
		
			XMVECTOR create_position;
			XMVECTOR random_point = getRandomVector3(
														XMVectorGetX(aux_boss_pos) - 17
														, XMVectorGetY(aux_boss_pos) + 60
														, XMVectorGetZ(aux_boss_pos) - 10
														, XMVectorGetX(aux_boss_pos) + 17
														, XMVectorGetY(aux_boss_pos) + 61
														, XMVectorGetZ(aux_boss_pos) + 10);

			bool equal = (Physics.XMVECTORToPxVec3(random_point) == Physics.XMVECTORToPxVec3(last_random_pos));
			if (equal){
				// Calculate a new pos
				PxVec3 m_boss_pos = Physics.XMVECTORToPxVec3(enemy_comp_trans->position);

				PxVec3 obj_boss_dir = Physics.XMVECTORToPxVec3(last_created_pos) - m_boss_pos;
				PxVec3 aux_up = PxVec3(0, 0.01f, 0);
				PxVec3 m_force = (obj_boss_dir.cross(PxVec3(0, 1, 0)).getNormalized());

				create_position = last_created_pos + Physics.PxVec3ToXMVECTOR(m_force * 6);
			}
			else{
				create_position = random_point;
			}

			// Check if is far from the player
			
			if (player_trans.isValid()){
				XMVECTOR aux_create_position = XMVectorSetY(create_position, XMVectorGetY(((TCompTransform*)player_trans)->position));
				float aux_distance = XMVectorGetX(XMVector3Length(aux_create_position - ((TCompTransform*)player_trans)->position));
				if (aux_distance >= 15){
					last_created_pos = create_position;
					last_random_pos = random_point;

					std::string name = "";

					// Debris
					int rnd = getRandomNumber(1, 20);
					name = "boss/debris_0" + std::to_string(rnd);

					CEntity* prefab_entity = prefabs_manager.getInstanceByName(name.c_str());

					TCompTransform* prefab_t = prefab_entity->get<TCompTransform>();
					if (prefab_t){
						prefab_t->init();
						prefab_t->teleport(create_position);
					}

					TCompRigidBody* prefab_rb = prefab_entity->get<TCompRigidBody>();
					if (prefab_rb){
						prefab_rb->init();
						PxTransform rigid_pose = prefab_rb->rigidBody->getGlobalPose();
						rigid_pose.q = Physics.XMVECTORToPxQuat(random_rotation);
						prefab_rb->rigidBody->setGlobalPose(rigid_pose);
					}

					debris_created++;
				}
			}
		}
	}
	else{
		debris_created = 0;
		active = false;
	}
	return active;

}

CHandle TCompAiBoss::objToStun(){	

	CHandle first_bomb = prefabs_manager.getInstanceByName("boss/first_bomb");

	if (first_bomb.isValid()){
		TCompTransform* first_bomb_trans = ((CEntity*)first_bomb)->get<TCompTransform>();
		TCompRigidBody* first_bomb_rigid = ((CEntity*)first_bomb)->get<TCompRigidBody>();
		TCompExplosion* first_bomb_exp = ((CEntity*)first_bomb)->get<TCompExplosion>();
		TCompTransform* boss_trans = ((CEntity*)mBoss)->get<TCompTransform>();

		// Calculate random rotation
		int rnd_angle = getRandomNumber(1, 360);
		XMVECTOR random_rotation = XMQuaternionRotationRollPitchYaw(rnd_angle, rnd_angle, rnd_angle);
		
		XMVECTOR boss_front = XMVectorSet(0, 0, 1, 0);
		if (boss_trans)
			boss_front = boss_trans->getFront() * 5;

		if (first_bomb_exp)
			first_bomb_exp->init();

		if (mBoss.isValid()){
			TCompTransform* aux_boss_trans = ((CEntity*)mBoss)->get<TCompTransform>();

			if (aux_boss_trans){
				PxTransform aux_bomb_pose = first_bomb_rigid->rigidBody->getGlobalPose();
				aux_bomb_pose.p = Physics.XMVECTORToPxVec3(XMVectorSetY(aux_boss_trans->position + boss_front 
					, (XMVectorGetY(aux_boss_trans->position) + 50)));
				aux_bomb_pose.q = Physics.XMVECTORToPxQuat(random_rotation);
				first_bomb_rigid->rigidBody->setGlobalPose(aux_bomb_pose);
			}
		}
	}	
	return first_bomb;
}

void TCompAiBoss::brokeHeart(){

	if (H_hitch.isValid()){
		TCompTransform* H_hitch_trans = ((CEntity*)H_hitch)->get<TCompTransform>();

		if ((!calculate_break_point) && (H_hitch_trans)){
			calculate_break_point = true;
			break_point = H_hitch_trans->position;
		}

		// Calculate distance between the heart and his hitch
		float aux_dist = 0;

		if (H_hitch_trans){
			aux_dist = XMVectorGetX(XMVector3Length(H_hitch_trans->position - break_point));
		}

		if (aux_dist > 3){
			// Remove rope
			CRope_manager& rope_manager = CRope_manager::get();
			
			TCompDistanceJoint* mJoint = nullptr;
			PxDistanceJoint* px_joint = nullptr;
			PxRigidActor* actor1 = nullptr;
			PxRigidActor* actor2 = nullptr;
			TCompRope* rope = nullptr;

			std::vector<CHandle> target_ropes;

			for (auto& string : CRope_manager::get().getStrings()) {
				rope = string;
				if (rope) {
					mJoint = rope->joint;

					if (mJoint){
						px_joint = mJoint->joint;
						px_joint->getActors(actor1, actor2);

						if (actor1)	{
							if (H_hitch == CHandle(actor1->userData)){
								target_ropes.push_back(CHandle(rope));
							}
						}

						if (actor2){
							if (H_hitch == CHandle(actor2->userData)){
								target_ropes.push_back(CHandle(rope));
							}
						}
					}
				}
			}

			for (auto& it : target_ropes) {
				if (it.isValid()) {
					rope = it;
					rope_manager.removeString(it);

					// Remove needles
					CHandle needle1 = rope->transform_1_aux;
					if (needle1.isValid()) {
						CEntity* e1 = CHandle(needle1).getOwner();
						if (e1) {
							CHandle c_needle1 = e1->get<TCompNeedle>();

							if (c_needle1.isValid()){
								Citem_manager::get().removeNeedleFromVector(c_needle1);
								Citem_manager::get().removeNeedle(c_needle1);
								CEntityManager::get().remove(CHandle(needle1).getOwner());
							}
						}
					}

					CHandle needle2 = rope->transform_2_aux;
					if (needle2.isValid()){
						CEntity* e2 = CHandle(needle2).getOwner();
						if (e2) {
							CHandle c_needle2 = e2->get<TCompNeedle>();
							if (c_needle2.isValid()){
								Citem_manager::get().removeNeedleFromVector(c_needle2);
								Citem_manager::get().removeNeedle(c_needle2);
								CEntityManager::get().remove(CHandle(needle2).getOwner());
							}
						}
					}
				}
			}

			// Save the heart position
			TCompTransform* h_transform = nullptr;
			h_transform = ((CEntity*)H_hitch)->get<TCompTransform>();

			// Remove Entity
			CEntityManager::get().remove(H_hitch);

			// Create Broken Heart
			CEntity* heart_piece1 = prefabs_manager.getInstanceByName("boss/heart_piece_1");
			CEntity* heart_piece2 = prefabs_manager.getInstanceByName("boss/heart_piece_2");
			CEntity* heart_piece3 = prefabs_manager.getInstanceByName("boss/heart_piece_3");
			CEntity* heart_piece4 = prefabs_manager.getInstanceByName("boss/heart_piece_4");
			CEntity* heart_piece5 = prefabs_manager.getInstanceByName("boss/heart_piece_5");
			CEntity* heart_piece6 = prefabs_manager.getInstanceByName("boss/heart_piece_6");

			TCompRigidBody* heart_piece_rigid1 = heart_piece1->get<TCompRigidBody>();
			TCompRigidBody* heart_piece_rigid2 = heart_piece2->get<TCompRigidBody>();
			TCompRigidBody* heart_piece_rigid3 = heart_piece3->get<TCompRigidBody>();
			TCompRigidBody* heart_piece_rigid4 = heart_piece4->get<TCompRigidBody>();
			TCompRigidBody* heart_piece_rigid5 = heart_piece5->get<TCompRigidBody>();
			TCompRigidBody* heart_piece_rigid6 = heart_piece6->get<TCompRigidBody>();

			// Set position
			if (((h_transform) && (heart_piece_rigid1))
				&& ((h_transform) && (heart_piece_rigid2))
				&& ((h_transform) && (heart_piece_rigid3))
				&& ((h_transform) && (heart_piece_rigid4))
				&& ((h_transform) && (heart_piece_rigid5))
				&& ((h_transform) && (heart_piece_rigid6)))
			{
				((PxRigidDynamic*)(heart_piece_rigid1->rigidBody))->setGlobalPose(Physics.transformToPxTransform(h_transform->getPrevTransform()));
				((PxRigidDynamic*)(heart_piece_rigid1->rigidBody))->setGlobalPose(Physics.transformToPxTransform(h_transform->getPrevTransform()));
				((PxRigidDynamic*)(heart_piece_rigid3->rigidBody))->setGlobalPose(Physics.transformToPxTransform(h_transform->getPrevTransform()));
				((PxRigidDynamic*)(heart_piece_rigid4->rigidBody))->setGlobalPose(Physics.transformToPxTransform(h_transform->getPrevTransform()));
				((PxRigidDynamic*)(heart_piece_rigid5->rigidBody))->setGlobalPose(Physics.transformToPxTransform(h_transform->getPrevTransform()));
				((PxRigidDynamic*)(heart_piece_rigid6->rigidBody))->setGlobalPose(Physics.transformToPxTransform(h_transform->getPrevTransform()));
			}
		}
	}
}
