#ifndef INC_SOUND_EVENTS_PARSER_H_
#define INC_SOUND_EVENTS_PARSER_H_

#include "XMLParser.h"
#include "sound_manager.h"

class CSoundEventsParser : public CXMLParser {
public:
	CSoundManager* manager;

	CSoundEventsParser(CSoundManager* the_manager) : CXMLParser() { manager = the_manager; }

	void onStartElement(const std::string &elem, MKeyValue &atts);

	// Cuando se acaba el tag de xml
	void onEndElement(const std::string &elem);
};

#endif