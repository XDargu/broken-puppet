#ifndef INC_COMP_ZONE_AABB_H_
#define INC_COMP_ZONE_AABB_H

#include "base_component.h"
#include "aabb.h"

struct TCompZoneAABB : TBaseComponent {
private:
	CHandle m_transform;
	CHandle m_aabb;
	CHandle m_name;
	CHandle player_transform;
	CHandle player;

public:
	TCompZoneAABB();
	~TCompZoneAABB();

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	CHandle getZoneName();

	bool isPlayerInside();

	void init();

};
#endif
