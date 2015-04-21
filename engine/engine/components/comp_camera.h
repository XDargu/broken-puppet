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
	}

	void updateViewProjection() {
		TCompTransform* trans = (TCompTransform*)transform;

		view = XMMatrixLookAtRH(trans->position, trans->position + trans->getFront(), XMVectorSet(0, 1, 0, 1));
		view_projection = view * projection;
	}
};

#endif
