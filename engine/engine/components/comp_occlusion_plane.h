#ifndef INC_COMP_OCCLUSION_PLANE_H_
#define INC_COMP_OCCLUSION_PLANE_H_

#include "base_component.h"

struct TCompOcclusionPlane : TBaseComponent {

	TCompOcclusionPlane() { }
	~TCompOcclusionPlane() { }

	void loadFromAtts(const std::string& elem, MKeyValue &atts);
};

#endif
