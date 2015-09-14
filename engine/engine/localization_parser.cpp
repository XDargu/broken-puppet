#include "mcv_platform.h"
#include "localization_parser.h"
#include "ai\logic_manager.h"

void CLocalizationParser::onStartElement(const std::string &elem, MKeyValue &atts) {
	if (elem == "subtitle") {
		CLogicManager::Subtitle subtitle;
		std::string guid = atts.getString("guid", "undefined");
		std::string sound = atts.getString("sound", "undefined");
		std::string text = atts.getString("text", "undefined");
		float time = atts.getFloat("time", 0);
		float size = atts.getFloat("size", 16);
		unsigned color = std::strtoul(atts.getString("color", "0xffffffff").c_str(), NULL, 16);

		subtitle.color = color;
		subtitle.sound = sound;
		subtitle.text = text;
		subtitle.time = time;
		subtitle.size = size;

		CLogicManager::get().addSubtitle(guid, subtitle);
	}
}