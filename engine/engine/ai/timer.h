#ifndef _TIMER_H
#define _TIMER_H

struct CTimer {
public:	
	float counter;
	float limit;

	CTimer();
	CTimer(float time);

	void update(float elapsed);
};

#endif