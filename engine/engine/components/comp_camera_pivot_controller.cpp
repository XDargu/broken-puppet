#include "mcv_platform.h"
#include "comp_camera_pivot_controller.h"
#include "io/iostatus.h"
#include "comp_transform.h"
#include "entity_manager.h"

void TCompCameraPivotController::loadFromAtts(const std::string& elem, MKeyValue &atts) {
		tilt_velocity = deg2rad(atts.getFloat("tiltVelocity", 90));
		offset = physx::PxVec3(0.5, 2, 0.5);
	}

void TCompCameraPivotController::init() {
	CEntity* e_player = CEntityManager::get().getByName("PlayerPivot");

	assert(e_player || fatal("TCompCameraPivotController requieres a player pivot entity"));

	player_pivot_transform = e_player->get<TCompTransform>();
	TCompTransform* player_pivot_trans = (TCompTransform*)player_pivot_transform;

	assert(player_pivot_trans || fatal("TCompCameraPivotController requieres a player pivot entity with a TTransform component"));

	CEntity* e = CHandle(this).getOwner();
	m_transform = e->get<TCompTransform>();
	TCompTransform* transform = (TCompTransform*)m_transform;

	transform->position = player_pivot_trans->position + player_pivot_trans->getLeft() * -offset.x + player_pivot_trans->getUp() * offset.y + player_pivot_trans->getFront() * -offset.z;
	transform->rotation = XMVectorSet(0, 0, 0, 1);

	assert(transform || fatal("TCompCameraPivotController requieres a TTransform component"));
}

void TCompCameraPivotController::update(float elapsed) {
	TCompTransform* player_pivot_trans = (TCompTransform*)player_pivot_transform;
	TCompTransform* transform = (TCompTransform*)m_transform;
	
	XMVECTOR desired_pos = player_pivot_trans->position + player_pivot_trans->getLeft() * -offset.x + player_pivot_trans->getUp() * offset.y + player_pivot_trans->getFront() * -offset.z;

	// Raycast camera
	float camera_dist = V3DISTANCE(desired_pos, player_pivot_trans->position);
	float collision_dist = camera_dist;

	physx::PxRaycastBuffer buf;
	XMVECTOR ray_position = player_pivot_trans->position;
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

	if (camera_dist != 0) {
		distance_normalized = collision_dist / camera_dist;
		distance_normalized -= 0.1f;
	}
	

	XMVECTOR target_pos = XMVectorLerp(player_pivot_trans->position, desired_pos, distance_normalized);

	float prev_y = XMVectorGetY(desired_pos);
	//transform->position = XMVectorLerp(transform->position, target_pos, 0.25f);
	transform->position = target_pos;
	XMVectorSetY(transform->position, prev_y);

	// Get player pivot Y rotation
	float player_pivot_yaw = getYawFromVector(player_pivot_trans->getFront());
	float m_yaw = getYawFromVector(transform->getFront());
	XMVECTOR player_pivot_rot = XMQuaternionRotationAxis(player_pivot_trans->getUp(), player_pivot_yaw - m_yaw);

	CIOStatus &io = CIOStatus::get();

	CIOStatus::TMouse mouse = io.getMouse();
	XMVECTOR rot = XMQuaternionRotationAxis(transform->getLeft(), tilt_velocity * mouse.dy * elapsed);
	transform->rotation = XMQuaternionMultiply(transform->rotation, rot);
	transform->rotation = XMQuaternionMultiply(transform->rotation, player_pivot_rot);

	// Clamp max and min camera Tilt
	float m_pitch = getPitchFromVector(transform->getFront());
	if (m_pitch > max_tilt)
		m_pitch = max_tilt;
	if (m_pitch < min_tilt)
		m_pitch = min_tilt;

	m_yaw = getYawFromVector(transform->getFront());

	XMVECTOR m_rotation = XMQuaternionRotationRollPitchYaw(m_pitch, m_yaw, 0);
	transform->rotation = m_rotation;

	// Clamp values with quaternion multiplication fails, why?????!!!
	//XMVECTOR max_pitch_correction = XMQuaternionRotationAxis(player_pivot_trans->getLeft(), max_tilt - m_pitch);
	//XMVECTOR min_pitch_correction = XMQuaternionRotationAxis(player_pivot_trans->getLeft(), m_pitch - min_tilt);
	//XMVECTOR max_pitch_correction = XMQuaternionRotationRollPitchYaw(m_pitch - max_tilt, 0, 0);
	//XMVECTOR min_pitch_correction = XMQuaternionRotationRollPitchYaw(m_pitch - min_tilt, 0, 0);

	/*if (m_pitch > max_tilt)
	transform->rotation = XMQuaternionMultiply(transform->rotation, max_pitch_correction);*/
	/*if (m_pitch > min_tilt)
	transform->rotation = XMQuaternionMultiply(transform->rotation, min_pitch_correction);*/

}

void TCompCameraPivotController::pointAt(XMVECTOR target) {
	TCompTransform* transform = (TCompTransform*)m_transform;
	TCompTransform* player_pivot_trans = (TCompTransform*)player_pivot_transform;

	// Get player pivot Y rotation
	float player_pivot_yaw = getYawFromVector(player_pivot_trans->getFront());
	XMVECTOR player_pivot_rot = XMQuaternionRotationAxis(player_pivot_trans->getUp(), player_pivot_yaw);

	transform->lookAt(target, player_pivot_trans->getUp());

	// Get only the Y axis rotation from yaw
	float camera_pivot_pitch = getPitchFromVector(transform->getFront());

	// Set the new rotation
	transform->rotation = XMQuaternionRotationRollPitchYaw(camera_pivot_pitch, player_pivot_yaw, 0);
}