#ifndef INC_COMP_POINT_LIGHT_H_
#define INC_COMP_POINT_LIGHT_H_

#include "base_component.h"

struct TCompPointLight : TBaseComponent {

	XMVECTOR color;
	XMVECTOR position;
	float radius;

	TCompPointLight() {}

	void loadFromAtts(MKeyValue &atts) {
		float intensity = atts.getFloat("intensity", 0.5f);
		color = atts.getQuat("color");
		color = XMVectorSetW(color, intensity * 0.1f);
		position = atts.getQuat("position");
		radius = atts.getFloat("radius", 20.0f);

		
	}
};

#endif