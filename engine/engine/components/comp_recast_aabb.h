#ifndef INC_COMP_RECAST_AABB_H_
#define INC_COMP_RECAST_AABB_H

#include "base_component.h"

struct TCompRecastAABB : TBaseComponent {

	CHandle m_transform;
	CHandle m_aabb;

	TCompRecastAABB();
	~TCompRecastAABB();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);

};
#endif

