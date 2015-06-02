#ifndef INC_IMPORTER_PARTICLE_GROUPS_H_
#define INC_IMPORTER_PARTICLE_GROUPS_H_

#include "mcv_platform.h"
#include "handle/handle.h"

class CParticleGroupDef {
public:
	std::string               name;
	std::string               xml_as_text;
	bool load(const char* name);
	void setName(const char *new_name) {
		name = new_name;
	}
	const std::string& getName() const { return name; }
};

class CImporterParticleGroups : public CXMLParser {
	std::string current_name;
	std::string current_xml;
public:

	std::vector<CParticleGroupDef> particle_group_definitions;

	CImporterParticleGroups() : CXMLParser() { current_name = ""; current_xml = ""; }

	void onStartElement(const std::string &elem, MKeyValue &atts);

	// Cuando se acaba el tag de xml
	void onEndElement(const std::string &elem);

	void addParticleGroupToEntity(CEntity* entity, std::string name);

	void destroy();
};

extern CImporterParticleGroups particle_groups_manager;


#endif