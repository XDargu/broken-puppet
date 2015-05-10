#ifndef _LOGIC_MANAGER_H_
#define _LOGIC_MANAGER_H_

#include "timer.h"
#include "rigid_animation.h"
#include "handle\handle.h"
#include "bot.h"
#include "moving_platform.h"
#include "prismatic_joint.h"
#include "hinge_joint.h"
#include "lua_vector.h"

class CLogicManager
{
private:
	std::map<std::string, CTimer> timers;
	std::vector<CHandle> triggers;

	std::vector<CRigidAnimation> animations;

	float water_level_dest;
	float lerp_water;
	CHandle water_transform;
public:

	static CLogicManager& get();

	CLogicManager();
	~CLogicManager();

	void update(float elapsed);
	void init();

	void setTimer(std::string, float time);

	void registerTrigger(CHandle trigger);
	void onTriggerEnter(CHandle trigger, CHandle who);
	void onTriggerExit(CHandle trigger, CHandle who);
	void unregisterTrigger(CHandle trigger);

	void onSwitchPressed(CHandle the_switch);
	void onSwitchReleased(CHandle the_switch);

	void startPathMovement(const char* name);

	/*void addKeyFrame(CHandle the_target_transform, XMVECTOR the_target_position, XMVECTOR the_target_rotation, float the_time);
	void addRelativeKeyFrame(CHandle the_target_transform, XMVECTOR position_offset, XMVECTOR rotation_offset, float the_time);
	void stopKeyframes(CHandle the_target_transform);
	void clearKeyframes();*/

	void addRigidAnimation(CRigidAnimation animation);
	void clearAnimations();

	void bootLUA();

	// LUA
	void execute(std::string text);

	void loadScene(std::string scene_name);
	void onSceneLoad(std::string scene_name);
	CBot getBot(std::string name);
	CMovingPlatform getMovingPlatform(std::string name);
	CPrismaticJoint getPrismaticJoint(std::string name);
	CHingeJoint getHingeJoint(std::string name);

	void changeWaterLevel(float pos1, float time);
	void pushPlayerLegsState(std::string state_name);

	void playerDead();

	void print(std::string text);
	void help();
};

#endif