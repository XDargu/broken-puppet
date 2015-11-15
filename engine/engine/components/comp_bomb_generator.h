#ifndef INC_COMP_BOMB_GENERATOR_H_
#define INC_COMP_BOMB_GENERATOR_H_

#include "base_component.h"
#include "handle\app_msgs.h"

struct TCompBombGenerator : TBaseComponent {


public:
	TCompBombGenerator();
	~TCompBombGenerator();

	void init();
	void onGenerateBomb(const TMsgGenerateBomb& msg);
	void loadFromAtts(const std::string& elem, MKeyValue &atts);

private:
	void generateBomb();

};

#endif
