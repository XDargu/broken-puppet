#include "mcv_platform.h"
#include "comp_bomb_generator.h"
#include "comp_transform.h"
#include "handle\prefabs_manager.h"
#include "comp_rigid_body.h"
#include "comp_particle_group.h"
#include "comp_explosion.h"
#include "handle\app_msgs.h"
#include "comp_ai_boss.h"

TCompBombGenerator::TCompBombGenerator(){}
TCompBombGenerator::~TCompBombGenerator(){}

void TCompBombGenerator::loadFromAtts(const std::string& elem, MKeyValue &atts) {
}

void TCompBombGenerator::init(){
	generateBomb();
}

void TCompBombGenerator::onGenerateBomb(const TMsgGenerateBomb& msg){
	generateBomb();
}

void TCompBombGenerator::generateBomb(){
	
	dbg("Entra en el generate bomb");

	// Get the generator position
	CEntity* mEntity = ((CEntity*)CHandle(this).getOwner());
	TCompTransform* comp_transform = mEntity->get<TCompTransform>();

	CEntity* prefab_entity = nullptr;
	// Generate the bomb
	CHandle prefab_handle = prefabs_manager.getInstanceByName("bomb_regular");
	if (prefab_handle.isValid())
		prefab_entity = prefab_handle;

	if ((comp_transform) && (prefab_entity)){
		dbg("Crea bomba");

		TCompTransform* prefab_t = prefab_entity->get<TCompTransform>();
		if (prefab_t){
			prefab_t->init();
			prefab_t->teleport(comp_transform->position);
		}
		dbg("coloca bomb");

		TCompRigidBody* prefab_rb = prefab_entity->get<TCompRigidBody>();
		if (prefab_rb){
			prefab_rb->init();
		}
		dbg("init bomb");

		TCompParticleGroup* prefab_pg = prefab_entity->get<TCompParticleGroup>();
		if (prefab_pg)
			prefab_pg->init();
		dbg("init particle bomb");

		TCompExplosion* prefab_E = prefab_entity->get<TCompExplosion>();
		if (prefab_E)
		{
			prefab_E->init();
			prefab_E->setGenerator(CHandle(mEntity));
		}
		dbg("init set generator bomb");
	}
}
