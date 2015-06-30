#ifndef _AICONTROLLER_H
#define _AICONTROLLER_H
#include "../handle/handle.h"

class aicontroller
{
private:
	unsigned int my_id;
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
	aicontroller();
	~aicontroller();
};
#endif

