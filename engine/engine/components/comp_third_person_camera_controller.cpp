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
	
	XMVECTOR desired_pos = camera_pivot_trans->position + camera_pivot_trans->getLeft() * -offset.x + camera_pivot_trans->getUp() * offset.y + camera_pivot_trans->getFront() * (-offset.z);

	// Raycast camera
	float camera_dist = V3DISTANCE(desired_pos, camera_pivot_trans->position);
	float collision_dist = camera_dist;

	physx::PxRaycastBuffer buf;
	XMVECTOR ray_position = camera_pivot_trans->position;
	XMVECTOR ray_dir = XMVector3Normalize(desired_pos - ray_position);
	Physics.raycastAll(ray_position, ray_dir, camera_dist, buf);

	for (int i = 0; i < (int)buf.nbTouches; i++)
	{
		CEntity* e = CHandle(buf.touches[i].actor->userData);
		if (!e->hasTag("player")) {
			if (buf.touches[i].distance < collision_dist)
				collision_dist = buf.touches[i].distance;
		}
	}

	float distance_normalized = 1;

	collision_dist -= 0.3f;

	if (camera_dist != 0)
		distance_normalized = collision_dist / camera_dist;	
	
	XMVECTOR target_pos = XMVectorLerp(camera_pivot_trans->position, desired_pos, distance_normalized);
	//XMVECTOR target_pos = camera_pivot_trans->position + (camera_pivot_trans->getLeft() * -offset.x + camera_pivot_trans->getUp() * offset.y + camera_pivot_trans->getFront() * (-offset.z)) * collision_dist;

	float prev_y = XMVectorGetY(desired_pos);
	//transform->position = XMVectorLerp(transform->position, target_pos, 0.25f);
	transform->position = target_pos;
	XMVectorSetY(target_pos, prev_y);

	// LERP
	//XMVECTOR target_pos = camera_pivot_trans->position + camera_pivot_trans->getLeft() * -offset.x + camera_pivot_trans->getUp() * offset.y + camera_pivot_trans->getFront() * (-offset.z);
	XMVECTOR target_rot = camera_pivot_trans->rotation;
	transform->position = target_pos;// XMVectorLerp(transform->position, target_pos, 15.f * elapsed);
	transform->rotation = target_rot;// XMQuaternionSlerp(transform->rotation, target_rot, 15.f * elapsed);
	//transform->position = camera_pivot_trans->position + camera_pivot_trans->getLeft() * -offset.x + camera_pivot_trans->getUp() * offset.y + camera_pivot_trans->getFront() * (-offset.z + collision_dist);
	//transform->rotation = camera_pivot_trans->rotation;

}
