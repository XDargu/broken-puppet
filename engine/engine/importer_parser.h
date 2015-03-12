#ifndef INC_IMPORTER_PARSER_H_
#define INC_IMPORTER_PARSER_H_

#include "handle\handle.h"
#include "XMLParser.h"

class CImporterParser : public CXMLParser {
	CHandle current_entity;
	CHandle current_comp;
	CHandle root_handle;
public:
	void onStartElement(const std::string &elem, MKeyValue &atts);

	// Cuando se acaba el tag de xml
	void onEndElement(const std::string &elem);

	CHandle getRootHandle() const { return root_handle; }
};

#endif