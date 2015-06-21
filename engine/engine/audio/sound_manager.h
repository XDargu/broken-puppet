#ifndef _SOUND_MANAGER_H_
#define _SOUND_MANAGER_H_

#include "bass.h" 
#include "components\comp_transform.h"

class CSoundManager
{
public:
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

	//vector of emiting channels
	//typedef std::vector< CSound* > VSounds;
	std::map<std::string, HSAMPLE>* sounds;
	//VSounds vector_sounds;

private:
	TMusicTrack music_tracks[8];
	int currentTrack;
	HSTREAM musicTracks[8];
public:
	CSoundManager();
	~CSoundManager();
	static CSoundManager& get();

	void addMusicTrack(int trackID, const char* file);

	void playTrack(int trackID, bool loop);
	void playTrack(std::string name, bool loop);
	void playFXTrack(std::string name);
	void addFXTrack(const char* file, std::string name);
	void playMusic(bool loop);
	void stopMusic();

	void crossFade(int trackID, float timeInSeconds, bool loop);

	void playFX(std::string name);
	void stopFX(std::string name);

	void play3DFX(std::string name, TTransform* trans, float volume_lambda);

	void play3DFX(std::string name, XMVECTOR pos);

	void setSound3DFactors(float distance, float roll, float doppler);
	//bool set3DPosition(std::string name);
};


#endif