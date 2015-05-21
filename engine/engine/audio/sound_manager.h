#ifndef _SOUND_MANAGER_H_
#define _SOUND_MANAGER_H_

#include "bass.h" 

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

	void playTrack(int trackID);
	void playFXTrack(std::string name);
	void addFXTrack(const char* file, std::string name);
	void playMusic();
	void stopMusic();

	void crossFade(int trackID, float timeInSeconds);


	void playFX(std::string name);
	void stopFX(std::string name);

	void setSound3DFactors(float distance, float roll, float doppler);
	//bool set3DPosition(std::string name);
};


#endif