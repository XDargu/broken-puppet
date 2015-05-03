#ifndef INC_COMP_POINT_LIGHT_H_
#define INC_COMP_POINT_LIGHT_H_

#include "base_component.h"
#include "render/render.h"

struct TCompPointLight : TBaseComponent {
	float               radius;
	float               decay_factor;       // Amount of radius where energy stars to decay
	float               intensity;
	XMVECTOR            color;

	TCompPointLight() : radius(1.f), decay_factor(0.7f), intensity(1.0f) {
		color = DirectX::XMVectorSet(1, 1, 1, 1);
	}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		radius = atts.getFloat("radius", 1.0f);
		// Intensity of the lights starts to decay at 70% of the radius
		// We want that the amount of light at radius is zero
		decay_factor = atts.getFloat("decay_factor", 0.7f);
		intensity = atts.getFloat("intensity", 1.f);
		color = atts.getPoint("color");
	}

	void draw();
};

#endif