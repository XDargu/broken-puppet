#ifndef INC_COMP_SENSOR_NEEDLES_H_
#define INC_COMP_SENSOR_NEEDLES_H_

#include "base_component.h"
#include "../item_manager.h"
#include "comp_transform.h"
#include "comp_needle.h"
#include "comp_rope.h"

struct needle_rope{
	TCompNeedle* needleRef;
	TCompRope*   rope_asociated;
	bool         call_it;
};

struct TCompSensorNeedles : TBaseComponent{
private:
	std::vector<needle_rope*> needlesInRangeVector;
	CHandle		transform;
	unsigned int numNeedlesInRange;
	float radius;
public:
	void loadFromAtts(const std::string& elem, MKeyValue &atts);
	void init();
	void getNeedlesInRange();
	void needleInRange(XMVECTOR pos, float radius);
	needle_rope* getTargetNeedle();
	int getNumNeedles();
};
#endif
