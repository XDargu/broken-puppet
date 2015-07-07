#ifndef _LUA_QUATERNION_H_
#define _LUA_QUATERNION_H_

// Vectro class for LUA
class CQuaterion {
public:
	float x;
	float y;
	float z;
	float w;

	CQuaterion() : x(0), y(0), z(0), w(0) {}
	CQuaterion(float cx, float cy, float cz, float cw) : x(cx), y(cy), z(cz), w(cw) {}
	CQuaterion(XMVECTOR rot) { x = XMVectorGetX(rot); y = XMVectorGetY(rot); z = XMVectorGetZ(rot); w = XMVectorGetW(rot); }
};

#endif