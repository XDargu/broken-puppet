#include "mcv_platform.h"
#include "comp_particle_editor.h"
#include "comp_particle_group.h"
#include "entity_manager.h"
#include "particles\importer_particle_groups.h"
#include "comp_transform.h"
#include "entity_inspector.h"

void TW_CALL CallBackParticleGroupSelected(void *clientData) {
	CEntity* new_e = CEntityManager::get().getByName("edited_ps");
	particle_groups_manager.addParticleGroupToEntity(new_e, *static_cast<std::string *>(clientData));
	CEntityInspector::get().inspectEntity(CEntityInspector::get().getInspectedEntity());
}

void TW_CALL CallBackParticleGroupCreate(void *clientData) {
	CEntity* new_e = CEntityManager::get().getByName("edited_ps");
	TCompParticleGroup* e_pg = new_e->get<TCompParticleGroup>();	
	particle_groups_manager.addParticleGroupToEntity(new_e, "Default");

	CEntityInspector::get().inspectEntity(CEntityInspector::get().getInspectedEntity());
}

void TCompParticleEditor::init() {
	particle_list_bar = TwNewBar("ParticleEditor");

	int barSize[2] = { 224, 120 };
	int varPosition[2] = { 500, 320 };
	TwSetParam(particle_list_bar, NULL, "size", TW_PARAM_INT32, 2, barSize);
	TwSetParam(particle_list_bar, NULL, "position", TW_PARAM_INT32, 2, varPosition);
	TwDefine(" ParticleEditor label='Particle list' ");
	TwDefine(" ParticleEditor refresh='60' ");

	for (auto& pg : particle_groups_manager.particle_group_definitions) {
		if (pg.getName() != "Default") {
			TwAddButton(particle_list_bar, pg.getName().c_str(), CallBackParticleGroupSelected, &pg.name, "");
		}
	}

	TwAddButton(particle_list_bar, "New particle group", CallBackParticleGroupCreate, NULL, "");

}

void TCompParticleEditor::update(float elapsed) {

}