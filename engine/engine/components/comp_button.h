#ifndef INC_COMP_GUI_BUTTON_H
#define INC_COMP_GUI_BUTTON_H

#include "base_component.h"

struct TCompButton : TBaseComponent {
private:
	char scene_change_name[128];
	bool exit_action;
	bool hovering;
	bool start_game_action;
public:

	TCompButton(){
		scene_change_name[0] = 0x00;
		exit_action = false;
		start_game_action = false;
	}

	void loadFromAtts(const std::string& elem, MKeyValue& atts);

	void init();

	void update(float elapsed);

};

#endif