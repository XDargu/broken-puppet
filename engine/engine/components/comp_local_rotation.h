#ifndef INC_COMP_LOCAL_ROTATION_H_
#define INC_COMP_LOCAL_ROTATION_H_

#include "base_component.h"

struct TCompLocalRotation : TBaseComponent {
	// 0: Front
	// 1: Right
	// 2: Up
	int axis;
	float speed;
	CHandle h_transform;

	TCompLocalRotation() : speed(1), axis(0) {}
	void loadFromAtts(const std::string& elem, MKeyValue &atts);
	void update(float elapsed);

};

#endif
