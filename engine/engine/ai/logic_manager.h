#ifndef _LOGIC_MANAGER_H_
#define _LOGIC_MANAGER_H_

#include "timer.h"
#include "key_frame.h"
#include "handle\handle.h"
#include "bot.h"
#include "lua_vector.h"

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
	void onTriggerEnter(CHandle trigger, CHandle who);
	void onTriggerExit(CHandle trigger, CHandle who);
	void unregisterTrigger(CHandle trigger);

	void onSwitchPressed(CHandle the_switch);
	void onSwitchReleased(CHandle the_switch);

	void addKeyFrame(CHandle the_target_transform, XMVECTOR the_target_position, XMVECTOR the_target_rotation, float the_time);
	void addRelativeKeyFrame(CHandle the_target_transform, XMVECTOR position_offset, XMVECTOR rotation_offset, float the_time);
	void stopKeyframes(CHandle the_target_transform);

	void clearKeyframes();

	void bootLUA();

	// LUA
	void execute(std::string text);

	void onSceneLoad(std::string scene_name);
	CBot getBot(std::string name);
	void print(std::string text);
	void help();
};

#endif