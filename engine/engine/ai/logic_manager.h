#ifndef _LOGIC_MANAGER_H_
#define _LOGIC_MANAGER_H_

#include "timer.h"
#include "key_frame.h"
#include "handle\handle.h"

class CLogicManager
{
private:
	std::map<std::string, CTimer> timers;
	std::vector<CHandle> triggers;
	std::vector<TKeyFrame> current_keyframes;
	std::vector<TKeyFrame> keyframe_queue;
public:

	static CLogicManager& get();

	CLogicManager();
	~CLogicManager();

	void update(float elapsed);

	void setTimer(std::string, float time);

	void registerTrigger(CHandle trigger);
	void unregisterTrigger(CHandle trigger);

	void addKeyFrame(CHandle the_target_transform, XMVECTOR the_target_position, XMVECTOR the_target_rotation, float the_time);
};

#endif