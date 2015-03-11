#ifndef INC_COMP_POINT_LIGHT_H_
#define INC_COMP_POINT_LIGHT_H_

#include "base_component.h"

struct TCompPointLight : TBaseComponent {
	XMVECTOR color;
	float radius;
	float intensity;

	TCompPointLight() {}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		CEntity* e = CHandle(this).getOwner();
		CHandle transform = e->get<TCompTransform>();

		TCompTransform* trans = (TCompTransform*)transform;

		assert(trans || fatal("TCompPointLight requieres a TCompTransform component"));

		intensity = atts.getFloat("intensity", 0.5f);
		color = atts.getQuat("color");
		radius = atts.getFloat("radius", 20.0f);
	}
};

#endif