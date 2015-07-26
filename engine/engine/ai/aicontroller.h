#ifndef _AICONTROLLER_H
#define _AICONTROLLER_H
#include "../handle/handle.h"

class aicontroller
{
public:
	virtual void recalc(float deltaTime) = 0;
	virtual void create(std::string) = 0;
	virtual void update(float elapsed) = 0;
	virtual void SetEntity(CHandle the_entity) = 0;
	virtual CHandle GetEntity() = 0;
	//virtual void setId(unsigned int id) = 0;
	//virtual unsigned int getInt() = 0;
	virtual std::string getCurrentNode() = 0;
	virtual CHandle getTransform() = 0;
	unsigned getId();
	void setId(unsigned int id);
	enum types
	{
		soldier,
		granny,
		untyped
	};
	//-------------------------------------
	virtual void setRol(int r) = 0;
	virtual int getRol() = 0;
	virtual void setAttackerSlot(int s) = 0;
	virtual int getAttackerSlot() = 0;
	virtual float getDistanceToPlayer() = 0;
	virtual int getNearestSlot(bool free_north, bool free_east, bool free_west) = 0;
	virtual bool isAngry() = 0;
	virtual int getIndRecastAABB()=0;
	virtual void setActive(bool act)=0;
	virtual void setType(types kind) = 0;
	virtual types getType() = 0;
	//-------------------------------------

	aicontroller();
	~aicontroller();
private:
	unsigned int my_id;
};
#endif

