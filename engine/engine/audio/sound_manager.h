#ifndef _SOUND_MANAGER_H_
#define _SOUND_MANAGER_H_

#include "audio\bass.h"
#include "audio\bass_fx.h"
#include "components\comp_transform.h"

class CSoundManager
{
public:
	struct stream_effects{
		HSTREAM stream;
		CHandle FX_zone;
		bool slow;
		HFX slow_effect;
		bool under_water;
		HFX under_water_effect;
	};

	struct TMusicTrack {
		std::string name;
		HSTREAM stream;
		int sound_id;

		TMusicTrack(std::string the_name);

		void load();
		void unload();
		bool is_loaded();

		TMusicTrack() : name(""), stream(3435973836) {}
	};

	typedef std::pair<stream_effects, HSAMPLE> sounds_map;

	//vector of emiting channels
	//typedef std::vector< CSound* > VSounds;
	
	std::map<std::string, sounds_map>* sounds;
	std::map<std::string, std::vector<sounds_map>>* sounds_categories;
	//VSounds vector_sounds;

private:
	TMusicTrack music_tracks[8];
	int currentTrack;
	HSTREAM musicTracks[8];
	bool first;
	bool slowed;
	bool under_water;

public:
	float f_Bandwidth;
	float f_Center;
	float f_Gain;
	float f_Q;
	float f_S;

	float f_PitchShift;
	float f_Semitones;
	float l_FFTsize;
	float l_Osamp;

public:
	CSoundManager();
	~CSoundManager();
	static CSoundManager& get();

	void init();

	void addMusicTrack(int trackID, const char* file);

	void playTrack(int trackID, bool loop);
	void playTrack(std::string name, bool loop);
	void playFXTrack(std::string name);
	void playFXTrack(std::string name, bool loop);
	void addFXTrack(const char* file, std::string name);
	void addFXTrack(const char* file, std::string name, std::string category_name);
	void addFX2DTrack(const char* file, std::string name);
	void addFX2DTrack(const char* file, std::string name, std::string category_name);
	void playRandomFX(std::string category_name);
	void playMusic(bool loop);
	void stopMusic();

	void crossFade(int trackID, float timeInSeconds, bool loop);

	void playFX(std::string name);
	void playFX(std::string name, bool loop);
	void stopFX(std::string name);

	void play3DFX(std::string name, TTransform* trans, float volume_lambda);

	void play3DFX(std::string name, XMVECTOR pos);

	void playImpactFX(float force, CHandle transform);

	void setSound3DFactors(float distance, float roll, float doppler);
	void setSound3DAttributes(DWORD handle);

	void setReverbHFX(CHandle comp_hfx, HCHANNEL channel);
	void setFreeReverbHFX(CHandle comp_hfx, HCHANNEL channel);
	void setEchoHFX(CHandle comp_hfx, HCHANNEL channel);

	void activateSlowMotionSounds();
	void desactivateSlowMotionSounds();
	void activateLowPassSounds();
	void desactivateLowPassSounds();
	bool getUnderWater();
	bool getSlowMotion();
	void activateSlowMoFilter(HCHANNEL channel, std::string name);
	void desactivateSlowMoFilter(HCHANNEL channel, std::string name);
	void activateSlowMoFilter(HCHANNEL channel, std::string category_name, int ind);
	void desactivateSlowMoFilter(HCHANNEL channel, std::string category_name, int ind);
	void activateLowPassFilter(HCHANNEL channel, std::string name);
	void activateLowPassFilter(HCHANNEL channel, std::string category_name, int ind);
	void desactivateLowPassFilter(HCHANNEL channel, std::string name);
	void desactivateLowPassFilter(HCHANNEL channel, std::string category_name, int ind);
	//bool set3DPosition(std::string name);
};


#endif