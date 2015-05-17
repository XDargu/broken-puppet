#ifndef INC_COMP_THIRD_PERSON_CAMERA_CONTROLLER_H_
#define INC_COMP_THIRD_PERSON_CAMERA_CONTROLLER_H_

#include "base_component.h"
#include "physics_manager.h"

struct TCompThirdPersonCameraController : TBaseComponent {
private:
	CHandle m_transform;
	CHandle	camera_pivot_transform;
public:

	physx::PxVec3 offset;

	TCompThirdPersonCameraController() {}

	void loadFromAtts(const std::string& elem, MKeyValue &atts);

	void init();

	void update(float elapsed);
};

#endif