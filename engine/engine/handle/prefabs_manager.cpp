#include "mcv_platform.h"
#include <sstream>
#include "prefabs_manager.h"
#include "importer_parser.h"

CPrefabsManager prefabs_manager;

bool CPrefabDef::load(const char* name) {

	char full_name[MAX_PATH];
	sprintf(full_name, "%s/%s.xml", "data/prefabs", name);

	CFileDataProvider fdp(full_name);
	if (!fdp.isValid())
		return false;
	size_t file_size = fdp.getFileSize();
	char* buf = new char[file_size + 1];
	buf[file_size] = 0x0;
	fdp.read(buf, file_size);
	xml_as_text = std::string(buf);
	delete[] buf;
	size_t len = xml_as_text.length();
	return true;
}

CHandle CPrefabsManager::getInstanceByName(const char* prefab_name) {

	const CPrefabDef* df = getByName(prefab_name);
	assert(df);

	CImporterParser p;
	std::istringstream istr(df->xml_as_text);
	bool is_ok = p.xmlParseStream(istr, prefab_name);
	assert(is_ok);

	return p.getRootHandle();
}
