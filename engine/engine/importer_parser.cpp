#include "mcv_platform.h"
#include "importer_parser.h"
#include "entity.h"

CHandle current_entity;

void CImporterParser::onStartElement(const std::string &elem, MKeyValue &atts) {
	dbg("onStart %s\n", elem.c_str());

	// Check if there is a handle manager with that name
	CHandleManager* hm = CHandleManager::the_register.getByName(elem.c_str());
	if (!hm) {
		dbg("unknown tag %s\n", elem.c_str());
		return;
	}

	CHandle h = hm->createEmptyObj(atts);

	if (elem != "entity") {
		assert(current_entity.isValid());
		CEntity* e = current_entity;
		e->add(h);
	}
	else {
		current_entity = h;
	}
}

// Cuando se acaba el tag de xml
void CImporterParser::onEndElement(const std::string &elem) {
	dbg("onEnd %s\n", elem.c_str());
	if (elem == "entity") {
		CEntityManager::get().add(current_entity);
	}
}