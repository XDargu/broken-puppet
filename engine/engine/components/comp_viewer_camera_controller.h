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
	float pan_speed;
	float camera_radius;
	float zoom_speed;
	float max_tilt;
	float min_tilt;
	XMVECTOR target;

	TCompViewerCameraController() {}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		transform = assertRequiredComponent<TCompTransform>(this);
		camera = assertRequiredComponent<TCompCamera>(this);

		h_speed = atts.getFloat("h_speed", 3);
		v_speed = atts.getFloat("v_speed", 3);
		pan_speed = atts.getFloat("pan_speed", 3);
		camera_radius = atts.getFloat("camera_radius", 10);
		zoom_speed = atts.getFloat("zoom_speed", 5);
		max_tilt = deg2rad(75);
		min_tilt = deg2rad(-75);

		target = XMVectorSet(0, 0, 0, 0);

		TCompTransform* o_transform = (TCompTransform*)transform;
		o_transform->rotation = XMQuaternionIdentity();

		o_transform->position = o_transform->getFront() * -camera_radius;
	}

	void init() {
		CApp::get().activateInspectorMode(true);
		CApp::get().renderAABB = false;
		CApp::get().renderAxis = false;
	}

	void update(float elapsed) {
		CIOStatus &io = CIOStatus::get();
		CIOStatus::TMouse mouse = io.getMouse();

		TCompTransform* o_transform = (TCompTransform*)transform;

		if (io.isPressed(CIOStatus::VIEWER_PAN)) {
			if (io.isPressed(CIOStatus::ALT)) {
				camera_radius += mouse.dy * elapsed * zoom_speed;
				if (camera_radius > 15)
					camera_radius = 15;
				if (camera_radius < 1)
					camera_radius = 1;
			}
			else {
				// Pan
				XMVECTOR front = o_transform->getFront();
				front = XMVector3Normalize(XMVectorSetY(front, 0));

				XMVECTOR right = -o_transform->getLeft();
				right = XMVector3Normalize(XMVectorSetY(right, 0));

				target += front * (float)mouse.dy * pan_speed * elapsed;
				target += -right * (float)mouse.dx * pan_speed * elapsed;
			}
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
		o_transform->position = target + o_transform->getFront() * -camera_radius;

		// Rotate light position, if exists
		CEntity* e_light = CEntityManager::get().getByName("the_light");
		if (e_light) {
			TCompTransform* l_transform = e_light->get<TCompTransform>();
			
			if (io.isPressed(CIOStatus::MOUSE_LEFT)) {

				XMVECTOR rot = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), -h_speed * mouse.dx * elapsed);
				//XMVECTOR rot2 = XMQuaternionRotationAxis(l_transform->getLeft(), v_speed * mouse.dy * elapsed);
				XMVECTOR rot2 = XMQuaternionIdentity();

				//o_transform->rotation = XMQuaternionMultiply(o_transform->rotation, rot);
				l_transform->rotation = XMQuaternionMultiply(XMQuaternionMultiply(l_transform->rotation, rot2), rot);

				// Clamp max and min camera Tilt
				float m_pitch = getPitchFromVector(l_transform->getFront());
				if (m_pitch > max_tilt)
					m_pitch = max_tilt;
				if (m_pitch < min_tilt)
					m_pitch = min_tilt;

				float m_yaw = getYawFromVector(l_transform->getFront());

				XMVECTOR m_rotation = XMQuaternionRotationRollPitchYaw(m_pitch, m_yaw, 0);
				l_transform->rotation = m_rotation;

				l_transform->position = l_transform->getFront() * -30;

			}
		}
	}
};

#endif