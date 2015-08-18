#include "mcv_platform.h"
#include "camera.h"

using namespace DirectX;

// -----------------------------------------------
CCamera::CCamera() 
	: fov_in_radians( deg2rad( 60.f ) )
	, znear( 1.0f )
	, zfar( 1000.f )
{
	projection = XMMatrixIdentity();
	setViewport(0, 0, 512, 512);    // Will update projection matrix
}

// -----------------------------------------------
void CCamera::setPerspective(float new_fov_in_rad, float new_znear, float new_zfar) {

	projection = XMMatrixPerspectiveFovRH(new_fov_in_rad, aspect_ratio, new_znear, new_zfar);

	fov_in_radians = new_fov_in_rad;
	znear = new_znear;
	zfar = new_zfar;
}

// -----------------------------------------------
void CCamera::setViewport(float x0, float y0, float width, float height) {
	viewport.TopLeftX = x0;
	viewport.TopLeftY = y0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;

	aspect_ratio = width / height;

	setPerspective(fov_in_radians, znear, zfar);
}

XMVECTOR CCamera::getWorldCoords(float norm_x, float norm_y) {
	XMVECTOR det;
	XMMATRIX inv_view_proj = XMMatrixInverse(&det, view_projection);

	XMVECTOR pos = XMVectorSet( 2 * norm_x - 1, 2 * (1 - norm_y) - 1, 0, 0);
	XMVECTOR world_coords = XMVector3TransformCoord(pos, inv_view_proj);
	
	return world_coords;
}

// -----------------------------------------------
bool CCamera::getScreenCoords(XMVECTOR world_coord, float *x, float *y) const {
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

void CCamera::setOcclusionViewport(float x0, float y0, float width_at_znear, float height_at_znear, float new_znear, float new_zfar) {
	viewport.TopLeftX = x0;
	viewport.TopLeftY = y0;
	viewport.Width = width_at_znear;
	viewport.Height = height_at_znear;
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 1.f;

	aspect_ratio = width_at_znear / height_at_znear;

	setOcclusionProjection(width_at_znear, height_at_znear, new_znear, new_zfar);
}

void CCamera::setOcclusionProjection(float width_at_znear, float height_at_znear, float new_znear, float new_zfar) {
	projection = XMMatrixPerspectiveRH(width_at_znear, height_at_znear, new_znear, new_zfar);

	//fov_in_radians = new_fov_in_rad;
	znear = new_znear;
	zfar = new_zfar;
}

void CCamera::setOcclusionViewProjection(XMVECTOR eye_position, XMVECTOR target_position, XMVECTOR the_front) {

	view = XMMatrixLookToRH(eye_position, the_front, XMVectorSet(0, 1, 0, 1));
	view_projection = view * projection;
}