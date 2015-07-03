#include "mcv_platform.h"
#include "culling.h"

void VPlanes::create(XMMATRIX view_proj) {
		clear();

		XMMATRIX vpt = XMMatrixTranspose(view_proj);
		XMVECTOR x = vpt.r[0];
		XMVECTOR y = vpt.r[1];
		XMVECTOR z = vpt.r[2];
		XMVECTOR w = vpt.r[3];
		push_back(XMPlaneNormalize(w + x));
		push_back(XMPlaneNormalize(w - x));
		push_back(XMPlaneNormalize(w + y));
		push_back(XMPlaneNormalize(w - y));
		push_back(XMPlaneNormalize(w + z));
		push_back(XMPlaneNormalize(w - z));
	}

bool VPlanes::isVisible(XMVECTOR coord) const {
		// We must be in the positive part of all the planes
		for (auto p = begin(); p != end(); ++p) {
			if (XMVectorGetX(XMPlaneDotCoord(*p, coord)) < 0)
				return false;
		}
		return true;
	}

bool VPlanes::isVisible(AABB* aabb) const {
	// We must be in the positive part of all the planes
	for (auto p = begin(); p != end(); ++p) {

		XMVECTOR abs_n = XMVectorMax(*p, -(*p));

		// Classifies aabb as per the plane: intersection, in back side, or in front side
		XMVECTOR r = XMPlaneDotNormal(aabb->getExtents(), abs_n);
		float fr = XMVectorGetX(r);

		// Distance from box center to the plane
		XMVECTOR c = XMPlaneDotCoord(*p, aabb->getCenter());
		float fc = XMVectorGetX(c);

		// Intersects
		if (fabsf(fc) < fr)
			continue;

		// Outside
		if (fc < fr)
			return false;

	}

	return true;
}


bool VPlanes::isInside(AABB* aabb) const {
	// We must be in the positive part of all the planes
	for (auto p = begin(); p != end(); ++p) {

		XMVECTOR abs_n = XMVectorMax(*p, -(*p));

		// Classifies aabb as per the plane: intersection, in back side, or in front side
		XMVECTOR r = XMPlaneDotNormal(aabb->getExtents(), abs_n);
		float fr = XMVectorGetX(r);

		// Distance from box center to the plane
		XMVECTOR c = XMPlaneDotCoord(*p, aabb->getCenter());
		float fc = XMVectorGetX(c);

		// Outside
		if (fc < fr)
			return false;

		// Intersects
		if (fabsf(fc) < fr)
			return false;

	}

	return true;
}


void OcclusionPlane::create(TTransform a_plane, float the_width, float the_height) {

	plane = a_plane;

	width = the_width;
	height = the_height;

	XMVECTOR occlusion_plane_front;
	XMVECTOR occlusion_plane_up;
	XMVECTOR occlusion_plane_left;
	//if (plane.isInUp(c_t->position)) {
	if (true) {
		// Occlusion plane front is front
		occlusion_plane_front = -plane.getUp();
		occlusion_plane_up = -plane.getLeft();
		occlusion_plane_left = -plane.getFront();
	}
	else {
		// Occlusion plane front is back
		occlusion_plane_front = plane.getUp();
		occlusion_plane_up = -plane.getLeft();
		occlusion_plane_left = plane.getFront();
	}

	// Make occlusion planes
	// Get plane points
	// Front is -up, Left is left, Up is -front
	left_up = plane.position + occlusion_plane_left * width * 0.5f + occlusion_plane_up * height * 0.5f;
	right_up = plane.position - occlusion_plane_left * width * 0.5f + occlusion_plane_up * height * 0.5f;
	left_down = plane.position + occlusion_plane_left * width * 0.5f - occlusion_plane_up * height * 0.5f;
	right_down = plane.position - occlusion_plane_left * width * 0.5f - occlusion_plane_up * height * 0.5f;
}

void OcclusionPlane::update(XMVECTOR eye_position) {

	clear();

	XMVECTOR occlusion_plane_front;
	XMVECTOR occlusion_plane_up;
	XMVECTOR occlusion_plane_left;
	if (plane.isInUp(eye_position)) {
		// Occlusion plane front is front
		occlusion_plane_front = -plane.getUp();
		occlusion_plane_up = -plane.getLeft();
		occlusion_plane_left = -plane.getFront();
	}
	else {
		// Occlusion plane front is back
		occlusion_plane_front = plane.getUp();
		occlusion_plane_up = -plane.getLeft();
		occlusion_plane_left = plane.getFront();
	}

	// Make occlusion planes
	// Get plane points
	// Front is -up, Left is left, Up is -front
	left_up = plane.position + occlusion_plane_left * width * 0.5f + occlusion_plane_up * height * 0.5f;
	right_up = plane.position - occlusion_plane_left * width * 0.5f + occlusion_plane_up * height * 0.5f;
	left_down = plane.position + occlusion_plane_left * width * 0.5f - occlusion_plane_up * height * 0.5f;
	right_down = plane.position - occlusion_plane_left * width * 0.5f - occlusion_plane_up * height * 0.5f;

	// Aux vector
	XMVECTOR eye_to_point;

	// Z near plane
	XMVECTOR plane1 = XMPlaneFromPointNormal(plane.position, occlusion_plane_front);
	// Z far plane
	XMVECTOR plane2 = XMPlaneFromPointNormal(plane.position + occlusion_plane_front * 100, -occlusion_plane_front);
	// Left plane
	eye_to_point = XMVector3Normalize(left_up - eye_position);
	XMVECTOR plane3 = XMPlaneFromPointNormal(left_up, XMVector3Cross(occlusion_plane_up, eye_to_point));
	// Right plane
	eye_to_point = XMVector3Normalize(right_up - eye_position);
	XMVECTOR plane4 = XMPlaneFromPointNormal(right_up, XMVector3Cross(-occlusion_plane_up, eye_to_point));
	// Up plane
	eye_to_point = XMVector3Normalize(left_up - eye_position);
	XMVECTOR plane5 = XMPlaneFromPointNormal(left_up, XMVector3Cross(-occlusion_plane_left, eye_to_point));
	// Bottom plane
	eye_to_point = XMVector3Normalize(left_down - eye_position);
	XMVECTOR plane6 = XMPlaneFromPointNormal(left_down, XMVector3Cross(occlusion_plane_left, eye_to_point));

	push_back(plane1);
	push_back(plane2);
	push_back(plane3);
	push_back(plane4);
	push_back(plane5);
	push_back(plane6);
}