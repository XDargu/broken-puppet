#ifndef INC_COMP_SENSOR_DISTANCE_PLAYER_H_
#define INC_COMP_SENSOR_DISTANCE_PLAYER_H_

#include "base_component.h"
#include "comp_transform.h"
using namespace DirectX;

struct TCompSensorDistPlayer : TBaseComponent{
private:
	TCompTransform*		e_transform;
	TCompTransform*     p_transform;
	float radius;
public:
	void loadFromAtts(const std::string& elem, MKeyValue &atts);
	void init();

	float playerDistance();
};
#endif
