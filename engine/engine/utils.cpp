#include "mcv_platform.h"

using namespace DirectX;

// -----------------------------------------
float deg2rad(float deg) { return deg * (float)M_PI / 180.f; }
float rad2deg(float rad) { return rad * (180.f) / ((float)M_PI); }
//float getDeltaYaw(float yaw_src, float yaw_dst) {
//}


// -----------------------------------------
float getYawFromVector(XMVECTOR v) {
  float x = XMVectorGetX(v);
  float z = XMVectorGetZ(v);
  return atan2f(x, z);
}

XMVECTOR getVectorFromYaw(float yaw) {
  return XMVectorSet(sinf( yaw ), 0.f, cosf( yaw ), 0.f );
}

float getPitchFromVector(XMVECTOR v) {
	float y = -XMVectorGetY(v);
	float distance = sqrt(XMVectorGetZ(v) * XMVectorGetZ(v) + XMVectorGetX(v) * XMVectorGetX(v));
	return atan2(y, distance);
}

// -----------------------------------------
bool isKeyPressed(int key) {
  return ( ::GetAsyncKeyState(key) & 0x8000 ) != 0;
}

// -----------------------------------------
int fatal(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char buf[512];
  size_t n = vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
  va_end(ap);
  ::OutputDebugString(buf);
  assert(!printf("%s", buf));
  return 0;
}

// -----------------------------------------
void dbg(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char buf[512];
  size_t n = vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
  ::OutputDebugString(buf);
  va_end(ap);
}

bool vectorEqual(XMVECTOR v1, XMVECTOR v2) {
	return XMVectorGetX(XMVectorEqual(v1, v2)) && XMVectorGetY(XMVectorEqual(v1, v2)) && XMVectorGetZ(XMVectorEqual(v2, v2));
}