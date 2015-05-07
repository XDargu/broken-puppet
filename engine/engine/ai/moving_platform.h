#ifndef _MOVING_PLATFORM_H_
#define _MOVING_PLATFORM_H_

#include "handle\handle.h"

// Wrapper LUA Bot class
class CMovingPlatform {
private:
	CHandle entity;
public:

	CMovingPlatform(CHandle the_entity);
	~CMovingPlatform();

	// LUA
	void start();
	void stop();
};

#endif