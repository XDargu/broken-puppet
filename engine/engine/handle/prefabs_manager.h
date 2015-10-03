#ifndef INC_PREFABS_MANAGER_H_
#define INC_PREFABS_MANAGER_H_

#include "mcv_platform.h"
#include "handle/handle.h"

class CPrefabDef {
public:
	std::string               name;
	std::string               xml_as_text;
	bool load(const char* name);
	void setName(const char *new_name) {
		name = new_name;
	}
	const std::string& getName() const { return name; }

	void destroy() {}
};

class CPrefabsManager : public CItemsByName < CPrefabDef > {
public:
	CHandle getInstanceByName(const char* prefab_name);
};
extern CPrefabsManager prefabs_manager;

#endif
