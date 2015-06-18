#ifndef INC_COMP_RECAST_AABB_H_
#define INC_COMP_RECAST_AABB_H

#include "base_component.h"
#include "aabb.h"

struct TCompRecastAABB : TBaseComponent {

	CHandle m_transform;
	AABB m_aabb;
	XMVECTOR min;
	XMVECTOR max;

	TCompRecastAABB();
	~TCompRecastAABB();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);

};
#endif

