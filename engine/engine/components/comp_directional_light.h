#ifndef INC_COMP_DIRECTIONAL_LIGHT_H_
#define INC_COMP_DIRECTIONAL_LIGHT_H_

#include "base_component.h"

struct TCompDirectionalLight : TBaseComponent {
	XMVECTOR color;
	float intensity;

	TCompDirectionalLight() {}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		CEntity* e = CHandle(this).getOwner();
		CHandle transform = e->get<TCompTransform>();

		TCompTransform* trans = (TCompTransform*)transform;

		assert(trans || fatal("TCompDirectionalLight requieres a TCompTransform component"));

		intensity = atts.getFloat("intensity", 0.5f);
		color = atts.getQuat("color");
		color = XMVectorSetW(color, intensity * 0.1f);
	}
};

#endif