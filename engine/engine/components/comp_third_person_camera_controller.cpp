#include "mcv_platform.h"
#include "comp_third_person_camera_controller.h"
#include "comp_transform.h"
#include "io/iostatus.h"
#include "entity_manager.h"

void TCompThirdPersonCameraController::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	offset = Physics.XMVECTORToPxVec3(atts.getPoint("offset"));
}

void TCompThirdPersonCameraController::init() {
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

void TCompThirdPersonCameraController::update(float elapsed) {
	TCompTransform* camera_pivot_trans = (TCompTransform*)camera_pivot_transform;
	TCompTransform* transform = (TCompTransform*)m_transform;

	float collision_dist = 0;

	// Raycast camera
	physx::PxRaycastBuffer hit;
	XMVECTOR ray_position = camera_pivot_trans->position + camera_pivot_trans->getFront();
	XMVECTOR ray_dir = XMVector3Normalize(transform->position - ray_position);
	Physics.raycast(ray_position, ray_dir, offset.z, hit);

	if (hit.hasBlock) {
		physx::PxRaycastHit blockHit = hit.block;
		collision_dist = blockHit.distance + 0.2f;
	}

	transform->position = camera_pivot_trans->position + camera_pivot_trans->getLeft() * -offset.x + camera_pivot_trans->getUp() * offset.y + camera_pivot_trans->getFront() * (-offset.z + collision_dist);
	transform->rotation = camera_pivot_trans->rotation;

}
