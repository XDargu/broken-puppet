#ifndef INC_COMP_RECAST_AABB_H_
#define INC_COMP_RECAST_AABB_H

#include "base_component.h"
#include "aabb.h"

struct TCompRecastAABB : TBaseComponent {

	CHandle m_transform;
	CHandle m_aabb;
	int index;

	TCompRecastAABB();
	~TCompRecastAABB();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	int getIndex();

	void init();

};
#endif

