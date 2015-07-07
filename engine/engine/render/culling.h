#ifndef INC_CULLING_H_
#define INC_CULLING_H_

using namespace DirectX;
#include "aabb.h"
#include "transform.h"
/*struct TAABB {
	XMVECTOR center;
	XMVECTOR half;

	TAABB() {
		center = XMVectorZero();
		half = XMVectorZero();
	}

	void setMinMax(XMVECTOR pmin, XMVECTOR pmax) {
		center = (pmax + pmin) * 0.5f;
		half = (pmax - pmin) * 0.5f;
	}

	// center +/- half_size
	// Model * ( center +/- halfsize ) = model * center + model * half_size
	TAABB(const TAABB &aabb, const XMMATRIX world) {
		center = XMVector3TransformCoord(aabb.center, world);
		XMFLOAT3 aabb_half;
		XMStoreFloat3(&aabb_half, aabb.half);

		XMFLOAT3 wx, wy, wz;
		XMStoreFloat3(&wx, world.r[0]);
		XMStoreFloat3(&wy, world.r[1]);
		XMStoreFloat3(&wz, world.r[2]);

		XMFLOAT4 new_half;
		new_half.x = aabb_half.x * fabsf(wx.x)
			+ aabb_half.y * fabsf(wy.x)
			+ aabb_half.z * fabsf(wz.x);
		new_half.y = aabb_half.x * fabsf(wx.y)
			+ aabb_half.y * fabsf(wy.y)
			+ aabb_half.z * fabsf(wz.y);
		new_half.z = aabb_half.x * fabsf(wx.z)
			+ aabb_half.y * fabsf(wy.z)
			+ aabb_half.z * fabsf(wz.z);
		new_half.w = 1.0;
		half = XMLoadFloat4(&new_half);
	}

};*/

// Conjunto de planos
class VPlanes : public std::vector < XMVECTOR > {
public:

	void create(XMMATRIX view_proj);

	bool isVisible(XMVECTOR coord) const;

	bool isVisible(AABB* aabb) const;

	bool isInside(AABB* aabb) const;
};

class OcclusionPlane : public VPlanes {
	XMVECTOR left_up;
	XMVECTOR right_up;
	XMVECTOR left_down;
	XMVECTOR right_down;
	TTransform plane;
	float width;
	float height;
public:
	void create(TTransform a_plane, float the_width, float the_height);
	void update(XMVECTOR eye_position);

};

// AABB
//   AABBworld = AABBlocal * TRansform




#endif
