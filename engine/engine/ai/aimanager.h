#ifndef INC_AIMANAGER_H_
#define INC_AIMANAGER_H_
#include "aicontroller.h"

class aimanager
{
public:
	std::vector<aicontroller*> bots;
	std::vector<aicontroller*> grandmas;

	aimanager();
	~aimanager();

	aicontroller* getAI(unsigned int id);
	void getAI(XMVECTOR pos, float radius, std::vector<aicontroller*>& botsInRange);
	aicontroller *getClosest(XMVECTOR pos);
	aicontroller *getClosestGrandma(XMVECTOR pos);
	static aimanager& get();
	void addBot(aicontroller* bot);
	void addGrandma(aicontroller* grandma);
	void removeBot(unsigned int id);
	void removeGrandma(unsigned int id);
	void warningToClose(aicontroller* me, float warning_distance, CHandle player_transform);
	void warningPlayerFound(aicontroller* me);
	void setEnemyRol(aicontroller* enemy);
	bool attackRolEmptySlot();
	void RemoveEnemyAttacker(aicontroller* enemy);
	void RemoveEnemyTaunt(aicontroller* enemy);
	void changeEnemyRol(aicontroller* enemy);
	void recastAABBActivate(int ind);
	void recastAABBDesactivate(int ind);
	void clear();
private:
	unsigned int last_id;
	std::vector<aicontroller*> attackers_rol;
	std::vector<aicontroller*> taunters_rol;};

#endif


