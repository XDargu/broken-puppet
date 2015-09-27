#ifndef INC_COMP_PARTICLE_EDITOR_H
#define INC_COMP_PARTICLE_EDITOR_H

#include "base_component.h"
#include <AntTweakBar.h>

struct TCompParticleEditor : TBaseComponent {
private:
	TwBar *particle_list_bar;
	bool random_move;
	bool random_rotate;
public:

	char aux_pg_name[128];
	char pg_selected_name[128];

	TCompParticleEditor() : random_move(false), random_rotate(false), particle_list_bar(nullptr) {
		aux_pg_name[0] = 0x00;
		pg_selected_name[0] = 0x00;
	}

	void loadFromAtts(const std::string& elem, MKeyValue& atts) {}

	void init();

	void update(float elapsed);

	void reloadParticleGroups();
};

#endif