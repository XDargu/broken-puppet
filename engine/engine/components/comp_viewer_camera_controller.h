#ifndef INC_COMP_VIEWER_CAMERA_CONTROLLER_H_
#define INC_COMP_VIEWER_CAMERA_CONTROLLER_H_

#include "base_component.h"
#include "../io/iostatus.h"

struct TCompViewerCameraController : TBaseComponent {
private:
	CHandle transform;
	CHandle camera;
public:

	float h_speed;
	float v_speed;
	float camera_radius;
	float zoom_speed;
	float max_tilt;
	float min_tilt;

	TCompViewerCameraController() {}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		transform = assertRequiredComponent<TCompTransform>(this);
		camera = assertRequiredComponent<TCompCamera>(this);

		h_speed = atts.getFloat("h_speed", 3);
		v_speed = atts.getFloat("v_speed", 3);
		camera_radius = atts.getFloat("camera_radius", 5);
		zoom_speed = atts.getFloat("zoom_speed", 5);
		max_tilt = deg2rad(75);
		min_tilt = deg2rad(-75);

		TCompTransform* o_transform = (TCompTransform*)transform;
		o_transform->rotation = XMQuaternionIdentity();

		o_transform->position = o_transform->getFront() * -camera_radius;
	}

	void update(float elapsed) {
		CIOStatus &io = CIOStatus::get();
		CIOStatus::TMouse mouse = io.getMouse();

		TCompTransform* o_transform = (TCompTransform*)transform;

		if (io.isPressed(CIOStatus::VIEWER_ZOOM)) {
			camera_radius += mouse.dy * elapsed * zoom_speed;
			if (camera_radius > 8)
				camera_radius = 8;
			if (camera_radius < 1)
				camera_radius = 1;
		}

		if (io.isPressed(CIOStatus::VIEWER_MOVE_CAM)) {			

			XMVECTOR rot = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), -h_speed * mouse.dx * elapsed);
			XMVECTOR rot2 = XMQuaternionRotationAxis(o_transform->getLeft(), v_speed * mouse.dy * elapsed);

			//o_transform->rotation = XMQuaternionMultiply(o_transform->rotation, rot);
			o_transform->rotation = XMQuaternionMultiply(XMQuaternionMultiply(o_transform->rotation, rot2), rot);

			// Clamp max and min camera Tilt
			float m_pitch = getPitchFromVector(o_transform->getFront());
			if (m_pitch > max_tilt)
				m_pitch = max_tilt;
			if (m_pitch < min_tilt)
				m_pitch = min_tilt;

			float m_yaw = getYawFromVector(o_transform->getFront());

			XMVECTOR m_rotation = XMQuaternionRotationRollPitchYaw(m_pitch, m_yaw, 0);
			o_transform->rotation = m_rotation;

		}

		// Update position
		o_transform->position = o_transform->getFront() * -camera_radius;
	}
};

#endif