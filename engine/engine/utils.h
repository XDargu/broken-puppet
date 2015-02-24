#ifndef INC_UTILS_H_
#define INC_UTILS_H_

int fatal(const char* fmt, ...);
void dbg(const char* fmt, ...);

// Angle utils
float deg2rad(float deg);
float rad2deg(float rad);
float getYawFromVector(XMVECTOR front);
XMVECTOR getVectorFromYaw(float yaw);

float getPitchFromVector(XMVECTOR front);


bool isKeyPressed(int key);

bool vectorEqual(XMVECTOR v1, XMVECTOR v2);
int getRandomNumber(int num1, int num2);

#endif
