#ifndef INC_COMP_THIRD_PERSON_CAMERA_CONTROLLER_H_
#define INC_COMP_THIRD_PERSON_CAMERA_CONTROLLER_H_

#include "base_component.h"
#include "comp_transform.h"
#include "../io/iostatus.h"

struct TCompThirdPersonCameraController : TBaseComponent {
private:
	CHandle m_transform;
	CHandle	camera_pivot_transform;
public:

	physx::PxVec3 offset;

	TCompThirdPersonCameraController() {}

	void loadFromAtts(MKeyValue &atts) {
		offset = Physics.XMVECTORToPxVec3(atts.getPoint("offset"));
	}

	void init() {
		CEntity* e_camera_pivot = CEntityManager::get().getByName("CameraPivot");

		assert(e_camera_pivot || fatal("TThirdPersonCameraController requieres a camera pivot entity"));

		camera_pivot_transform = e_camera_pivot->get<TCompTransform>();
		TCompTransform* camera_pivot_trans = (TCompTransform*)e_camera_pivot;

		assert(camera_pivot_trans || fatal("TThirdPersonCameraController requieres a camera pivot entity with a TTransform component"));

		CEntity* e = CHandle(this).getOwner();
		m_transform = e->get<TCompTransform>();
		TCompTransform* trans = (TCompTransform*)m_transform;

		assert(trans || fatal("TThirdPersonCameraController requieres a TTransform component"));
	}

	void update(float elapsed) {
		TCompTransform* camera_pivot_trans = (TCompTransform*)camera_pivot_transform;
		TCompTransform* transform = (TCompTransform*)m_transform;

		transform->position = camera_pivot_trans->position + camera_pivot_trans->getLeft() * -offset.x + camera_pivot_trans->getUp() * offset.y + camera_pivot_trans->getFront() * -offset.z;
		transform->rotation = camera_pivot_trans->rotation;
	}

	std::string toString() {
		return "Camera controller";
	}
};

#endif