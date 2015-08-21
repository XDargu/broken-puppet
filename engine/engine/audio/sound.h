#ifndef _SOUND_H_
#define _SOUND_H_

#include "bass.h" 
#include "sound_manager.h"

class CSound
{
private:
	HSAMPLE own_sample;
	HCHANNEL own_channel;
	bool loop;

public:
	CSound();
	~CSound();
	void init_sound(std::string name, DWORD mode, float min, float max);
	void init_sound(std::string name);
	/*void setStream(std::string name);
	void setChannel();*/
	HCHANNEL getChannel();
	void playSound();
	bool is_playing();
	void stopSound();
	void setSoundPosition(BASS_3DVECTOR* pos_source, BASS_3DVECTOR* orient, BASS_3DVECTOR* vel);
	bool Set3DSampleAttributes(DWORD handle, DWORD mode, float min, float max);
	void setLoop(bool looped);
	bool getLoop();
	void setSoundVolume(float volume);
};
#endif
