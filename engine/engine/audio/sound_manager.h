#ifndef _SOUND_MANAGER_H_
#define _SOUND_MANAGER_H_

#include "bass.h"

class CSoundManager
{
public:
	struct TMusicTrack {
		std::string name;
		HSTREAM stream;

		TMusicTrack(std::string the_name);

		void load();
		void unload();
		bool is_loaded() { return stream != 3435973836; }

		TMusicTrack() : name(""), stream(3435973836) {}
	};
private:
	TMusicTrack music_tracks[8];

	int currentTrack;	
	HSTREAM musicTracks[8];
public:
	CSoundManager();
	~CSoundManager();

	void addMusicTrack(int trackID, const char* file);

	void playTrack(int trackID);
	void playMusic();
	void stopMusic();

	void crossFade(int trackID, float timeInSeconds);
};


#endif