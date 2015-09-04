#include "mcv_platform.h"
#include "sound_manager.h"
#include "components\comp_hfx_zone.h"
#include "ai\logic_manager.h"

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
	XASSERT(HIWORD(BASS_FX_GetVersion()) == BASSVERSION, "Error versión Bass_FX");

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

void CSoundManager::init(){
	first = false;
	slowed = false;
	under_water = false;
	currentTrack = 0;
	f_Bandwidth = 0.f;
	f_Center = 700.f;
	f_Gain = 0.f;
	f_Q = 2.f;
	f_S = 0;
	f_PitchShift = 0.5;
	f_Semitones = 0.5;
	l_FFTsize = 8192;
	l_Osamp = 24;
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
	HSAMPLE h_sample = BASS_SampleLoad(0, filename.c_str(), 0, 0, 200, BASS_SAMPLE_3D | BASS_SAMPLE_MUTEMAX);
	HSTREAM h_stream = BASS_StreamCreateFile(FALSE, filename.c_str(), 0, 0, BASS_SAMPLE_3D);
	stream_effects stream_FX;
	stream_FX.stream = h_stream;
	stream_FX.FX_zone = CHandle();
	stream_FX.slow = false;
	stream_FX.slow_effect = 0;
	stream_FX.under_water = false;
	stream_FX.under_water_effect = 0;
	sounds_map val = std::make_pair(stream_FX, h_sample);
	sounds->operator[](name) = val;
	//setSound3DAttributes(stream_FX.stream);
}

void CSoundManager::addFXTrack(const char* file, std::string name, std::string category_name){
	std::string filename = ("data/sounds/" + std::string(file));
	HSAMPLE h_sample = BASS_SampleLoad(0, filename.c_str(), 0, 0, 200, BASS_SAMPLE_3D | BASS_SAMPLE_MUTEMAX);
	HSTREAM h_stream = BASS_StreamCreateFile(FALSE, filename.c_str(), 0, 0, BASS_SAMPLE_3D);
	stream_effects stream_FX;
	stream_FX.stream = h_stream;
	stream_FX.FX_zone = CHandle();
	stream_FX.slow = false;
	stream_FX.slow_effect = 0;
	stream_FX.under_water = false;
	stream_FX.under_water_effect = 0;
	sounds_map val = std::make_pair(stream_FX, h_sample);
	sounds->operator[](name) = val;
	sounds_categories->operator[](category_name).push_back(val);
	//setSound3DAttributes(stream_FX.stream);
}

void CSoundManager::addFX2DTrack(const char* file, std::string name){
	std::string filename = ("data/sounds/" + std::string(file));
	HSAMPLE h_sample = BASS_SampleLoad(0, filename.c_str(), 0, 0, 200, BASS_SAMPLE_MONO);
	HSTREAM h_stream = BASS_StreamCreateFile(FALSE, filename.c_str(), 0, 0, BASS_SAMPLE_MONO);
	stream_effects stream_FX;
	stream_FX.stream = h_stream;
	stream_FX.FX_zone = CHandle();
	stream_FX.slow = false;
	stream_FX.slow_effect = 0;
	stream_FX.under_water = false;
	stream_FX.under_water_effect = 0;
	sounds_map val = std::make_pair(stream_FX, h_sample);
	sounds->operator[](name) = val;
}

void CSoundManager::addFX2DTrack(const char* file, std::string name, std::string category_name){
	std::string filename = ("data/sounds/" + std::string(file));
	HSAMPLE h_sample = BASS_SampleLoad(0, filename.c_str(), 0, 0, 200, BASS_SAMPLE_MONO);
	HSTREAM h_stream = BASS_StreamCreateFile(FALSE, filename.c_str(), 0, 0, BASS_SAMPLE_MONO);
	stream_effects stream_FX;
	stream_FX.stream = h_stream;
	stream_FX.FX_zone = CHandle();
	stream_FX.slow = false;
	stream_FX.slow_effect = 0;
	stream_FX.under_water = false;
	stream_FX.under_water_effect = 0;
	sounds_map val = std::make_pair(stream_FX, h_sample);
	sounds->operator[](name) = val;
	sounds_categories->operator[](category_name).push_back(val);
}

