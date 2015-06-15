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

	// LUA
	CVector getPosition();
	void setPosition(CVector newPos);

	void moveToPosition(CVector position, float speed);
};

#endif