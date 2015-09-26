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
	CEntityInspector::get().inspectEntity(new_e);
}

void TW_CALL CallBackParticleReset(void *clientData) {
	CEntity* ps_e = CEntityManager::get().getByName("edited_ps");
	TCompTransform* ps_t = ps_e->get<TCompTransform>();

	ps_t->position = XMVectorSet(0, 0, 0, 1);
	ps_t->rotation = XMQuaternionRotationAxis(XMVectorSet(1, 0, 0, 0), deg2rad(-90));
}

void TW_CALL CallBackParticleGroupDelete(void *clientData) {

	CEntity* e_editor = CEntityManager::get().getByName("ParticleEditor");
	TCompParticleEditor* pg_editor = e_editor->get<TCompParticleEditor>();

	CEntity* new_e = CEntityManager::get().getByName("edited_ps");
	
	particle_groups_manager.addParticleGroupToEntity(new_e, "Default");
	TCompParticleGroup* e_pg = new_e->get<TCompParticleGroup>();

	particle_groups_manager.removeByName(*static_cast<std::string *>(clientData));

	CEntityInspector::get().inspectEntity(new_e);

	pg_editor->reloadParticleGroups();

	particle_groups_manager.saveToDisk();
}

void TW_CALL CallBackParticleGroupCreate(void *clientData) {

	TCompParticleEditor* pg_editor = static_cast<TCompParticleEditor*>(clientData);

	// Validation of the name
	if (!particle_groups_manager.validateName(pg_editor->aux_pg_name))
		return;

	CEntity* new_e = CEntityManager::get().getByName("edited_ps");
	particle_groups_manager.addParticleGroupToEntity(new_e, "Default");

	TCompParticleGroup* e_pg = new_e->get<TCompParticleGroup>();

	CParticleGroupDef def;
	def.setName(pg_editor->aux_pg_name);
	def.xml_as_text = e_pg->getXMLDefinitionWithName(pg_editor->aux_pg_name);
	particle_groups_manager.particle_group_definitions.push_back(def);	
	
	CEntityInspector::get().inspectEntity(new_e);

	pg_editor->reloadParticleGroups();

	particle_groups_manager.saveToDisk();
}

void TCompParticleEditor::init() {
#ifdef _DEBUG
	particle_list_bar = TwNewBar("ParticleEditor");

	CApp &app = CApp::get();

	// AntTweakBar test
	int barSize[2] = { 224, app.yres };
	int varPosition[2] = { 0, 0 };
	TwSetParam(particle_list_bar, NULL, "size", TW_PARAM_INT32, 2, barSize);
	TwSetParam(particle_list_bar, NULL, "position", TW_PARAM_INT32, 2, varPosition);
	TwDefine(" ParticleEditor label='Particle list' ");
	TwDefine(" ParticleEditor refresh='60' ");


	reloadParticleGroups();
#endif
	CEntity* ps_e = CEntityManager::get().getByName("edited_ps");
	TCompTransform* ps_t = ps_e->get<TCompTransform>();

	ps_t->rotation = XMQuaternionRotationAxis(XMVectorSet(1, 0, 0, 0), deg2rad(-90));
}

void TCompParticleEditor::update(float elapsed) {
#ifdef _DEBUG

	// Hide console, listener and actioner bars
	TwDefine(" Lister visible=false ");
	TwDefine(" Actioner visible=false ");
	TwDefine(" Console visible=false ");
	TwDefine(" ConsoleInput visible=false ");

	CEntity* ps_e = CEntityManager::get().getByName("edited_ps");
	TCompTransform* ps_t = ps_e->get<TCompTransform>();

	if (random_move) {
		ps_t->position += getRandomVector3(-10, 10) * elapsed;
	}
	if (random_rotate) {
		ps_t->rotation = XMQuaternionMultiply(ps_t->rotation, XMQuaternionRotationRollPitchYawFromVector(getRandomVector3(-10 * elapsed, 10 * elapsed)));
	}
#endif
}

void TCompParticleEditor::reloadParticleGroups() {

	TwRemoveAllVars(particle_list_bar);

	TwAddVarRW(particle_list_bar, "CreationName", TW_TYPE_CSSTRING(sizeof(aux_pg_name)), &aux_pg_name, "group='Creation' label='Name'");
	TwAddButton(particle_list_bar, "Create new Particle Group", CallBackParticleGroupCreate, ((TCompParticleEditor*)this), "group='Creation'");
	TwAddSeparator(particle_list_bar, "CreationSeparator", "group='Creation'");

	TwAddSeparator(particle_list_bar, "MiscSeparator", "group='Misc'");
	TwAddVarRW(particle_list_bar, "Random movement", TW_TYPE_BOOL8, &random_move, "group='Misc'");
	TwAddVarRW(particle_list_bar, "Random rotation", TW_TYPE_BOOL8, &random_rotate, "group='Misc'");
	TwAddButton(particle_list_bar, "Reset", CallBackParticleReset, NULL, "group='Misc'");


	std::string aux = "";
	std::string aux2 = "";
	for (auto& pg : particle_groups_manager.particle_group_definitions) {

		if (pg.getName() != "Default") {

			aux = "group='" + pg.getName() + "' label='Edit'";
			aux2 = pg.getName() + "Edit";
			TwAddButton(particle_list_bar, aux2.c_str(), CallBackParticleGroupSelected, &pg.name, aux.c_str());

			aux = "group='" + pg.getName() + "'";
			aux2 = pg.getName() + "Separator1";
			TwAddSeparator(particle_list_bar, aux2.c_str(), aux.c_str());

			aux = "group='" + pg.getName() + "' label='Delete'";
			aux2 = pg.getName() + "Delete";
			TwAddButton(particle_list_bar, aux2.c_str(), CallBackParticleGroupDelete, &pg.name, aux.c_str());

			aux = "group='" + pg.getName() + "'";
			aux2 = pg.getName() + "Separator2";
			TwAddSeparator(particle_list_bar, aux2.c_str(), aux.c_str());
		}
	}
}