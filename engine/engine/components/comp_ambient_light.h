#ifndef INC_COMP_AMBIENT_LIGHT_H_
#define INC_COMP_AMBIENT_LIGHT_H_

#include "base_component.h"

struct TCompAmbientLight : TBaseComponent {

	XMVECTOR color;

	TCompAmbientLight() {}

	void loadFromAtts(MKeyValue &atts) {
		float intensity = atts.getFloat("intensity", 0.5f);
		color = atts.getQuat("color");
		color = XMVectorSetW(color, intensity * 0.1f);
	}
};

#endif