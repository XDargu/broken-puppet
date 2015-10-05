#ifndef INC_COMP_CAMERA_PIVOT_CONTROLLER_H_
#define INC_COMP_CAMERA_PIVOT_CONTROLLER_H_

#include "base_component.h"
#include "physics_manager.h"

struct TCompCameraPivotController : TBaseComponent {
private:
	CHandle m_transform;
	CHandle player_pivot_transform;
public:

	float tilt_velocity;
	float min_tilt;
	float max_tilt;

	physx::PxVec3 offset;

	TCompCameraPivotController() : tilt_velocity(deg2rad(30.0f)), min_tilt(deg2rad(-75)), max_tilt(deg2rad(75)) {}

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);

	void pointAt(XMVECTOR target);
	void aimAt(XMVECTOR target, float t);

};

#endif