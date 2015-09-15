#ifndef INC_LOCALIZATION_PARSER_H_
#define INC_LOCALIZATION_PARSER_H_

#include "XMLParser.h"

class CLocalizationParser : public CXMLParser {
public:

	CLocalizationParser() : CXMLParser() { }

	void onStartElement(const std::string &elem, MKeyValue &atts);
};

#endif