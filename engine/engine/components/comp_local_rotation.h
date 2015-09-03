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
	CHandle h_rigidbody;
	bool move_with_speed;
	XMVECTOR origin;

	TCompLocalRotation() : speed(1), axis(0), move_with_speed(false){}
	void loadFromAtts(const std::string& elem, MKeyValue &atts);
	void update(float elapsed);

};

#endif
