#ifndef INC_COMP_DIRECTIONAL_LIGHT_H_
#define INC_COMP_DIRECTIONAL_LIGHT_H_

#include "base_component.h"

struct TCompDirectionalLight : TBaseComponent {

	XMVECTOR color;
	XMVECTOR direction;

	TCompDirectionalLight() {}

	void loadFromAtts(MKeyValue &atts) {
		float intensity = atts.getFloat("intensity", 0.5f);
		color = atts.getQuat("color");
		direction = atts.getQuat("direction");
		color = XMVectorSetW(color, intensity * 0.1f);
	}
};

#endif