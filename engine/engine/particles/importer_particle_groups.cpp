#include "mcv_platform.h"
#include <sstream>
#include "importer_parser.h"
#include "importer_particle_groups.h"
#include "handle\entity.h"
#include "handle\handle_manager.h"
#include "handle/prefabs_manager.h"
#include "entity_manager.h"
#include "components\comp_particle_group.h"
#include "memory\data_saver.h"

CImporterParticleGroups particle_groups_manager;

void CImporterParticleGroups::onStartElement(const std::string &elem, MKeyValue &atts) {
	if (elem == "particleGroup") {
		current_name = atts.getString("name", "unnamed");
	}
	if (elem != "particleGroups") {
		current_xml += "<" + elem;
		for (auto& key : atts) {
			current_xml += " " + key.first + "=\"" + key.second + "\"";
		}

		current_xml += ">";
	}
}

// Cuando se acaba el tag de xml
void CImporterParticleGroups::onEndElement(const std::string &elem) {
	if (elem != "particleGroups") {
		current_xml += "</" + elem + ">";

		if (elem == "particleGroup") {
			// Add the current XML definition
			CParticleGroupDef def;
			def.setName(current_name.c_str());
			def.xml_as_text = current_xml;
			particle_group_definitions.push_back(def);
			current_xml = "";
		}
	}
}

void CImporterParticleGroups::addParticleGroupToEntity(CEntity* entity, std::string name) {
	for (auto& pg : particle_group_definitions) {
		if (pg.name == name) {
			CImporterParser p;
			p.setCurrentEntity(entity);
			TCompParticleGroup* e_pg = entity->get<TCompParticleGroup>();
			e_pg->def_name = name;
			if (e_pg) {
				e_pg->clearParticleSystems();
			}				
			std::istringstream istr(pg.xml_as_text);
			bool is_ok = p.xmlParseStream(istr, pg.getName().c_str());
			XASSERT(is_ok, "Error al parsear el grupo de partículas: %s", pg.getName().c_str());
		}
	}
}

void CImporterParticleGroups::updateParticleGroupFromEntity(CEntity* entity, std::string name) {
	for (auto& pg : particle_group_definitions) {
		if (pg.name == name) {
			TCompParticleGroup* e_pg = entity->get<TCompParticleGroup>();
			pg.xml_as_text = e_pg->getXMLDefinitionWithName(name);
		}
	}
}

void CImporterParticleGroups::destroy() {
	particle_group_definitions.clear();
}

bool CImporterParticleGroups::validateName(std::string name) {
	if (name.empty())
		return false;

	for (auto& pg : particle_group_definitions) {
		if (pg.name == name) {
			return false;
		}
	}

	return true;
}

std::string CImporterParticleGroups::getXMLDefinition() {
	std::string def = "";

	def += "<particleGroups>";

	for (auto& pg : particle_group_definitions) {
		def += pg.xml_as_text;
	}

	def += "</particleGroups>";

	return def;
}

void CImporterParticleGroups::saveToDisk() {
	// Virtual file
	CStringDataSaver mds;
	std::string def = getXMLDefinition();	
	mds.write(&def, def.size());

	mds.saveToFile("data/particles/particle_groups.xml");
}

void CImporterParticleGroups::removeByName(std::string name) {

	std::vector<CParticleGroupDef>::iterator it = particle_group_definitions.begin();

	while (it != particle_group_definitions.end()) {

		if (it->name == name) {
			it = particle_group_definitions.erase(it);
		}
		else ++it;
	}
}