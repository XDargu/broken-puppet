#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include <time.h>
// Error


#define XASSERT(test, msg, ...) do {if (!(test)) error(__LINE__, __FILE__, "Assertion failed: " #test,\
        msg, __VA_ARGS__);} while (0)
#define XERROR(msg, ...) do { error(__LINE__, __FILE__, "Error thrown: ", msg, __VA_ARGS__);} while (0)
#define XDEBUG(msg, ...) do { debug(__LINE__, __FILE__, "Debug message: ", msg, __VA_ARGS__);} while (0)
#define SET_ERROR_CONTEXT(name, data) CErrorContext ec(name, data);

#define clamp(v,x,y) max(min(v, y), x)

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
float getRandomNumber(float min,float max);
XMVECTOR getRandomVector3(float minXYZ, float maxXYZ);
XMVECTOR getRandomVector3(float minX, float minY, float minZ, float maX, float maxY, float maxZ);
XMVECTOR getRandomVector3(XMFLOAT3 minXYZ, XMFLOAT3 maxXYZ);

float lerp(float a, float b, float f);
// Strings
bool stringEndsWith(std::string const &value, std::string const &ending);
std::vector<std::string> split_string(const std::string& str, const std::string& delimiter);
std::string V3ToString(XMVECTOR vector);
std::string V4ToString(XMVECTOR vector);

// Files
void files_in_directory(std::string directory, std::vector<std::string>& list);

// XMVECTOR definitions
#define V3COMPARE(v1, v2) memcmp(&v1, &v2, 12)
#define V3DISTANCE(x, y) XMVectorGetX(XMVector3Length((x) - (y)))

#define V4COMPARE(v1, v2) memcmp(&v1, &v2, 16)

#define SAFE_DELETE(x) if (x != nullptr) { delete x; x = nullptr; }

// Timers
struct CDBGTimer {
public:
	CDBGTimer() {
		reset();
	}
	/// reset() makes the timer start over counting from 0.0 seconds.
	void reset() {
		unsigned __int64 pf;
		QueryPerformanceFrequency((LARGE_INTEGER *)&pf);
		freq_ = 1.0 / (double)pf;
		QueryPerformanceCounter((LARGE_INTEGER *)&baseTime_);
	}
	/// seconds() returns the number of seconds (to very high resolution)
	/// elapsed since the timer was last created or reset().
	double seconds() {
		unsigned __int64 val;
		QueryPerformanceCounter((LARGE_INTEGER *)&val);
		return (val - baseTime_) * freq_;
	}
	/// seconds() returns the number of milliseconds (to very high resolution)
	/// elapsed since the timer was last created or reset().
	double milliseconds() {
		return seconds() * 1000.0;
	}
private:
	double freq_;
	unsigned __int64 baseTime_;
};



#endif
