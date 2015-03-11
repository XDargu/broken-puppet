#ifndef INC_COMP_CAMERA_H_
#define INC_COMP_CAMERA_H_

#include "camera.h"
#include "base_component.h"
#include "comp_transform.h"

struct TCompCamera : TBaseComponent {
private:
	CHandle transform;
public:

	XMMATRIX       view;            // Where is and where is looking at
	XMMATRIX       projection;      // Prespective info
	XMMATRIX       view_projection;

	XMVECTOR       target;          // Where we are looking at

	float          fov_in_radians;  // Field of view in radians
	float          aspect_ratio;
	float          znear, zfar;

	D3D11_VIEWPORT viewport;

	TCompCamera()
		: fov_in_radians(deg2rad(60.f))
		, znear(1.0f)
		, zfar(1000.f)
	{
		projection = XMMatrixIdentity();
	}

	void loadFromAtts(const std::string& elem, MKeyValue &atts) {
		target = atts.getPoint("target");
		zfar = atts.getFloat("zfar", 1000);
		znear = atts.getFloat("znear", 1);
		fov_in_radians = deg2rad(atts.getFloat("fov", 60));
	}

	void init() {
		CEntity* e = CHandle(this).getOwner();
		transform = e->get<TCompTransform>();

		TCompTransform* trans = (TCompTransform*)transform;

		assert(trans || fatal("TCamera requieres a TTransform component"));

		trans->lookAt(target, trans->getUp());
		setViewport(0, 0, CApp::get().xres, CApp::get().yres);
	}

	void update(float elapsed) {
		// Update matrix
		updateViewProjection();
	}

	// -----------------------------------------------
	void setPerspective(float new_fov_in_rad, float new_znear, float new_zfar) {

		projection = XMMatrixPerspectiveFovRH(new_fov_in_rad, aspect_ratio, new_znear, new_zfar);

		fov_in_radians = new_fov_in_rad;
		znear = new_znear;
		zfar = new_zfar;

		updateViewProjection();
	}

	// -----------------------------------------------
	void updateViewProjection() {
		TCompTransform* trans = (TCompTransform*)transform;

		view = XMMatrixLookAtRH(trans->position, trans->position + trans->getFront(), XMVectorSet(0, 1, 0, 1));
		view_projection = view * projection;
	}


	// -----------------------------------------------
	void setViewport(float x0, float y0, float width, float height) {
		viewport.TopLeftX = x0;
		viewport.TopLeftY = y0;
		viewport.Width = width;
		viewport.Height = height;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;

		aspect_ratio = width / height;

		setPerspective(fov_in_radians, znear, zfar);
	}


	// -----------------------------------------------
	bool getScreenCoords(XMVECTOR world_coord, float *x, float *y) const {
		XMVECTOR homo_coords = XMVector3TransformCoord(world_coord, view_projection);
		float sx = XMVectorGetX(homo_coords);
		float sy = XMVectorGetY(homo_coords);
		float sz = XMVectorGetZ(homo_coords);

		if (sz < 0.f || sz > 1.f)
			return false;

		*x = viewport.TopLeftX + (sx * 0.5f + 0.5f) * viewport.Width;
		*y = viewport.TopLeftY + (-sy * 0.5f + 0.5f) * viewport.Height;

		return true;
	}

	std::string toString() {
		return "Target: (" + std::to_string(XMVectorGetX(target)) + ", " + std::to_string(XMVectorGetY(target)) + ", " + std::to_string(XMVectorGetZ(target)) + ")";
	}
};

#endif
