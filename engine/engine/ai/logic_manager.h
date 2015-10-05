#ifndef _LOGIC_MANAGER_H_
#define _LOGIC_MANAGER_H_

#include "timer.h"
#include "rigid_animation.h"
#include "handle\handle.h"
#include "bot.h"
#include "moving_platform.h"
#include "prismatic_joint.h"
#include "fixed_joint.h"
#include "hinge_joint.h"
#include "lua_vector.h"
#include "lua_quaternion.h"
//#include "audio\bass.h"
#include "mcv_object.h"
#include "audio\sound_manager.h"

class CLogicManager
{
public:
	struct Subtitle {
		float time;
		float size;
		std::string sound;
		std::string text;
		unsigned color;
		std::string next;
	};
	unsigned int p_group_counter;
private:
	std::map<std::string, CTimer> timers;
	std::vector<CHandle> triggers;
	std::vector<CHandle> GNLogic;
	std::vector<CHandle> ZonesAABB;

	std::vector<CRigidAnimation> animations;

	float band_heigth_dest;
	float band_heigth;
	float lerp_bands;

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
	CHandle camera;

	// Lock on target
	CHandle lock_on_target;
	XMVECTOR lock_on_position;

	// Shakecam
	bool shake_cam;
	float shake_amount;

	std::string scene_to_load;

	// Subtitles
	Subtitle current_subtitle;
	float subtitle_counter;

	std::map<std::string, Subtitle> subtitle_map;

	int current_player_zone;

public:	

	static CLogicManager& get();

	CLogicManager();
	~CLogicManager();

	void update(float elapsed);
	void init();
	void draw();
	void destroy();

	void setTimer(std::string, float time);

	void registerTrigger(CHandle trigger);
	void registerGNLogic(CHandle golden_logic);
	void registerZoneAABB(CHandle zone_aabb);
	void onTriggerEnter(CHandle trigger, CHandle who);
	void onTriggerFirstEnter(CHandle trigger, CHandle who);
	void onTriggerExit(CHandle trigger, CHandle who);
	void unregisterTrigger(CHandle trigger);
	void unregisterGNLogic(CHandle golden_logic);
	void unregisterZoneAABB(CHandle zone_aabb);

	void onSwitchPressed(CHandle the_switch);
	void onSwitchReleased(CHandle the_switch);

	void addSubtitle(std::string guid, Subtitle subtitle);

	CHandle instantiateParticleGroup(std::string pg_name, CVector position, CQuaterion rotation);


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
	int getPlayerZoneID();
	int getPointZoneID(XMVECTOR position);

	//Golden Needle
	bool playerInsideGNZone(XMVECTOR& vector, CHandle& logicGN);

	CHandle getPlayerHandle() { return player; }

	// LUA
	void execute(std::string text);

	void loadScene(std::string scene_name);
	void onSceneLoad(std::string scene_name);
	CBot getBot(std::string name);
	CMCVObject getObject(std::string name);
	CMovingPlatform getMovingPlatform(std::string name);
	CPrismaticJoint getPrismaticJoint(std::string name);
	CFixedJoint getFixedJoint(std::string name);
	CHingeJoint getHingeJoint(std::string name);

	void pushPlayerLegsState(std::string state_name);
	void cameraLookAtBot(CBot bot);
	void cameraLookAtPosition(CVector target);
	void cameraLookAt(XMVECTOR target);

	void playerDead();
	

	// SOUND & MUSIC
	void playEvent(std::string name);
	void playEventAtPosition(std::string name, CVector pos);
	void playEventParameter(std::string name, std::string parameter, float value);
	void playEventParameterAtPosition(std::string name, std::string parameter, float value, CVector pos);

	// BASS -- TODO: Delete deprecated methods
	/*void changeTrack(std::string name, bool loop);
	void stopMusic();
	void playMusic(bool loop);*/

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

	// PLAYER
	void setCanThrow(bool active);
	void setCanPull(bool active);
	void setCanCancel(bool active);
	void setCanTense(bool active);
	void setCanMove(bool active);

	// EFFECTS
	void setBand(bool bands);

	// SUBTITLES
	void playSubtitles(std::string guid);

	// CAMERA
	void changeCamera(std::string name);
	void setPlayerCameraMediumShotActive(bool active);
	void setPlayerCameraLongShotActive(bool active);
	void resetPlayerCamera();
	void lockOnBot(CBot bot);
	void lockOnPosition(CVector position);
	void releaseCameraLock();
	void playAnimation(std::string name, CMCVObject target_object);
	void shakeCamera(float amount);
	void stopShakeCamera();

	// MISC
	void print(std::string text);
	void help();
};

#endif