#ifndef INC_AIMANAGER_H_
#define INC_AIMANAGER_H_
#include "aicontroller.h"

class aimanager
{
public:
	aimanager();
	~aimanager();
	aicontroller* getAI(unsigned int id);
	aicontroller *getAI(XMVECTOR pos, float radius);
	aicontroller *getClosest(XMVECTOR pos);
	static aimanager& get();
	void addBot(aicontroller* bot);
	void removeBot(unsigned int id);
private:
	unsigned int last_id;
};

#endif


