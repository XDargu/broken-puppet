#include "mcv_platform.h"
#include "sound_manager.h"
#include "components\comp_hfx_zone.h"
#include "ai\logic_manager.h"
#include "bass.h"

static CSoundManager the_sound_manager;
static float volume_factor = 1000;

CSoundManager::TMusicTrack::TMusicTrack(std::string the_name) {
	name = the_name;
}

void CSoundManager::TMusicTrack::load() {
	stream = BASS_StreamCreateFile(FALSE, name.c_str(), 0, 0, 0);
}

bool CSoundManager::TMusicTrack::is_loaded() {
	if (stream != 3435973836)
		return false;
	else
		return true;
}

void CSoundManager::TMusicTrack::unload() {
	BASS_StreamFree(stream);
	stream = 3435973836;
}

CSoundManager& CSoundManager::get() {
	return the_sound_manager;
}

CSoundManager::CSoundManager()
{
	sounds = new std::map<std::string, sounds_map>();
	sounds_categories = new std::map<std::string, std::vector<sounds_map>>();
	BASS_Init(-1, 44100, BASS_DEVICE_3D, 0, NULL);
	first = false;
	slowed = false;
	currentTrack = 0;
}

CSoundManager::~CSoundManager()
{
	delete sounds;
	sounds = nullptr;
	delete sounds_categories;
	sounds_categories = nullptr;
	//vector_sounds.clear();
	BASS_Free();
}


void CSoundManager::addMusicTrack(int trackID, const char* file) {
	music_tracks[trackID] = TMusicTrack("data/music/" + std::string(file));
}

void CSoundManager::playTrack(int trackID, bool loop) {
	// Unload the curren track, if needed
	if (currentTrack != trackID) {
		if (music_tracks[currentTrack].is_loaded())
			music_tracks[currentTrack].unload();
	}
	currentTrack = trackID;
	playMusic(loop);
}

void CSoundManager::playTrack(std::string name, bool loop) {
	int trackID = currentTrack;
	int counter = 0;
	for (auto& music_track : music_tracks) {
		counter++;
		if (music_track.name == ("data/music/" + std::string(name))) {
			trackID = counter;
			break;
		}
	}
	// Unload the curren track, if needed
	if (currentTrack != trackID) {
		if (music_tracks[currentTrack].is_loaded())
			music_tracks[currentTrack].unload();
	}
	currentTrack = trackID;
	playMusic(loop);
}

