#ifndef INC_COMP_ANIM_EDITOR_H
#define INC_COMP_ANIM_EDITOR_H

#include "base_component.h"
#include <AntTweakBar.h>

struct TCompAnimEditor : TBaseComponent {

	TCompAnimEditor(){}

	void loadFromAtts(const std::string& elem, MKeyValue& atts) {}

	void init();

	void update(float elapsed);

};

#endif