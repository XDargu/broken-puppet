#include "mcv_platform.h"
#include "comp_prefab_generator.h"
#include "comp_transform.h"
#include "handle\prefabs_manager.h"
#include "comp_rigid_body.h"

TCompPrefabGenerator::TCompPrefabGenerator(){}
TCompPrefabGenerator::~TCompPrefabGenerator(){}

void TCompPrefabGenerator::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	debris_created = 0;
	amount_to_generate = 0;
	prefab_creation_delay = 0;
	last_random_pos = XMVectorSet(0,0,0,0);
	last_created_pos = XMVectorSet(0, 0, 0, 0);
	generate = false;
}

void TCompPrefabGenerator::init(){

}

void TCompPrefabGenerator::update(float elapsed){

	if (generate){
		if (!generatingPrefabs(amount_to_generate, elapsed)){
			//generate = false;
		}
	}

}

void TCompPrefabGenerator::generatePrefab(int amount){
	amount_to_generate = amount;
	generate = true;
}

/**/
bool TCompPrefabGenerator::generatingPrefabs(int amount, float elapsed){

	// Cargar un prefab
	int debris_amount = amount;
	float debris_respawn_time = 0.05f;
	bool active = true;

	if (debris_created <= debris_amount){

		prefab_creation_delay += elapsed;

		if (prefab_creation_delay >= debris_respawn_time){
			prefab_creation_delay = 0;

			TCompTransform* generator_comp_trans = ((CEntity*)CHandle(this).getOwner())->get<TCompTransform>();

			XMVECTOR aux_generator_pos = generator_comp_trans->position;

			int rnd_angle = getRandomNumber(1, 360);
			XMVECTOR random_rotation = XMQuaternionRotationRollPitchYaw(rnd_angle, rnd_angle, rnd_angle);

			XMVECTOR create_position;
			XMVECTOR random_point = getRandomVector3(
				XMVectorGetX(aux_generator_pos) - 2
				, XMVectorGetY(aux_generator_pos) + 2
				, XMVectorGetZ(aux_generator_pos) - 2
				, XMVectorGetX(aux_generator_pos) + 2
				, XMVectorGetY(aux_generator_pos) + 2
				, XMVectorGetZ(aux_generator_pos) + 2);

			bool equal = (Physics.XMVECTORToPxVec3(random_point) == Physics.XMVECTORToPxVec3(last_random_pos));

			if (equal){
				// Calculate a new pos
				PxVec3 m_boss_pos = Physics.XMVECTORToPxVec3(generator_comp_trans->position);

				PxVec3 obj_boss_dir = Physics.XMVECTORToPxVec3(last_created_pos) - m_boss_pos;
				PxVec3 aux_up = PxVec3(0, 0.01f, 0);
				PxVec3 m_force = (obj_boss_dir.cross(PxVec3(0, 1, 0)).getNormalized());

				create_position = last_created_pos + Physics.PxVec3ToXMVECTOR(m_force * 6);

			}
			else{
				create_position = random_point;
			}

			last_created_pos = create_position;
			last_random_pos = random_point;

			std::string name = "";

			// Debris
			int rnd = getRandomNumber(1, 20);
			name = "puppets/debris_0" + std::to_string(rnd);

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
	else{
		active = false;
	}

	return active;
}
/*

	*/