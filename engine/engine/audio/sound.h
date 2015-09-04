#ifndef _SOUND_H_
#define _SOUND_H_

#include "bass.h" 
#include "sound_manager.h"

class CSound
{
public:
	HSTREAM own_stream;
	HCHANNEL own_channel;
	bool loop;
	CHandle FX_zone;
	bool slow;
	HFX slow_effect;
	bool under_water;
	HFX under_water_effect;

public:
	CSound();
	~CSound();
	void init_sound(std::string name, DWORD mode, float min, float max);
	void init_sound(std::string name);
	/*void setStream(std::string name);
	void setChannel();*/
	void playSound();
	bool is_playing();
	void stopSound();
	void setSoundPosition(BASS_3DVECTOR* pos_source, BASS_3DVECTOR* orient, BASS_3DVECTOR* vel);
	void Set3DSampleAttributes(DWORD handle, DWORD mode, float min, float max);
	/*void setLoop(bool looped);
	bool getLoop();*/
	void setSoundVolume(float volume);
};
#endif
