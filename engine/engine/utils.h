#ifndef INC_UTILS_H_
#define INC_UTILS_H_

// Error

#define XASSERT(test, msg, ...) do {if (!(test)) error(__LINE__, __FILE__, "Assertion failed: " #test,\
        msg, __VA_ARGS__);} while (0)
#define XERROR(msg, ...) do { error(__LINE__, __FILE__, "Error thrown: ", msg, __VA_ARGS__);} while (0)
#define XDEBUG(msg, ...) do { debug(__LINE__, __FILE__, "Debug message: ", msg, __VA_ARGS__);} while (0)
#define SET_ERROR_CONTEXT(name, data) CErrorContext ec(name, data);

int fatal(const char* fmt, ...);
void dbg(const char* fmt, ...);

void error(int line, char* file, char* method, char* msg, ...);
void debug(int line, char* file, char* method, char* msg, ...);

// Angle utils
float deg2rad(float deg);
float rad2deg(float rad);
float getYawFromVector(XMVECTOR front);
XMVECTOR getVectorFromYaw(float yaw);

float getPitchFromVector(XMVECTOR front);

bool isKeyPressed(int key);

bool vectorEqual(XMVECTOR v1, XMVECTOR v2);
int getRandomNumber(int num1, int num2);

// XMVECTOR definitions
#define V3COMPARE(v1, v2) memcmp(&v1, &v2, 12);
#define V3DISTANCE(x, y) XMVectorGetX(XMVector3Length(x - y))

#define V4COMPARE(v1, v2) memcmp(&v1, &v2, 16);

#endif