void CSoundManager::playMusic(bool loop) {
	//BASS_ChannelPlay(musicTracks[currentTrack], 0);

	// Load the stream if needed
	if (!music_tracks[currentTrack].is_loaded())
		music_tracks[currentTrack].load();

	// Play it
	if(loop)
		BASS_ChannelFlags(music_tracks[currentTrack].stream, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
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

void CSoundManager::crossFade(int trackID, float timeInSeconds, bool loop) {
	// Only crossFade if the track is different
	if (trackID != currentTrack) {

		// Load the new track
		music_tracks[trackID].load();

		// Play the new track
		if (loop)
			BASS_ChannelFlags(music_tracks[trackID].stream, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);

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

void CSoundManager::addFXTrack(const char* file, std::string name){
	std::string filename = ("data/sounds/" + std::string(file));
	HSAMPLE h_sample = BASS_SampleLoad(0, filename.c_str(), 0, 0, 200, BASS_SAMPLE_3D);
	HSTREAM h_stream = BASS_StreamCreateFile(FALSE, filename.c_str(), 0, 0, BASS_SAMPLE_3D);
	stream_effects stream_FX;
	stream_FX.stream = h_stream;
	stream_FX.FX_zone = CHandle();
	sounds_map val = std::make_pair(stream_FX, h_sample);
	sounds->operator[](name) = val;
}

void CSoundManager::addFXTrack(const char* file, std::string name, std::string category_name){
	std::string filename = ("data/sounds/" + std::string(file));
	HSAMPLE h_sample = BASS_SampleLoad(0, filename.c_str(), 0, 0, 200, BASS_SAMPLE_3D);
	HSTREAM h_stream = BASS_StreamCreateFile(FALSE, filename.c_str(), 0, 0, BASS_SAMPLE_3D);
	stream_effects stream_FX;
	stream_FX.stream = h_stream;
	stream_FX.FX_zone = CHandle();
	sounds_map val = std::make_pair(stream_FX, h_sample);
	sounds->operator[](name) = val;
	sounds_categories->operator[](category_name).push_back(val);
}

void CSoundManager::addFX2DTrack(const char* file, std::string name){
	std::string filename = ("data/sounds/" + std::string(file));
	HSAMPLE h_sample = BASS_SampleLoad(0, filename.c_str(), 0, 0, 200, BASS_SAMPLE_MONO);
	HSTREAM h_stream = BASS_StreamCreateFile(FALSE, filename.c_str(), 0, 0, BASS_SAMPLE_FX);
	stream_effects stream_FX;
	stream_FX.stream = h_stream;
	stream_FX.FX_zone = CHandle();
	sounds_map val = std::make_pair(stream_FX, h_sample);
	sounds->operator[](name) = val;
}

void CSoundManager::addFX2DTrack(const char* file, std::string name, std::string category_name){
	std::string filename = ("data/sounds/" + std::string(file));
	HSAMPLE h_sample = BASS_SampleLoad(0, filename.c_str(), 0, 0, 200, BASS_SAMPLE_MONO);
	HSTREAM h_stream = BASS_StreamCreateFile(FALSE, filename.c_str(), 0, 0, BASS_SAMPLE_FX);
	stream_effects stream_FX;
	stream_FX.stream = h_stream;
	stream_FX.FX_zone = CHandle();
	sounds_map val = std::make_pair(stream_FX, h_sample);
	sounds->operator[](name) = val;
	sounds_categories->operator[](category_name).push_back(val);
}

void CSoundManager::playRandomFX(std::string category_name){
	if (sounds_categories->operator[](category_name).size() > 0){
		int random_ind = getRandomNumber(0, sounds_categories->operator[](category_name).size());
		sounds_map stream = sounds_categories->operator[](category_name)[random_ind];
		HCHANNEL channel = BASS_SampleGetChannel(stream.first.stream, FALSE);
		BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, 0.3f);
		BASS_ChannelFlags(channel, BASS_STREAM_AUTOFREE, BASS_STREAM_AUTOFREE);
		BASS_ChannelPlay(channel, 0);
	}
}

void CSoundManager::playFXTrack(std::string name) {
	//If the Fx exists in the tree, is previouly loaded
	if (sounds->find(name) == sounds->end()){
		XASSERT(sounds, "error, sonido inexistente %s", name.c_str());
	}
	else{
		playFX(name);
	}
}

void CSoundManager::playFXTrack(std::string name, bool loop) {
	//If the Fx exists in the tree, is previouly loaded
	if (sounds->find(name) == sounds->end()){
		XASSERT(sounds, "error, sonido inexistente %s", name.c_str());
	}
	else{
		playFX(name, loop);
	}
}

void CSoundManager::playFX(std::string name){
	// Play FX
	sounds_map stream = sounds->operator[](name);
	HCHANNEL channel = BASS_SampleGetChannel(stream.first.stream, FALSE);
	BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, 0.3f);
	BASS_ChannelFlags(channel, BASS_STREAM_AUTOFREE, BASS_STREAM_AUTOFREE);
	BASS_ChannelPlay(channel, 0);
}

void CSoundManager::playFX(std::string name, bool loop){
	// Play FX
	sounds_map stream = sounds->operator[](name);
	HCHANNEL channel = BASS_SampleGetChannel(stream.first.stream, FALSE);
	if (loop){
		BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, 1.f);
		BASS_ChannelFlags(channel, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
	}else
		BASS_ChannelFlags(channel, BASS_STREAM_AUTOFREE, BASS_STREAM_AUTOFREE);
	BASS_ChannelPlay(channel, 0);
}

void CSoundManager::stopFX(std::string name){
	// Play FX
	sounds_map stream = sounds->operator[](name);
	HCHANNEL channel = BASS_SampleGetChannel(stream.first.stream, FALSE);
	BASS_ChannelStop(channel);
}

void CSoundManager::play3DFX(std::string name, TTransform* trans, float volume_lambda){
	sounds_map stream = sounds->operator[](name);
	HCHANNEL channel = BASS_SampleGetChannel(stream.second, FALSE);
	BASS_ChannelFlags(channel, BASS_STREAM_AUTOFREE, BASS_STREAM_AUTOFREE);
	BASS_3DVECTOR pos_ref;
	BASS_3DVECTOR front_ref;
	pos_ref.x = XMVectorGetX(trans->position);
	pos_ref.y = XMVectorGetY(trans->position);
	pos_ref.z = XMVectorGetZ(trans->position);
	front_ref.x = XMVectorGetX(trans->position + trans->getFront());
	front_ref.y = XMVectorGetY(trans->position + trans->getFront());
	front_ref.z = XMVectorGetZ(trans->position + trans->getFront());
	BASS_3DVECTOR* pos;
	BASS_3DVECTOR* front;
	pos = &pos_ref;
	front = &front_ref;
	BASS_ChannelSet3DPosition(channel, pos, front, NULL);
	BASS_Apply3D();
	float volume;
	BASS_ChannelGetAttribute(channel, BASS_ATTRIB_VOL, &volume);
	float final_volume = volume + volume_lambda / volume_factor;
	BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, final_volume);


	bool zone = false;
	CHandle comp_hfx = CLogicManager::get().soundsInsideHFXZone(trans->position);
	if (comp_hfx.isValid()){
		if (stream.first.FX_zone == comp_hfx){
			stream.first.FX_zone = comp_hfx;
		}
		else{
			TCompHfxZone* comp_hfx_ent = (TCompHfxZone*)comp_hfx;
			if (comp_hfx_ent->getType()&TCompHfxZone::type::REVERB){
				setReverbHFX(comp_hfx, stream.first.stream);
				zone = true;
			} 
			if (comp_hfx_ent->getType()&TCompHfxZone::type::ECHO){
				setEchoHFX(comp_hfx, stream.first.stream);
				zone = true;
			}
		}
	}
	BASS_ChannelPlay(stream.first.stream, 0);

	if (zone){
		stream.first.FX_zone = comp_hfx;
		stream_effects stream_FX;
		stream_FX.stream = stream.first.stream;
		stream_FX.FX_zone = comp_hfx;
		std::pair<stream_effects, HSAMPLE> val = std::make_pair(stream_FX, stream.second);
		sounds->operator[](name) = val;
	}
	
}

void CSoundManager::play3DFX(std::string name, XMVECTOR pos){
	sounds_map stream = sounds->operator[](name);
	HCHANNEL channel = BASS_SampleGetChannel(stream.first.stream, FALSE);
	BASS_ChannelFlags(channel, BASS_STREAM_AUTOFREE, BASS_STREAM_AUTOFREE);
	BASS_3DVECTOR pos_ref;
	BASS_3DVECTOR front_ref;
	pos_ref.x = XMVectorGetX(pos);
	pos_ref.y = XMVectorGetY(pos);
	pos_ref.z = XMVectorGetZ(pos);
	BASS_3DVECTOR* position;
	position = &pos_ref;
	BASS_ChannelSet3DPosition(channel, position, NULL, NULL);
	BASS_Apply3D();

	bool zone = false;
	CHandle comp_hfx = CLogicManager::get().soundsInsideHFXZone(pos);
	if (comp_hfx.isValid()){
		if (stream.first.FX_zone == comp_hfx){
			stream.first.FX_zone = comp_hfx;
		}
		else{
			TCompHfxZone* comp_hfx_ent = (TCompHfxZone*)comp_hfx;
			if (comp_hfx_ent->getType()&TCompHfxZone::type::REVERB){
				setReverbHFX(comp_hfx, stream.first.stream);
				zone = true;
			}
			if (comp_hfx_ent->getType()&TCompHfxZone::type::ECHO){
				setEchoHFX(comp_hfx, stream.first.stream);
				zone = true;
			}
		}
	}
	BASS_ChannelPlay(stream.first.stream, 0);

	if (zone){
		stream.first.FX_zone = comp_hfx;
		stream_effects stream_FX;
		stream_FX.stream = stream.first.stream;
		stream_FX.FX_zone = comp_hfx;
		std::pair<stream_effects, HSAMPLE> val = std::make_pair(stream_FX, stream.second);
		sounds->operator[](name) = val;
	}


	BASS_ChannelPlay(channel, 0);
}

void CSoundManager::setSound3DFactors(float distance, float roll, float doppler){
	BASS_Set3DFactors(distance, roll, doppler);
}

void CSoundManager::playImpactFX(float force, CHandle transform){
	TCompTransform* t_transform = (TCompTransform*)transform;
	if ((force > 10) && (force <= 5000)){
		CSoundManager::get().play3DFX("hit_wood_light", (TTransform*)t_transform, force);
	}else if ((force > 5000) && (force <= 8000)){
		CSoundManager::get().play3DFX("hit_wood_medium", (TTransform*)t_transform, force);
	}else if (force > 8000){
		CSoundManager::get().play3DFX("hit_wood_heavy", (TTransform*)t_transform, force);
	}
}

void CSoundManager::setEchoHFX(CHandle comp_hfx, HSTREAM channel){
	if (comp_hfx.isValid()){
		if (((TCompHfxZone*)comp_hfx)->getType() & TCompHfxZone::type::ECHO){
			HFX FX = BASS_ChannelSetFX(channel, BASS_FX_DX8_ECHO, 0);
			if (FX != 0){
				BASS_DX8_ECHO* e = ((TCompHfxZone*)comp_hfx)->getEcho();
				if (e != nullptr){
					BASS_DX8_ECHO echo;
					echo.fFeedback = e->fFeedback;
					echo.fLeftDelay = e->fLeftDelay;
					echo.fRightDelay = e->fRightDelay;
					echo.fWetDryMix = e->fWetDryMix;
					echo.lPanDelay = e->lPanDelay;
					bool success = BASS_FXSetParameters(FX, &echo);
					if (!success){
						int code = BASS_ErrorGetCode();
						if (code == BASS_ERROR_HANDLE){
							XASSERT(code, "Error, FX handle invalid");
						}
						else if (code == BASS_ERROR_ILLPARAM){
							XASSERT(code, "Error, FX params invalid");
						}
						else if (code == BASS_ERROR_UNKNOWN){
							XASSERT(code, "Error, FX Unknown error");
						}
					}
				}
			}
			else{
				int code = BASS_ErrorGetCode();
				if (code == BASS_ERROR_HANDLE){
					XASSERT(code, "Error, FX handle invalid");
				}
				else if (code == BASS_ERROR_ILLPARAM){
					XASSERT(code, "Error, FX params invalid");
				}
				else if (code == BASS_ERROR_UNKNOWN){
					XASSERT(code, "Error, FX Unknown error");
				}
			}
		}
	}
}

void CSoundManager::setReverbHFX(CHandle comp_hfx, HSTREAM channel){
	if (comp_hfx.isValid()){
		if (((TCompHfxZone*)comp_hfx)->getType() & TCompHfxZone::type::REVERB){
			HFX FX = BASS_ChannelSetFX(channel, BASS_FX_DX8_REVERB, 9);
			if (FX != 0){
				BASS_DX8_REVERB* r = ((TCompHfxZone*)comp_hfx)->getReverb();
				if (r != nullptr){
					//HFX reverb=((TCompHfxZone*)comp_hfx)->getHFXZoneAtributtes();
					//bool success = BASS_FXGetParameters(((TCompHfxZone*)comp_hfx)->getHFXZoneAtributtes(), r);
					//if (success){
					BASS_DX8_REVERB reverb;
					reverb.fInGain = r->fInGain;
					reverb.fReverbMix = r->fReverbMix;
					reverb.fReverbTime = r->fReverbTime;
					reverb.fHighFreqRTRatio = r->fHighFreqRTRatio;
					bool success = BASS_FXSetParameters(FX, &reverb);
					//}
					if (!success){
						int code = BASS_ErrorGetCode();
						if (code == BASS_ERROR_HANDLE){
							XASSERT(code, "Error, FX handle invalid");
						}
						else if (code == BASS_ERROR_ILLPARAM){
							XASSERT(code, "Error, FX params invalid");
						}
						else if (code == BASS_ERROR_UNKNOWN){
							XASSERT(code, "Error, FX Unknown error");
						}
					}
				}
			}else{
				int code = BASS_ErrorGetCode();
				if (code == BASS_ERROR_HANDLE){
					XASSERT(code, "Error, FX handle invalid");
				}
				else if (code == BASS_ERROR_ILLPARAM){
					XASSERT(code, "Error, FX params invalid");
				}
				else if (code == BASS_ERROR_UNKNOWN){
					XASSERT(code, "Error, FX Unknown error");
				}
			}
		}
	}
}


