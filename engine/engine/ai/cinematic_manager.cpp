#include "mcv_platform.h"
#include <sstream>
#include "cinematic_manager.h"
#include "cinematic_parser.h"

CCinematicManager cinematic_manager;

bool CCinematicDef::load(const char* name) {

	char full_name[MAX_PATH];
	sprintf(full_name, "%s/%s.xml", "data/cinematics", name);

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

CRigidAnimation CCinematicManager::getInstanceByName(const char* cinematic_name, CHandle target_transform) {

	const CCinematicDef* df = getByName(cinematic_name);
	XASSERT(df, "No se encuentra la cinemática: %s", cinematic_name);

	CCinematicParser p;
	p.target_transform = target_transform;
	std::istringstream istr(df->xml_as_text);
	bool is_ok = p.xmlParseStream(istr, cinematic_name);
	XASSERT(is_ok, "Error al parsear la cinemática: %s", cinematic_name);

	return p.animation;
}
