#ifndef _MCV_OBJECT_H_
#define _MCV_OBJECT_H_

#include "handle\handle.h"
#include "lua_vector.h"

// Wrapper LUA Object class
class CMCVObject {
private:
	CHandle entity;
public:

	CMCVObject(CHandle the_entity);
	~CMCVObject();

	CHandle getEntity() { return entity; }

	// LUA
	CVector getPosition();
	void setPosition(CVector newPos);

	void moveToPosition(CVector position, float speed);
	void setEmissive(bool active);
	void applyForce(CVector force);
	void setRender(bool active);
	void riseUpBoss();
	void initLittleTalk();
	void hitSubstitute();
	CMCVObject firstBombBoss();
	void initialRain(int amount);
	void generatePrefabs(int amount);
};

#endif