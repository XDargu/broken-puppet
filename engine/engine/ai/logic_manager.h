#ifndef _LOGIC_MANAGER_H_
#define _LOGIC_MANAGER_H_

#include "timer.h"
#include "handle\handle.h"

class CLogicManager
{
private:
	std::map<std::string, CTimer> timers;
	std::vector<CHandle> triggers;
public:

	static CLogicManager& get();

	CLogicManager();
	~CLogicManager();

	void update(float elapsed);

	void setTimer(std::string, float time);

	void registerTrigger(CHandle trigger);
	void unregisterTrigger(CHandle trigger);
};

#endif