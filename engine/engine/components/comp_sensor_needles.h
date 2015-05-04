#ifndef INC_COMP_SENSOR_NEEDLES_H_
#define INC_COMP_SENSOR_NEEDLES_H_

#include "base_component.h"
#include "../item_manager.h"
#include "comp_transform.h"
#include "comp_needle.h"

struct TCompSensorNeedles : TBaseComponent{
private:
	CHandle		transform;
	unsigned int numNeedlesInRange;
	float radius;
public:
	std::vector<TCompNeedle*> needlesInRange;
	void loadFromAtts(const std::string& elem, MKeyValue &atts);
	void init();
	void getNeedlesInRange(std::vector<TCompNeedle*>* needlesInRange);
	void needleInRange(XMVECTOR pos, float radius);
};
#endif
