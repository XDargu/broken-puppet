#ifndef INC_COMP_GUI_BUTTON_H
#define INC_COMP_GUI_BUTTON_H

#include "base_component.h"

struct TCompButton : TBaseComponent {
private:
	std::string scene_change_name;
	bool exit_action;
public:

	TCompButton(){
		scene_change_name = "";
		exit_action = false;
	}

	void loadFromAtts(const std::string& elem, MKeyValue& atts);

	void init();

	void update(float elapsed);

};

#endif