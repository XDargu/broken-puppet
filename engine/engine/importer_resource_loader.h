#ifndef INC_IMPORTER_RESOURCE_LOADER_H_
#define INC_IMPORTER_RESOURCE_LOADER_H_

#include "XMLParser.h"

class CImporterResourceLoader : public CXMLParser {
public:

	CImporterResourceLoader() : CXMLParser() { }

	void onStartElement(const std::string &elem, MKeyValue &atts);
};

#endif