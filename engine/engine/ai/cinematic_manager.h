#ifndef INC_CINEMATIC_MANAGER_H_
#define INC_CINEMATIC_MANAGER_H_

#include "mcv_platform.h"
#include "rigid_animation.h"

class CCinematicDef {
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

class CCinematicManager : public CItemsByName < CCinematicDef > {
public:
	CRigidAnimation getInstanceByName(const char* cinematic_name, CHandle target_transform);
};
extern CCinematicManager cinematic_manager;

#endif
