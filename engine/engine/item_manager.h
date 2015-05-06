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
	void asociateTargetNeedle(XMVECTOR pos, float radius, CHandle grandma);
	CHandle getNeedleAsociated(CHandle grandma);
	CHandle getRopeAsociated(CHandle grandma);
	int getNumInRangle(XMVECTOR pos, float radius);
	//std::vector<TCompNeedle*> needleInRange(XMVECTOR pos, float radius);
	void removeNeedle(needle_rope n);
	static Citem_manager& get();
};
#endif

