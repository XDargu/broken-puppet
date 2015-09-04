#ifndef INC_COMP_VIBRATION_H_
#define INC_COMP_VIBRATION_H_

#include "base_component.h"

struct TCompVibration : TBaseComponent {
	float	frequency;
	float	amount;
	CHandle h_transform;

	TCompVibration() : amount(0.01f), frequency(100) {}
	void loadFromAtts(const std::string& elem, MKeyValue &atts);
	void update(float elapsed);

};

#endif
