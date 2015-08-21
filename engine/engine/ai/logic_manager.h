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
#include "lua_quaternion.h"
#include "audio\bass.h"
#include "mcv_object.h"

class CLogicManager
{
private:
	std::map<std::string, CTimer> timers;
	std::vector<CHandle> triggers;
	std::vector<CHandle> GNLogic;
	std::vector<CHandle> ZonesAABB;
	std::vector<CHandle> HFXZones;

	std::vector<CRigidAnimation> animations;

	float water_level_dest;
	float lerp_water;
	int particle_group_counter;

	XMVECTOR ambient_light;
	float lerp_ambient_light;

	// Keep some usefull handles
	CHandle water_transform;
	CHandle water2_transform;
	
	CHandle player;
	CHandle player_pivot;
	CHandle camera_pivot;

	std::string scene_to_load;
public:

	static CLogicManager& get();

	CLogicManager();
	~CLogicManager();

	void update(float elapsed);
	void init();

	void setTimer(std::string, float time);

	void registerTrigger(CHandle trigger);
	void registerGNLogic(CHandle golden_logic);
	void registerZoneAABB(CHandle zone_aabb);
	void registerHFXZone(CHandle hfx_zone);
	void onTriggerEnter(CHandle trigger, CHandle who);
	void onTriggerExit(CHandle trigger, CHandle who);
	void unregisterTrigger(CHandle trigger);
	void unregisterGNLogic(CHandle golden_logic);
	void unregisterZoneAABB(CHandle zone_aabb);
	void unregisterHFXZone(CHandle hfx_zone);

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

	//Zone AABB
	CHandle getPlayerZoneName();

	//Golden Needle
	bool playerInsideGNZone(XMVECTOR& vector, CHandle& logicGN);

	//HFX 
	CHandle soundsInsideHFXZone(XMVECTOR sound_pos);

	// LUA
	void execute(std::string text);

	void loadScene(std::string scene_name);
	void onSceneLoad(std::string scene_name);
	CBot getBot(std::string name);
	CMCVObject getObject(std::string name);
	CMovingPlatform getMovingPlatform(std::string name);
	CPrismaticJoint getPrismaticJoint(std::string name);
	CHingeJoint getHingeJoint(std::string name);

	void pushPlayerLegsState(std::string state_name);
	void cameraLookAtBot(CBot bot);
	void cameraLookAtPosition(CVector target);
	void cameraLookAt(XMVECTOR target);

	void playerDead();
	void changeCamera(std::string name);

	// SOUND & MUSIC
	void changeTrack(std::string name, bool loop);
	void stopMusic();
	void playMusic(bool loop);

	// LIGHT & ENVIROMENT
	void changeWaterLevel(float pos1, float time);
	void changeAmbientLight(float r, float g, float b, float time);

	// PARTICLES
	void createParticleGroup(std::string pg_name, CVector position, CQuaterion rotation);

	// STRING EVENTS
	void stringThrown();
	void stringsTensed();
	void stringPulled();
	void stringCancelled();
	void stringAllCancelled();

	// MISC
	void print(std::string text);
	void help();
};

#endif