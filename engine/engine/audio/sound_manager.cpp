#include "mcv_platform.h"
#include "sound_manager.h"
#include "bass.h"

CSoundManager::TMusicTrack::TMusicTrack(std::string the_name) {
	name = the_name;	
}

void CSoundManager::TMusicTrack::load() {
	stream = BASS_StreamCreateFile(FALSE, name.c_str(), 0, 0, 0);
}

void CSoundManager::TMusicTrack::unload() {
	BASS_StreamFree(stream);
	stream = 3435973836;
}

CSoundManager::CSoundManager()
{
	BASS_Init(-1, 44100, 0, 0, NULL);
	currentTrack = 0;
}

CSoundManager::~CSoundManager()
{
	BASS_Free();
}


void CSoundManager::addMusicTrack(int trackID, const char* file) {
	music_tracks[trackID] = TMusicTrack("data/music/" + std::string(file));
}


void CSoundManager::playTrack(int trackID) {
	// Unload the curren track, if needed
	if (currentTrack != trackID) {
		if (music_tracks[currentTrack].is_loaded())
			music_tracks[currentTrack].unload();
	}
	currentTrack = trackID;
	playMusic();
}

void CSoundManager::playMusic() {
	//BASS_ChannelPlay(musicTracks[currentTrack], 0);

	// Load the stream if needed
	if (!music_tracks[currentTrack].is_loaded())
		music_tracks[currentTrack].load();

	// Play it
	BASS_ChannelPlay(music_tracks[currentTrack].stream, 0);
}

void CSoundManager::stopMusic() {
	//BASS_ChannelStop(musicTracks[currentTrack]);
	BASS_ChannelStop(music_tracks[currentTrack].stream);
}

void CALLBACK StopCrossFade(HSYNC handle, DWORD channel, DWORD data, void *user)
{
	((CSoundManager::TMusicTrack*)user)->unload();
	BASS_ChannelStop(handle);
}

void CSoundManager::crossFade(int trackID, float timeInSeconds) {
	// Only crossFade if the track is different
	if (trackID != currentTrack) {

		// Load the new track
		music_tracks[trackID].load();

		// Play the new track
		BASS_ChannelPlay(music_tracks[trackID].stream, true);

		// Set the slides to change the volume
		BASS_ChannelSlideAttribute(music_tracks[trackID].stream, BASS_ATTRIB_VOL, 1, timeInSeconds * 1000);
		BASS_ChannelSlideAttribute(music_tracks[currentTrack].stream, BASS_ATTRIB_VOL, 0, timeInSeconds * 1000);

		// Sync to stop when the music ends
		BASS_ChannelSetSync(music_tracks[currentTrack].stream, BASS_SYNC_SLIDE, NULL, StopCrossFade, &music_tracks[currentTrack]);

		// Change the current track
		currentTrack = trackID;
	}
}


