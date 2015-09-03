#ifndef _SOUND_MANAGER_H_
#define _SOUND_MANAGER_H_

#include "audio\bass.h"
#include "audio\bass_fx.h"
#include "components\comp_transform.h"
#include "fmod.hpp"
#include "fmod_studio.hpp"
#include "fmod_errors.h"

class CSoundManager
{
private:
	FMOD::Studio::Bank* masterBank;
	FMOD::Studio::Bank* stringsBank;

	FMOD::Studio::System* system;
	std::map<std::string, FMOD::Studio::EventDescription*> event_descriptions;

	XMVECTOR invalidPosition;

	bool slowed;

public:
	
	struct SoundParameter {
		std::string name;
		float value;
	};

	/*struct stream_effects{
		HSTREAM stream;
		CHandle FX_zone;
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

public:*/
	CSoundManager();
	~CSoundManager();
	static CSoundManager& get();

	void playEvent(std::string path, SoundParameter* parameters, int nparameters);
	void playEvent(std::string path);

	void playEvent(std::string path, SoundParameter* parameters, int nparameters, XMVECTOR pos);
	void playEvent(std::string path, XMVECTOR pos);

	void setListenerTransform(TTransform listener);

	void update(float elapsed);

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

	void setReverbHFX(CHandle comp_hfx, HSTREAM channel);
	void setFreeReverbHFX(CHandle comp_hfx, HSTREAM channel);
	void setEchoHFX(CHandle comp_hfx, HSTREAM channel);
	//bool set3DPosition(std::string name);

	FMOD_VECTOR XMVECTORtoFmod(XMVECTOR vector);
	XMVECTOR FmodToXMVECTOR(FMOD_VECTOR vector);

	void activateSlowMo();
	void desactivateSlowMo();

};


#endif