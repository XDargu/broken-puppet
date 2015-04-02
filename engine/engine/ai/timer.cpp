#include "mcv_platform.h"
#include "timer.h"

CTimer::CTimer() : counter(0.f), limit(0.f)  { 
}

CTimer::CTimer(float time) {
	counter = 0.f;
	limit = time;
}

void CTimer::update(float elapsed) {
	counter += elapsed;
}