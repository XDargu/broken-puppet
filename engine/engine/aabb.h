#ifndef INC_AABB_H_
#define INC_AABB_H_

#include "mcv_platform.h"

struct AABB {
	XMVECTOR min;
	XMVECTOR max;

	AABB() {
		min = XMVectorZero();
		max = XMVectorZero();
	}

	AABB(XMVECTOR the_min, XMVECTOR the_max) {
		min = the_min;
		max = the_max;
	}

	XMVECTOR getCenter() { return (min + max) / 2; };
	XMVECTOR getExtents() { return (max - min) / 2; };
	XMVECTOR getSize() { return max - min; };

	// Squared distance from a point to the AABB
	float sqrDistance(XMVECTOR point) {
		XMVECTOR nearestPoint = XMVectorZero();
		nearestPoint = XMVectorSetX(nearestPoint, (XMVectorGetX(point) <  XMVectorGetX(min)) ? XMVectorGetX(min) : (XMVectorGetX(point) > XMVectorGetX(max)) ? XMVectorGetX(max) : XMVectorGetX(point));
		nearestPoint = XMVectorSetY(nearestPoint, (XMVectorGetY(point) <  XMVectorGetY(min)) ? XMVectorGetY(min) : (XMVectorGetY(point) > XMVectorGetY(max)) ? XMVectorGetY(max) : XMVectorGetY(point));
		nearestPoint = XMVectorSetZ(nearestPoint, (XMVectorGetZ(point) <  XMVectorGetZ(min)) ? XMVectorGetZ(min) : (XMVectorGetZ(point) > XMVectorGetZ(max)) ? XMVectorGetZ(max) : XMVectorGetZ(point));

		return XMVectorGetX(XMVector3LengthSq(nearestPoint - point));
	}

	// Check if the AABB contains a point
	bool containts(XMVECTOR point) {
		return XMVector3InBounds(point - getCenter(), getExtents());
	}

	// Checks if the aabb intersects with another one
	bool intersects(AABB* aabb) {
		return XMVector3Greater(max, aabb->min) && XMVector3Greater(aabb->max, min);
	}
};

#endif