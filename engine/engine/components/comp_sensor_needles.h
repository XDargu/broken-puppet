#ifndef INC_COMP_SENSOR_NEEDLES_H_
#define INC_COMP_SENSOR_NEEDLES_H_

#include "base_component.h"
#include "../item_manager.h"
#include "comp_transform.h"
#include "comp_needle.h"
#include "comp_rope.h"

struct needle_rope{
	CHandle      needleRef;
	CHandle      rope_asociated;
	CHandle      grandma_asociated;
};

struct TCompSensorNeedles : TBaseComponent{
private:
	CHandle		transform;
	unsigned int numNeedlesInRange;
	float radius;
public:
	void loadFromAtts(const std::string& elem, MKeyValue &atts);
	void init();
	//void getNeedlesInRange();
	void needleInRange(XMVECTOR pos, float radius);
	void asociateGrandmaTargetNeedle(CHandle gradmaRef);
	CHandle getNeedleAsociatedSensor(CHandle grandma);
	CHandle getRopeAsociatedSensor(CHandle grandma);
	int getNumNeedles();
};
#endif
