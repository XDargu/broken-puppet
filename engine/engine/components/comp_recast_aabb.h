#ifndef INC_COMP_RECAST_AABB_H_
#define INC_COMP_RECAST_AABB_H

#include "base_component.h"
#include "aabb.h"

struct TCompRecastAABB : TBaseComponent {

private:
	CHandle m_transform;
	int index;
	bool active;

public:
	CHandle m_aabb;
	TCompRecastAABB();
	~TCompRecastAABB();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void setIndex(int ind);
	int getIndex();
	bool getActive();
	void setActive(bool act);


	void init();

};
#endif

