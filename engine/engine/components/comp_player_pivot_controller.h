#ifndef INC_COMP_PLAYER_PIVOT_CONTROLLER_H_
#define INC_COMP_PLAYER_PIVOT_CONTROLLER_H_

#include "base_component.h"

struct TCompPlayerPivotController : TBaseComponent {
private:
	CHandle m_transform;
	CHandle player_transform;
public:

	float rotation_velocity;

	TCompPlayerPivotController() : rotation_velocity(deg2rad(90.0f)) {}

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);

	void pointAt(XMVECTOR target);
};

#endif