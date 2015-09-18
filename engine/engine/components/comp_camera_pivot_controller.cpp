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

	XMVECTOR orig_pos = transform->position;
	XMVECTOR orig_rot = transform->rotation;
	
	XMVECTOR desired_pos = player_pivot_trans->position + player_pivot_trans->getLeft() * -offset.x + player_pivot_trans->getUp() * offset.y + player_pivot_trans->getFront() * -offset.z;

	float y_diff = XMVectorGetY(player_pivot_trans->position) - XMVectorGetY(transform->position);
	// Raycast camera
	float camera_dist = V3DISTANCE(desired_pos, player_pivot_trans->position + player_pivot_trans->getUp() * offset.y);
	float collision_dist = camera_dist;

	physx::PxRaycastBuffer buf;
	XMVECTOR ray_position = player_pivot_trans->position + player_pivot_trans->getUp() * offset.y;
	XMVECTOR ray_dir = XMVector3Normalize(desired_pos - ray_position);
	Physics.raycastAll(ray_position, ray_dir, camera_dist, buf);

	for (int i = 0; i < (int)buf.nbTouches; i++)
	{
		CHandle h = CHandle(buf.touches[i].actor->userData);
		if (h.isValid()) {
			CEntity* e = h;
			if (e) {
				if (!e->hasTag("player")) {
					if (buf.touches[i].distance < collision_dist)
						collision_dist = buf.touches[i].distance;
				}
			}
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
	float m_pitch = getPitchFromVector(transform->getFront());
	XMVECTOR player_pivot_rot = XMQuaternionRotationAxis(player_pivot_trans->getUp(), player_pivot_yaw - m_yaw);

	CIOStatus &io = CIOStatus::get();

	CIOStatus::TMouse mouse = io.getMouse();

	// Rotación vertical que debo añadir
	float rot_increment = tilt_velocity * mouse.dy * 0.01f/* elapsed*/ * io.mouse_sensibility;
	float max_rot = max_tilt - m_pitch;
	float min_rot = min_tilt - m_pitch;

	rot_increment = clamp(rot_increment, min_rot, max_rot);

	XMVECTOR rot = XMQuaternionRotationAxis(transform->getLeft(), rot_increment);
	//rot = XMQuaternionSlerp(XMQuaternionIdentity(), rot, clamp(0.25, 0, 1));

	// Rotación mía + la que debo añadir
	transform->rotation = XMQuaternionMultiply(transform->rotation, rot);
	transform->rotation = XMQuaternionSlerp(orig_rot, transform->rotation, clamp(0.25, 0, 1));

	// Rotación mía + incremento de rotación del player pivot
	transform->rotation = XMQuaternionMultiply(transform->rotation, player_pivot_rot);
		
	float lerp_val = 0.8f;
	transform->position = XMVectorLerp(orig_pos, transform->position, clamp(lerp_val, 0, 1));
	//transform->rotation = XMQuaternionSlerp(orig_rot, transform->rotation, clamp(lerp_val, 0, 1));
	

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