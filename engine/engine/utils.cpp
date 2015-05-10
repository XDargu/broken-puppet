#include "mcv_platform.h"
#include "error\log.h"
#include <time.h>

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
	return 0;
	va_list ap;
	va_start(ap, fmt);
	char buf[512];
	size_t n = vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
	va_end(ap);
	::OutputDebugString(buf);
	std::string s = buf;
	std::wstring stemp = std::wstring(s.begin(), s.end());
	LPCWSTR sw = stemp.c_str();
	MessageBoxW(NULL, sw, L"Error", MB_OK | MB_ICONINFORMATION);
	FILE_LOG(logERROR) << buf;;
	assert(!printf("%s", buf));
	return 0;
}

// -----------------------------------------
void dbg(const char* fmt, ...) {
	return;
	va_list ap;
	va_start(ap, fmt);
	char buf[512];
	size_t n = vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
	::OutputDebugString(buf);
	FILE_LOG(logDEBUG) << buf;;
	va_end(ap);
}

void error(int line, char* file, char* method, char* msg, ...) {
	return;
	va_list ap;
	va_start(ap, msg);
	char buf[512];
	size_t n = vsnprintf(buf, sizeof(buf) - 1, msg, ap);

	std::string error_context = "";
	for (int i = 0; i < _error_context_name.size(); ++i) {
		error_context += "When " + std::string(_error_context_name[i]) + ": " + std::string(_error_context_data[i]) + "\n";
	}

	fatal("%s%s\n\t%s\n\tLine: %i\n\tFile: %s\n", error_context.c_str(), method, buf, line, file);
}

void debug(int line, char* file, char* method, char* msg, ...) {
	return;
	va_list ap;
	va_start(ap, msg);
	char buf[512];
	size_t n = vsnprintf(buf, sizeof(buf) - 1, msg, ap);

	std::string error_context = "";
	for (int i = 0; i < _error_context_name.size(); ++i) {
		error_context += "When " + std::string(_error_context_name[i]) + ": " + std::string(_error_context_data[i]) + "\n";
	}

	dbg("%s%s\n\t%s\n\tLine: %i\n\tFile: %s\n", error_context.c_str(), method, buf, line, file);
}

bool vectorEqual(XMVECTOR v1, XMVECTOR v2) {
	return XMVectorGetX(XMVectorEqual(v1, v2)) && XMVectorGetY(XMVectorEqual(v1, v2)) && XMVectorGetZ(XMVectorEqual(v2, v2));
}


int getRandomNumber(int num1, int num2){
	return rand() % num2 + num1;
}

float getRandomNumber(float min, float max){
	//srand(static_cast<float>(time(NULL)));
	float rand_num = (max - min) * ((((float)rand()) / (float)RAND_MAX)) + min;
	return rand_num;
}

bool stringEndsWith(std::string const &value, std::string const &ending) {
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::vector<std::string> split_string(const std::string& str,
	const std::string& delimiter)
{
	std::vector<std::string> strings;

	std::string::size_type pos = 0;
	std::string::size_type prev = 0;
	while ((pos = str.find(delimiter, prev)) != std::string::npos)
	{
		strings.push_back(str.substr(prev, pos - prev));
		prev = pos + 1;
	}

	// To get the last substring (or only, if delimiter is not found)
	strings.push_back(str.substr(prev));

	return strings;
}