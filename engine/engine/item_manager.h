#ifndef INC_ITEM_MANAGER_H_
#define INC_ITEM_MANAGER_H_

#include "handle\handle.h"

struct needle_rope;

class Citem_manager
{
public:
	std::vector<needle_rope> needles;
	Citem_manager();
	~Citem_manager();
	void addNeedle(CHandle needle, CHandle rope);
	bool asociateTargetNeedle(XMVECTOR pos, float radius, CHandle grandma, float max_dist_reach_needle, float distance_change_way_point);
	CHandle getNeedleAsociated(CHandle grandma);
	CHandle getRopeAsociated(CHandle grandma);
	void DesAsociatePriorityNeedleRope(CHandle grandma);
	bool DesAsociateNoPriorityNeedleRope(CHandle grandma);
	int getNumInRangle(CHandle grandmaRef, XMVECTOR pos, float radius, float max_dist_reach_needle, float distance_change_way_point);
	//std::vector<TCompNeedle*> needleInRange(XMVECTOR pos, float radius);
	void removeNeedle(CHandle n);
	static Citem_manager& get();
	void clear();
};
#endif

