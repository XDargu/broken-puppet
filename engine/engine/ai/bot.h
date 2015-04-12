#ifndef _BOT_H_
#define _BOT_H_

#include "handle\handle.h"

// Wrapper LUA Bot class
class CBot {
private:
	CHandle entity;
public:

	CBot(CHandle the_entity);
	~CBot();

	// LUA
	void kill();
	void teleport(float x, float y, float z);
	float getLife();
	void setLife(float value);
	void hurt(float value);
	void help();
};

#endif