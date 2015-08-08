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
	needle_rope asociatedNeedle;
public:
	//TCompSensorNeedles(){ transform = CHandle(); numNeedlesInRange = 0; radius = 0; asociatedNeedle.grandma_asociated = CHandle(); asociatedNeedle.needleRef = CHandle(); asociatedNeedle.rope_asociated = CHandle(); };
	//~TCompSensorNeedles();
	void loadFromAtts(const std::string& elem, MKeyValue &atts);
	void init();
	//void getNeedlesInRange();
	//void needleInRange(XMVECTOR pos, float radius);
	//bool asociateGrandmaTargetNeedle(float max_dist_reach_needle);
    //bool asociateGrandmaTargetNeedle(CHandle gradmaRef, float max_dist_reach_needle);
	CHandle getNeedleAsociatedSensor(CHandle grandma);
	CHandle getRopeAsociatedSensor(CHandle grandma);
	int getNumNeedles(CHandle grandmaRef, float max_dist_reach_needle);
	void removeNeedleRope(CHandle grandmaRef);
	void setAsociatedNeedle(needle_rope needle_ref);
	needle_rope getAsociatedNeedle();
};
#endif
