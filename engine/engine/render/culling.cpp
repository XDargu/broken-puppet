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

		if (fabsf(fc) < fr)
			continue;

		if (fc < fr)
			return false;

	}

	return true;
}




