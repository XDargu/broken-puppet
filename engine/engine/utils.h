#ifndef INC_UTILS_H_
#define INC_UTILS_H_

void fatal(const char* fmt, ...);
void dbg(const char* fmt, ...);

// Angle utils
float deg2rad(float deg);
float rad2deg(float rad);
float getYawFromVector(XMVECTOR front);
float getPitchFromVector(XMVECTOR front);
XMVECTOR getVectorFromYaw(float yaw);

bool isKeyPressed(int key);

#endif
