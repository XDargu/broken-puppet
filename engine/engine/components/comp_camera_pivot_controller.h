#ifndef INC_COMP_CAMERA_PIVOT_CONTROLLER_H_
#define INC_COMP_CAMERA_PIVOT_CONTROLLER_H_

#include "base_component.h"
#include "comp_transform.h"
#include "../io/iostatus.h"

struct TCompCameraPivotController : TBaseComponent {
private:
	CHandle m_transform;
	CHandle player_pivot_transform;
public:

	float tilt_velocity;
	float min_tilt;
	float max_tilt;

	physx::PxVec3 offset;

	TCompCameraPivotController() : tilt_velocity(deg2rad(90.0f)), min_tilt(deg2rad(-55)), max_tilt(deg2rad(55)) {}

	void loadFromAtts(MKeyValue &atts) {
		tilt_velocity = deg2rad(atts.getFloat("tiltVelocity", 90));
		offset = physx::PxVec3(0.5, 2, 0.5);
	}

	void init() {
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

	void update(float elapsed) {
		TCompTransform* player_pivot_trans = (TCompTransform*)player_pivot_transform;
		TCompTransform* transform = (TCompTransform*)m_transform;

		transform->position = player_pivot_trans->position + player_pivot_trans->getLeft() * -offset.x + player_pivot_trans->getUp() * offset.y + player_pivot_trans->getFront() * -offset.z;

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

	std::string toString() {
		return "Camera pivot controller";
	}
};

#endif