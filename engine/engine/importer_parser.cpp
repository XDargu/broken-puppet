#include "mcv_platform.h"
#include "importer_parser.h"
#include "handle\entity.h"
#include "handle\handle_manager.h"
#include "handle/prefabs_manager.h"
#include "entity_manager.h"

void CImporterParser::onStartElement(const std::string &elem, MKeyValue &atts) {
	//dbg("onStart %s\n", elem.c_str());

	// Check if there is a handle manager with that name

	// If we are inside a component, send the xml tag to the component
	if (current_comp.isValid()) {
		current_comp.loadFromAtts(elem, atts);
		return;
	}

	// If not, check if it's a new component type
	CHandleManager* hm = CHandleManager::the_register.getByName(elem.c_str());
	if (!hm) {
		if (elem != "scene")
			dbg("unknown tag %s\n", elem.c_str());
		return;
	}

	bool reusing_component = false;

	CHandle h;
	if (elem == "entity" && atts.has("prefab")) {
		SET_ERROR_CONTEXT("Adding prefab", atts["prefab"].c_str());
		h = prefabs_manager.getInstanceByName(atts["prefab"].c_str());
	}
	else {

		if (current_entity.isValid()) {
			CEntity* e = current_entity;
			CHandle existing_comp = e->getByType(hm->getType());
			if (existing_comp.isValid()) {
				reusing_component = true;
				h = existing_comp;
			}
		}

		// If we are not reusing an existing component of the entity, create a new one
		if (!reusing_component)
			h = hm->createEmptyObj();
	}

	if (elem != "entity") {
		SET_ERROR_CONTEXT("Adding component", elem.c_str());
		XASSERT(current_entity.isValid(), "Entity not valid");
		CEntity* e = current_entity;
		if (!reusing_component)
			e->add(h);
		h.loadFromAtts(elem, atts);
		current_comp = h;
	}
	else {
		current_entity = h;
		current_entity.loadFromAtts(elem, atts);
	}

	// Keep a copy of the root handle
	if (!root_handle.isValid())
		root_handle = h;

}

// Cuando se acaba el tag de xml
void CImporterParser::onEndElement(const std::string &elem) {
	//dbg("onEnd %s\n", elem.c_str());
	if (elem == "entity") {
		auto it = std::find(CEntityManager::get().getEntities().begin(), CEntityManager::get().getEntities().end(), current_entity);
		if (it == CEntityManager::get().getEntities().end()) {
			CEntityManager::get().add(current_entity);
			current_entity = CHandle();
		}
	}
	else {
		if (current_comp.getTypeName() == elem) {
			current_comp = CHandle();
		}
	}
}