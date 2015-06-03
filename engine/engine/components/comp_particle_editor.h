#ifndef INC_COMP_PARTICLE_EDITOR_H
#define INC_COMP_PARTICLE_EDITOR_H

#include "base_component.h"
#include <AntTweakBar.h>

struct TCompParticleEditor : TBaseComponent {
private:
	TwBar *particle_list_bar;
public:

	std::string aux_pg_name;
	std::string pg_selected_name;

	TCompParticleEditor() {}

	void loadFromAtts(const std::string& elem, MKeyValue& atts) {}

	void init();

	void update(float elapsed);

	void reloadParticleGroups();
};

#endif