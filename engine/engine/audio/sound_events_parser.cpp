#include "mcv_platform.h"
#include "sound_events_parser.h"
#include "entity_manager.h"

void CSoundEventsParser::onStartElement(const std::string &elem, MKeyValue &atts) {
	if (elem == "event") {
		std::string id = atts.getString("id", "undefined");
		std::string path = atts.getString("path", "undefined");

		manager->sound_events[id] = path;
	}
}

// Cuando se acaba el tag de xml
void CSoundEventsParser::onEndElement(const std::string &elem) {

}