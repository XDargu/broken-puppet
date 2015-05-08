#ifndef INC_AIMANAGER_H_
#define INC_AIMANAGER_H_
#include "aicontroller.h"

class aimanager
{
public:
	aimanager();
	~aimanager();
	aicontroller* getAI(unsigned int id);
	void getAI(XMVECTOR pos, float radius, std::vector<aicontroller*>& botsInRange);
	aicontroller *getClosest(XMVECTOR pos);
	static aimanager& get();
	void addBot(aicontroller* bot);
	void removeBot(unsigned int id);
	void warningToClose(aicontroller* me, float warning_distance);
	void warningPlayerFound(aicontroller* me);
	void getEnemyRol(aicontroller* enemy);
private:
	unsigned int last_id;
	vector<aicontroller*> attackers_rol;

};

#endif


