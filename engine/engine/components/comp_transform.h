#ifndef INC_COMP_TRANSFORM_H_
#define INC_COMP_TRANSFORM_H_

#include "mcv_platform.h"
#include "base_component.h"
#include "transform.h"
#include "comp_name.h"

// ----------------------------------------
struct TCompTransform : public TTransform, TBaseComponent {     // 1

	TCompTransform() : TTransform() {}
	TCompTransform(XMVECTOR np, XMVECTOR nr, XMVECTOR ns) : TTransform(np, nr, ns) {}

	void loadFromAtts(const std::string& elem, MKeyValue& atts) {
		position = atts.getPoint("position");
		rotation = atts.getQuat("rotation");
		scale = atts.getPoint("scale");
	}
};

#endif
