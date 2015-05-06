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
};

struct TCompSensorNeedles : TBaseComponent{
private:
	CHandle		transform;
	unsigned int numNeedlesInRange;
	float radius;
public:
	void loadFromAtts(const std::string& elem, MKeyValue &atts);
	void init();
	void getNeedlesInRange(std::vector<needle_rope>* needlesInRange);
	void needleInRange(std::vector<needle_rope>* needlesInRange, XMVECTOR pos, float radius);
};
#endif
