#ifndef INC_COMP_CAMERA_H_
#define INC_COMP_CAMERA_H_

#include "camera.h"
#include "base_component.h"
#include "comp_transform.h"

struct TCompCamera : public CCamera, TBaseComponent {
private:
	CHandle transform;
public:

	TCompCamera() : CCamera() {}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		zfar = atts.getFloat("zfar", 1000);
		znear = atts.getFloat("znear", 1);
		fov_in_radians = deg2rad(atts.getFloat("fov", 60));

		float width = atts.getFloat("width", (float)CApp::get().xres);
		float height = atts.getFloat("height", (float)CApp::get().yres);
		
		transform = assertRequiredComponent<TCompTransform>(this);

		TCompTransform* m_trans = transform;
		m_trans->lookAt(m_trans->position - m_trans->getUp(), m_trans->getUp());

		setViewport(0, 0, width, height);

		updateViewProjection();
	}

	void init() {		
	}

	void update(float elapsed) {
		// Update matrix
		updateViewProjection();
		TCompTransform* m_t = transform;
		position = m_t->position;
		front = m_t->getFront();
		right = -m_t->getLeft();
		up = m_t->getUp();
	}

	void updateViewProjection() {
		TCompTransform* trans = (TCompTransform*)transform;

		view = XMMatrixLookAtRH(trans->position, trans->position + trans->getFront(), XMVectorSet(0, 1, 0, 1));

		TTransform prev_trans = trans->getPrevTransform();

		XMVECTOR lerp_pos = XMVectorLerp(prev_trans.position, trans->position, 0.5f);
		XMVECTOR lerp_front = XMVectorLerp(prev_trans.getFront(), trans->getFront(), 0.5f);

		XMMATRIX prev_view = XMMatrixLookAtRH(
			lerp_pos,
			lerp_pos + lerp_front,
			XMVectorSet(0, 1, 0, 1));

		prev_view_projection = prev_view * projection;
		view_projection = view * projection;
	}

	void changeZFar(float new_zfar) {
		zfar = new_zfar;

		float width = (float)CApp::get().xres;
		float height = (float)CApp::get().yres;

		transform = assertRequiredComponent<TCompTransform>(this);
		setViewport(0, 0, width, height);

		updateViewProjection();
	}

	float getRotationOffset() {
		TCompTransform* trans = (TCompTransform*)transform;

		XMVECTOR diff = XMQuaternionMultiply(XMQuaternionInverse(trans->getPrevTransform().rotation), trans->rotation);

		float angle;
		XMVECTOR axis;
		XMQuaternionToAxisAngle(&axis, &angle, diff);

		return angle;
	}
};

#endif
