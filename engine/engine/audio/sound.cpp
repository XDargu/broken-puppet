#include "mcv_platform.h"
#include "bass.h"
#include "sound.h"


CSound::CSound()
{
	own_sample = 0;
	own_channel = 0;
	loop = false;
}

void CSound::init_sound(std::string name, DWORD mode, float min, float max){
	//initialize all the properties
	if (CSoundManager::get().sounds->find(name) == CSoundManager::get().sounds->end()){
		XASSERT(CSoundManager::get().sounds, "error, sonido inexistente %s", name.c_str());
	}
	else{
		CSoundManager::sounds_map val = CSoundManager::get().sounds->operator[](name);
		own_sample = val.second;
		/*bool setting = Set3DSampleAttributes(own_sample, mode, min, max);
		if (!setting){
			XASSERT(CSoundManager::get().sounds, "error, sample atribbutes %s", name.c_str());
		}*/
		own_channel = BASS_SampleGetChannel(own_sample, FALSE);
		if (BASS_ErrorGetCode()==BASS_ERROR_HANDLE){
			XASSERT(CSoundManager::get().sounds, "error, codigo sample %s", name.c_str());
		}
		else if (BASS_ErrorGetCode() == BASS_ERROR_NOCHAN){
			XASSERT(CSoundManager::get().sounds, "error, sample sin channel disponible %s", name.c_str());
		}
		else if (BASS_ErrorGetCode() == BASS_ERROR_TIMEOUT){
			XASSERT(CSoundManager::get().sounds, "error, timeout mingap %s", name.c_str());
		}
		//BASS_Apply3D();
	}
}

void CSound::init_sound(std::string name){
	//initialize all the properties
	if (CSoundManager::get().sounds->find(name) == CSoundManager::get().sounds->end()){
		XASSERT(CSoundManager::get().sounds, "error, sonido inexistente %s", name.c_str());
	}
	else{
		CSoundManager::sounds_map val = CSoundManager::get().sounds->operator[](name);
		own_sample = val.second;
		own_channel = BASS_SampleGetChannel(own_sample, FALSE);
		if (own_channel){
			XASSERT(CSoundManager::get().sounds, "error cargando channel %s", name.c_str());
		}
	}
}

CSound::~CSound()
{
}

void CSound::playSound(){
	if (!is_playing()){
		if (loop)
			BASS_ChannelFlags(own_channel, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
		bool success=BASS_ChannelPlay(own_channel, 0);
		if (!success){
			XASSERT(success, "error play channel");
		}
	}
}

HCHANNEL CSound::getChannel(){
	return own_channel;
}

void CSound::setLoop(bool looped){
	loop = looped;
}

bool CSound::getLoop(){
	return loop;
}

void CSound::stopSound(){
	if (!loop){
		if (is_playing()){
			bool success = BASS_ChannelStop(own_channel);
			if (!success){
				XASSERT(success, "error stop channel");
			}
			else{
				bool success = BASS_SampleFree(own_channel);
				if (!success){
					XASSERT(success, "error free channel");
				}
			}
		}
	}else{
		if (is_playing()){
			bool success = BASS_ChannelStop(own_channel);
			XASSERT(!success, "error stop channel");
		}
	}
}

bool CSound::is_playing(){
	bool is_playing = false;
	DWORD result = BASS_ChannelIsActive(own_sample);
	if (result == BASS_ACTIVE_PLAYING){
		is_playing=true;
	}else if (result == BASS_ACTIVE_STOPPED){
		is_playing = false;
	}else if (result == BASS_ACTIVE_PAUSED){
		is_playing = false;
	}else if (result == BASS_ACTIVE_STALLED){
		is_playing = false;
	}
	return is_playing;
}

void CSound::setSoundPosition(BASS_3DVECTOR* pos_source, BASS_3DVECTOR* orient, BASS_3DVECTOR* vel){
	BASS_ChannelSet3DPosition(own_channel, pos_source, orient, vel);
}

void CSound::setSoundVolume(float volume){
	BASS_ChannelSetAttribute(own_channel, BASS_ATTRIB_VOL, volume);
}

bool CSound::Set3DSampleAttributes(DWORD handle, DWORD mode, float min, float max){
	BASS_SAMPLE info;
	info.flags = mode;
	info.mindist = min;
	info.maxdist = max;
	info.mingap = 0;
	info.freq = 44100;
	bool success = BASS_SampleSetInfo(handle, &info);
	return success;
}