void CSoundManager::playRandomFX(std::string category_name){
	if (sounds_categories->operator[](category_name).size() > 0){
		int random_ind = getRandomNumber(0, sounds_categories->operator[](category_name).size());
		sounds_map sound_effects = sounds_categories->operator[](category_name)[random_ind];
		//HCHANNEL channel = BASS_SampleGetChannel(stream.second, FALSE);
		BASS_ChannelSetAttribute(sound_effects.first.stream, BASS_ATTRIB_VOL, 0.3f);
		//BASS_ChannelFlags(stream.first.stream, BASS_STREAM_AUTOFREE, BASS_STREAM_AUTOFREE);

		if (slowed){
			if (!sound_effects.first.slow){
				activateSlowMoFilter(sound_effects.first.stream, category_name, random_ind);
			}
		}else{
			desactivateSlowMoFilter(sound_effects.first.stream, category_name, random_ind);
		}

		if (under_water){
			if (!sound_effects.first.under_water){
				activateLowPassFilter(sound_effects.first.stream, category_name, random_ind);
			}
		}else{
			desactivateLowPassFilter(sound_effects.first.stream, category_name, random_ind);
		}

		CHandle hfx_player = CLogicManager::get().playerInsideHFXZone();
		bool zone = false;
		if (hfx_player.isValid()){
			TCompHfxZone* comp_hfx = (TCompHfxZone*)hfx_player;
			if (!sound_effects.first.FX_zone.isValid()){
				if (comp_hfx->getType()&TCompHfxZone::type::REVERB){
					setReverbHFX(hfx_player, sound_effects.first.stream);
					zone = true;
				}
				if (comp_hfx->getType()&TCompHfxZone::type::ECHO){
					setEchoHFX(hfx_player, sound_effects.first.stream);
					zone = true;
				}
				if (comp_hfx->getType()&TCompHfxZone::type::FREE_REVERB){
					setFreeReverbHFX(hfx_player, sound_effects.first.stream);
					zone = true;
				}
			}
		}

		BASS_ChannelPlay(sound_effects.first.stream, 0);

		if (zone){
			sounds_categories->operator[](category_name)[random_ind].first.FX_zone = hfx_player;
		}
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
	sounds_map sounds_effect = sounds->operator[](name);
	//HCHANNEL channel = BASS_SampleGetChannel(stream.second, FALSE);
	//BASS_ChannelSetAttribute(sounds_effect.first.stream, BASS_ATTRIB_VOL, 1.f);
	//BASS_ChannelFlags(stream.first.stream, BASS_STREAM_AUTOFREE, BASS_STREAM_AUTOFREE);

	if (slowed){
		if (!sounds_effect.first.slow){
			activateSlowMoFilter(sounds_effect.first.stream, name);
		}
	}
	else{
		desactivateSlowMoFilter(sounds_effect.first.stream, name);
	}

	if (under_water){
		if (!sounds_effect.first.under_water){
			activateLowPassFilter(sounds_effect.first.stream, name);
		}
	}
	else{
		desactivateLowPassFilter(sounds_effect.first.stream, name);
	}

	CHandle hfx_player = CLogicManager::get().playerInsideHFXZone();
	bool zone = false;
	if (hfx_player.isValid()){
		TCompHfxZone* comp_hfx = (TCompHfxZone*)hfx_player;
		if (!sounds_effect.first.FX_zone.isValid()){
			if (comp_hfx->getType()&TCompHfxZone::type::REVERB){
				setReverbHFX(hfx_player, sounds_effect.first.stream);
				zone = true;
			}
			if (comp_hfx->getType()&TCompHfxZone::type::ECHO){
				setEchoHFX(hfx_player, sounds_effect.first.stream);
				zone = true;
			}
			if (comp_hfx->getType()&TCompHfxZone::type::FREE_REVERB){
				setFreeReverbHFX(hfx_player, sounds_effect.first.stream);
				zone = true;
			}
		}
	}
	BASS_Apply3D();
	BOOL success = BASS_ChannelPlay(sounds_effect.first.stream, 0);
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

	if (zone){
		sounds->operator[](name).first.FX_zone = hfx_player;
	}
}

void CSoundManager::playFX(std::string name, bool loop){
	// Play FX
	sounds_map sounds_effect = sounds->operator[](name);
	//HCHANNEL channel = BASS_SampleGetChannel(stream.second, FALSE);

	if (loop){
		//BASS_ChannelSetAttribute(sounds_effect.first.stream, BASS_ATTRIB_VOL, 1.f);
		
		BASS_ChannelFlags(sounds_effect.first.stream, BASS_SAMPLE_3D | BASS_SAMPLE_LOOP, BASS_SAMPLE_3D | BASS_SAMPLE_LOOP);
	}//else
		//BASS_ChannelFlags(stream.stream, BASS_sample_, BASS_STREAM_AUTOFREE);

	if (slowed){
		if (!sounds_effect.first.slow){
			activateSlowMoFilter(sounds_effect.first.stream, name);
		}
	}
	else{
		desactivateSlowMoFilter(sounds_effect.first.stream, name);
	}

	if (under_water){
		if (!sounds_effect.first.under_water){
			activateLowPassFilter(sounds_effect.first.stream, name);
		}
	}
	else{
		desactivateLowPassFilter(sounds_effect.first.stream, name);
	}

	CHandle hfx_player = CLogicManager::get().playerInsideHFXZone();
	bool zone = false;
	if (hfx_player.isValid()){
		TCompHfxZone* comp_hfx = (TCompHfxZone*)hfx_player;
		if (!sounds_effect.first.FX_zone.isValid()){
			if (comp_hfx->getType()&TCompHfxZone::type::REVERB){
				setReverbHFX(hfx_player, sounds_effect.first.stream);
				zone = true;
			}
			if (comp_hfx->getType()&TCompHfxZone::type::ECHO){
				setEchoHFX(hfx_player, sounds_effect.first.stream);
				zone = true;
			}
			if (comp_hfx->getType()&TCompHfxZone::type::FREE_REVERB){
				setFreeReverbHFX(hfx_player, sounds_effect.first.stream);
				zone = true;
			}
		}
	}

	BASS_ChannelPlay(sounds_effect.first.stream, 0);

	if (zone){
		sounds->operator[](name).first.FX_zone = hfx_player;
	}
}

void CSoundManager::stopFX(std::string name){
	// Play FX
	sounds_map sounds_effect = sounds->operator[](name);
	//HCHANNEL channel = BASS_SampleGetChannel(stream.second, FALSE);
	BASS_ChannelStop(sounds_effect.first.stream);
}

void CSoundManager::play3DFX(std::string name, TTransform* trans, float volume_lambda){
	sounds_map sounds_effect = sounds->operator[](name);
	HCHANNEL channel = BASS_SampleGetChannel(sounds_effect.second, FALSE);
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
	BASS_ChannelSet3DPosition(sounds_effect.first.stream, pos, front, NULL);
	//float volume;
	//BASS_ChannelGetAttribute(sounds_effect.first.stream, BASS_ATTRIB_VOL, &volume);
	//float final_volume = volume + volume_lambda / volume_factor;
	//BASS_ChannelSetAttribute(sounds_effect.first.stream, BASS_ATTRIB_VOL, final_volume);

	if (slowed){
		if (!sounds_effect.first.slow){
			activateSlowMoFilter(sounds_effect.first.stream, name);
		}
	}else{
		desactivateSlowMoFilter(sounds_effect.first.stream, name);
	}

	if (under_water){
		if (!sounds_effect.first.under_water){
			activateLowPassFilter(sounds_effect.first.stream, name);
		}
	}
	else{
		desactivateLowPassFilter(sounds_effect.first.stream, name);
	}

	bool zone = false;
	CHandle comp_hfx = CLogicManager::get().soundsInsideHFXZone(trans->position);
	if (comp_hfx.isValid()){
		if (sounds_effect.first.FX_zone == comp_hfx){
			sounds_effect.first.FX_zone = comp_hfx;
		}
		else{
			TCompHfxZone* comp_hfx_ent = (TCompHfxZone*)comp_hfx;
			if (comp_hfx_ent->getType()&TCompHfxZone::type::REVERB){
				setReverbHFX(comp_hfx, sounds_effect.first.stream);
				zone = true;
			} 
			if (comp_hfx_ent->getType()&TCompHfxZone::type::ECHO){
				setEchoHFX(comp_hfx, sounds_effect.first.stream);
				zone = true;
			}
			if (comp_hfx_ent->getType()&TCompHfxZone::type::FREE_REVERB){
				setFreeReverbHFX(comp_hfx, sounds_effect.first.stream);
				zone = true;
			}
		}
	}
	BASS_Apply3D();
	BASS_ChannelPlay(sounds_effect.first.stream, 0);

	if (zone){
		sounds->operator[](name).first.FX_zone = comp_hfx;
	}
	
}

void CSoundManager::play3DFX(std::string name, XMVECTOR pos){
	sounds_map sounds_effect = sounds->operator[](name);
	//HCHANNEL channel = BASS_SampleGetChannel(stream.second, FALSE);
	//BASS_ChannelFlags(stream.stream, BASS_STREAM_AUTOFREE, BASS_STREAM_AUTOFREE);
	BASS_3DVECTOR pos_ref;
	BASS_3DVECTOR front_ref;
	pos_ref.x = XMVectorGetX(pos);
	pos_ref.y = XMVectorGetY(pos);
	pos_ref.z = XMVectorGetZ(pos);
	BASS_3DVECTOR* position;
	position = &pos_ref;
	BASS_ChannelSet3DPosition(sounds_effect.first.stream, position, NULL, NULL);
	BASS_Apply3D();

	if (slowed){
		if (!sounds_effect.first.slow){
			activateSlowMoFilter(sounds_effect.first.stream, name);
		}
	}
	else{
		desactivateSlowMoFilter(sounds_effect.first.stream, name);
	}

	if (under_water){
		if (!sounds_effect.first.under_water){
			activateLowPassFilter(sounds_effect.first.stream, name);
		}
	}
	else{
		desactivateLowPassFilter(sounds_effect.first.stream, name);
	}

	bool zone = false;
	CHandle comp_hfx = CLogicManager::get().soundsInsideHFXZone(pos);
	if (comp_hfx.isValid()){
		if (sounds_effect.first.FX_zone == comp_hfx){
			sounds_effect.first.FX_zone = comp_hfx;
		}
		else{
			TCompHfxZone* comp_hfx_ent = (TCompHfxZone*)comp_hfx;
			if (comp_hfx_ent->getType()&TCompHfxZone::type::REVERB){
				setReverbHFX(comp_hfx, sounds_effect.first.stream);
				zone = true;
			}
			if (comp_hfx_ent->getType()&TCompHfxZone::type::ECHO){
				setEchoHFX(comp_hfx, sounds_effect.first.stream);
				zone = true;
			}
			if (comp_hfx_ent->getType()&TCompHfxZone::type::FREE_REVERB){
				setFreeReverbHFX(comp_hfx, sounds_effect.first.stream);
				zone = true;
			}
		}
	}
	BASS_Apply3D();
	BASS_ChannelPlay(sounds_effect.first.stream, 0);

	if (zone){
		sounds->operator[](name).first.FX_zone = comp_hfx;
	}

	BASS_Apply3D();
	BASS_ChannelPlay(sounds_effect.first.stream, 0);
}

void CSoundManager::setSound3DFactors(float distance, float roll, float doppler){
	BOOL success=BASS_Set3DFactors(distance, roll, doppler);
	if (!success){
		int code = BASS_ErrorGetCode();
		if (code == BASS_ERROR_HANDLE){
			XASSERT(code, "Error, FX handle invalid");
		}
		else if (code == BASS_ERROR_ILLPARAM){
			XASSERT(code, "Error, FX params invalid");
		}
		else if (code == BASS_ERROR_NO3D){
			XASSERT(code, "Error, FX No 3D funtionality");
		}
	}
}

void CSoundManager::setSound3DAttributes(DWORD handle){
	int mode = BASS_3DMODE_NORMAL;
	float min = 1.f;
	float max = 2.f;
	int iangle = 0;
	int oangle = 0;
	float outvol = -1.f;
	BOOL success = BASS_ChannelSet3DAttributes(handle, mode, min, max, iangle, oangle, outvol);
	if (!success){
		int code = BASS_ErrorGetCode();
		if (code == BASS_ERROR_HANDLE){
			XASSERT(code, "Error, FX handle invalid");
		}
		else if (code == BASS_ERROR_ILLPARAM){
			XASSERT(code, "Error, FX params invalid");
		}
		else if (code == BASS_ERROR_NO3D){
			XASSERT(code, "Error, FX No 3D funtionality");
		}
	}

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

void CSoundManager::setFreeReverbHFX(CHandle comp_hfx, HSTREAM channel){
	if (comp_hfx.isValid()){
		if (((TCompHfxZone*)comp_hfx)->getType() & TCompHfxZone::type::FREE_REVERB){
			HFX FX = BASS_ChannelSetFX(channel, BASS_FX_BFX_FREEVERB, 9);
			if (FX != 0){
				BASS_BFX_FREEVERB* r = ((TCompHfxZone*)comp_hfx)->getFreeReverb();
				if (r != nullptr){
					//HFX reverb=((TCompHfxZone*)comp_hfx)->getHFXZoneAtributtes();
					//bool success = BASS_FXGetParameters(((TCompHfxZone*)comp_hfx)->getHFXZoneAtributtes(), r);
					//if (success){
					BASS_BFX_FREEVERB free_reverb;
					free_reverb.fDryMix = r->fDryMix;
					free_reverb.fWetMix = r->fWetMix;
					free_reverb.fRoomSize = r->fRoomSize;
					free_reverb.fDamp = r->fDamp;
					free_reverb.fWidth = r->fWidth;
					free_reverb.lChannel = r->lChannel;
					free_reverb.lMode = r->lMode;
					bool success = BASS_FXSetParameters(FX, &free_reverb);
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

void CSoundManager::activateSlowMotionSounds(){
	slowed = true;
	
	//Paro la música ambiental para que no vuelva a empezar desde el princio
	if (music_tracks[currentTrack].is_loaded())
		BOOL success = BASS_ChannelPause(music_tracks[currentTrack].stream);

	/*std::map<std::string, stream_effects>::const_iterator itr;
	for (itr = sounds->begin(); itr != sounds->end(); ++itr){
		BOOL success= BASS_ChannelStop(itr->second.stream);
	}*/
	
}

void CSoundManager::desactivateSlowMotionSounds(){
	slowed = false;

	/*std::map<std::string, stream_effects>::const_iterator itr;
	for (itr = sounds->begin(); itr != sounds->end(); ++itr){
		BOOL success = BASS_ChannelStop(itr->second.stream);
	}*/

	//Paro la música ambiental para que no vuelva a empezar desde el princio
	if (music_tracks[currentTrack].is_loaded())
		BOOL success = BASS_ChannelPlay(music_tracks[currentTrack].stream, false);

}

void CSoundManager::activateLowPassSounds(){
	under_water = true;

	//Paro la música ambiental para que no vuelva a empezar desde el princio
	if (music_tracks[currentTrack].is_loaded())
		BOOL success = BASS_ChannelPause(music_tracks[currentTrack].stream);

	/*std::map<std::string, stream_effects>::const_iterator itr;
	for (itr = sounds->begin(); itr != sounds->end(); ++itr){
		BOOL success = BASS_ChannelStop(itr->second.stream);
	}*/

}

void CSoundManager::desactivateLowPassSounds(){
	under_water = false;

	/*std::map<std::string, stream_effects>::const_iterator itr;
	for (itr = sounds->begin(); itr != sounds->end(); ++itr){
		BOOL success = BASS_ChannelStop(itr->second.stream);
	}*/

	//Paro la música ambiental para que no vuelva a empezar desde el princio
	if (music_tracks[currentTrack].is_loaded())
		BOOL success = BASS_ChannelPlay(music_tracks[currentTrack].stream, false);

}

bool CSoundManager::getUnderWater(){
	return under_water;
}

bool CSoundManager::getSlowMotion(){
	return slowed;
}

void CSoundManager::activateSlowMoFilter(HSTREAM channel, std::string name){
	HFX FX = BASS_ChannelSetFX(channel, BASS_FX_BFX_PITCHSHIFT, 9);
	if (FX != 0){
		BASS_BFX_PITCHSHIFT pitchshift;
		pitchshift.fPitchShift = f_PitchShift;
		pitchshift.fSemitones = f_Semitones;
		pitchshift.lChannel = BASS_BFX_CHAN1;
		pitchshift.lFFTsize = l_FFTsize;
		pitchshift.lOsamp = l_Osamp;
		bool success = BASS_FXSetParameters(FX, &pitchshift);
	
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
		}else{
			sounds->operator[](name).first.slow = true;
			sounds->operator[](name).first.slow_effect = FX;
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

void CSoundManager::activateSlowMoFilter(HSTREAM channel, std::string category_name, int ind){
	HFX FX = BASS_ChannelSetFX(channel, BASS_FX_BFX_PITCHSHIFT, 9);
	if (FX != 0){
		BASS_BFX_PITCHSHIFT pitchshift;
		pitchshift.fPitchShift = f_PitchShift;
		pitchshift.fSemitones = f_Semitones;
		pitchshift.lChannel = BASS_BFX_CHAN1;
		pitchshift.lFFTsize = l_FFTsize;
		pitchshift.lOsamp = l_Osamp;
		bool success = BASS_FXSetParameters(FX, &pitchshift);

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
		else{
			sounds_categories->operator[](category_name)[ind].first.slow = true;
			sounds_categories->operator[](category_name)[ind].first.slow_effect = FX;
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

void CSoundManager::desactivateSlowMoFilter(HSTREAM channel, std::string name){
	
	if (sounds->operator[](name).first.slow){
		BOOL success = BASS_ChannelRemoveFX(channel, sounds->operator[](name).first.slow_effect);
		if (success){
			sounds->operator[](name).first.slow_effect = 0;
			sounds->operator[](name).first.slow = false;
		}
	}
}

void CSoundManager::desactivateSlowMoFilter(HSTREAM channel, std::string category_name, int ind){

	if (sounds_categories->operator[](category_name)[ind].first.slow){
		BOOL success = BASS_ChannelRemoveFX(channel, sounds_categories->operator[](category_name)[ind].first.slow_effect);
		if (success){
			sounds_categories->operator[](category_name)[ind].first.slow_effect = 0;
			sounds_categories->operator[](category_name)[ind].first.slow = false;
		}
	}
}

void CSoundManager::activateLowPassFilter(HSTREAM channel, std::string name){
	HFX FX = BASS_ChannelSetFX(channel, BASS_FX_BFX_BQF, 9);
	if (FX != 0){
		BASS_BFX_BQF lowpass;
		lowpass.fBandwidth = f_Bandwidth;
		lowpass.fCenter = f_Center;
		lowpass.fGain = f_Gain;
		lowpass.fQ = f_Q;
		lowpass.fS = f_S;
		lowpass.lChannel = BASS_BFX_CHANALL;
		lowpass.lFilter = BASS_BFX_BQF_LOWPASS;
		bool success = BASS_FXSetParameters(FX, &lowpass);

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
		else{
			sounds->operator[](name).first.under_water = true;
			sounds->operator[](name).first.under_water_effect = FX;
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

void CSoundManager::activateLowPassFilter(HSTREAM channel, std::string category_name, int ind){
	HFX FX = BASS_ChannelSetFX(channel, BASS_FX_BFX_BQF, 9);
	if (FX != 0){
		BASS_BFX_BQF lowpass;
		lowpass.fBandwidth = f_Bandwidth;
		lowpass.fCenter = f_Center;
		lowpass.fGain = f_Gain;
		lowpass.fQ = f_Q;
		lowpass.fS = f_S;
		lowpass.lChannel = BASS_BFX_CHANALL;
		lowpass.lFilter = BASS_BFX_BQF_LOWPASS;
		bool success = BASS_FXSetParameters(FX, &lowpass);

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
		else{
			sounds_categories->operator[](category_name)[ind].first.under_water = true;
			sounds_categories->operator[](category_name)[ind].first.under_water_effect = FX;
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

void CSoundManager::desactivateLowPassFilter(HSTREAM channel, std::string name){

	if (sounds->operator[](name).first.under_water){
		BOOL success = BASS_ChannelRemoveFX(channel, sounds->operator[](name).first.under_water_effect);
		if (success){
			sounds->operator[](name).first.under_water_effect = 0;
			sounds->operator[](name).first.under_water = false;
		}
	}
}

void CSoundManager::desactivateLowPassFilter(HSTREAM channel, std::string category_name, int ind){

	if (sounds_categories->operator[](category_name)[ind].first.under_water){
		BOOL success = BASS_ChannelRemoveFX(channel, sounds_categories->operator[](category_name)[ind].first.under_water_effect);
		if (success){
			sounds_categories->operator[](category_name)[ind].first.under_water_effect = 0;
			sounds_categories->operator[](category_name)[ind].first.under_water = false;
		}
	}
}



