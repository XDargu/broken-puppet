#ifndef _LUA_VECTOR_H_
#define _LUA_VECTOR_H_

// Vectro class for LUA
class CVector {
public:
	float x;
	float y;
	float z;

	CVector() : x(0), y(0), z(0) {}
	CVector(float cx, float cy, float cz) : x(cx), y(cy), z(cz) {}
	CVector(XMVECTOR pos) { x = XMVectorGetX(pos); y = XMVectorGetY(pos); z = XMVectorGetZ(pos); }
};

#endif