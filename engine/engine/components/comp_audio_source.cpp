#include "mcv_platform.h"
#include "comp_trigger.h"
#include "entity_manager.h"
#include "../audio/sound_manager.h"
#include "comp_audio_source.h"
#include "comp_transform.h"
#include "comp_hfx_zone.h"
#include "ai\logic_manager.h"

TCompAudioSource::TCompAudioSource() {
	m_mode = BASS_SAMPLE_3D;
	m_min= 3.5f;
	m_max= 6.f;
	autoPlaySound = false;
	played = false;
	loop = false;
}

TCompAudioSource::~TCompAudioSource() {
	//CLogicManager::get().unregisterTrigger(CHandle(this));	
}

void TCompAudioSource::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	std::string sound_name=atts.getString("sound", "undefined");
	std::string sound_name_copy = sound_name;
	const char* sound_char = sound_name_copy.c_str();
	std::size_t found = sound_name.find('.');
	if (found != std::string::npos){
		sound_name.replace(sound_name.find('.'), sound_name.length(), "");
		if (sound_name != "undefined"){
			CSoundManager::get().addFXTrack(sound_char, sound_name);
			asociated_sound.init_sound(sound_name);
			loop = atts.getBool("loop", false);
			distance_max = atts.getFloat("distance", 5.5f);
			volume = atts.getFloat("volume", 0.5f);
			//asociated_sound.setSoundVolume(volume);
			asociated_sound.loop = loop;
			autoPlaySound = true;
		}
	}
}

void TCompAudioSource::init() {
	player = CEntityManager::get().getByName("Player");
	player_transform = ((CEntity*)player)->get<TCompTransform>();
	asociated_sound.slow = false;
	asociated_sound.FX_zone = 0;
	asociated_sound.slow_effect = 0;
	asociated_sound.under_water = false;
	asociated_sound.under_water_effect = 0;
}

void TCompAudioSource::update(float elapsed){
	//Hacer metodo que convierta XMVECTOR en BASS_3DVECTOR
	BASS_3DVECTOR pos_ref;
	BASS_3DVECTOR front_ref;
	TCompTransform* transform = getSibling<TCompTransform>(this);
	pos_ref.x = XMVectorGetX(transform->position);
	pos_ref.y = XMVectorGetY(transform->position);
	pos_ref.z = XMVectorGetZ(transform->position);
	front_ref.x = XMVectorGetX(transform->position+transform->getFront());
	front_ref.y = XMVectorGetY(transform->position + transform->getFront());
	front_ref.z = XMVectorGetZ(transform->position + transform->getFront());
	pos = &pos_ref;
	front = &front_ref;
	asociated_sound.setSoundPosition(pos, front, NULL);
	BASS_Apply3D();
	//HFX ------------------------------------------------------------------
	CHandle comp_hfx = CLogicManager::get().soundsInsideHFXZone(transform->position);

	if (CSoundManager::get().getSlowMotion()){
		if (!asociated_sound.slow){
			activateSlowMoFilter(asociated_sound.own_stream);
		}
	}
	else{
		desactivateSlowMoFilter(asociated_sound.own_stream);
	}

	if ((CSoundManager::get().getUnderWater())){
		if (!asociated_sound.under_water){
			activateLowPassFilter(asociated_sound.own_stream);
		}
	}
	else{
		desactivateLowPassFilter(asociated_sound.own_stream);
	}

	bool zone = false;
	if (own_transform.isValid()){
		CHandle hfx_player = CLogicManager::get().soundsInsideHFXZone(((TCompTransform*)own_transform)->position);
		if (comp_hfx.isValid()){
			TCompHfxZone* comp_hfx = (TCompHfxZone*)hfx_player;
			if (!asociated_sound.FX_zone.isValid()){
				if (comp_hfx->getType()&TCompHfxZone::type::REVERB){
					setReverbHFX(comp_hfx, asociated_sound.own_stream);
					zone = true;
				}
				if (comp_hfx->getType()&TCompHfxZone::type::ECHO){
					setEchoHFX(comp_hfx, asociated_sound.own_stream);
					zone = true;
				}
				if (comp_hfx->getType()&TCompHfxZone::type::FREE_REVERB){
					setFreeReverbHFX(comp_hfx, asociated_sound.own_stream);
					zone = true;
				}
			}
		}
	}

	//----------------------------------------------------------------------
	if (autoPlaySound){
		player_transform = ((CEntity*)player)->get<TCompTransform>();
		TCompTransform* p_transform = (TCompTransform*)player_transform;
		TCompTransform* m_transform = (TCompTransform*)own_transform;
		float distance = V3DISTANCE(transform->position, p_transform->position);
		if (!played){
			//if (distance < distance_max){
				asociated_sound.playSound();
				played = true;
			//}
		}
		/*else{
			if (loop){
				if (distance >= distance_max){
					asociated_sound.stopSound();
				}
			}
		}*/
	}

	if (zone){
		asociated_sound.FX_zone = comp_hfx;
	}

}

void TCompAudioSource::activateSlowMoFilter(HSTREAM channel){
	HFX FX = BASS_ChannelSetFX(channel, BASS_FX_BFX_PITCHSHIFT, 9);
	if (FX != 0){
		BASS_BFX_PITCHSHIFT pitchshift;
		pitchshift.fPitchShift = CSoundManager::get().f_PitchShift;
		pitchshift.fSemitones = CSoundManager::get().f_Semitones;
		pitchshift.lChannel = BASS_BFX_CHAN1;
		pitchshift.lFFTsize = CSoundManager::get().l_FFTsize;
		pitchshift.lOsamp = CSoundManager::get().l_Osamp;
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
			asociated_sound.slow = true;
			asociated_sound.slow_effect = FX;
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
void TCompAudioSource::desactivateSlowMoFilter(HSTREAM channel){

	if (asociated_sound.slow){
		BOOL success = BASS_ChannelRemoveFX(channel, asociated_sound.slow_effect);
		if (success){
			asociated_sound.slow_effect = 0;
			asociated_sound.slow = false;
		}
	}
}

void TCompAudioSource::activateLowPassFilter(HSTREAM channel){
	HFX FX = BASS_ChannelSetFX(channel, BASS_FX_BFX_BQF, 9);
	if (FX != 0){
		BASS_BFX_BQF lowpass;
		lowpass.fBandwidth = CSoundManager::get().f_Bandwidth;
		lowpass.fCenter = CSoundManager::get().f_Center;
		lowpass.fGain = CSoundManager::get().f_Gain;
		lowpass.fQ = CSoundManager::get().f_Q;
		lowpass.fS = CSoundManager::get().f_S;
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
			asociated_sound.under_water = true;
			asociated_sound.under_water_effect = FX;
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

void TCompAudioSource::desactivateLowPassFilter(HSTREAM channel){

	if (asociated_sound.under_water){
		BOOL success = BASS_ChannelRemoveFX(channel, asociated_sound.under_water_effect);
		if (success){
			asociated_sound.under_water_effect = 0;
			asociated_sound.under_water = false;
		}
	}
}


void TCompAudioSource::setEchoHFX(CHandle comp_hfx, HSTREAM channel){
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

void TCompAudioSource::setReverbHFX(CHandle comp_hfx, HSTREAM channel){
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

void TCompAudioSource::setFreeReverbHFX(CHandle comp_hfx, HSTREAM channel){
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

void TCompAudioSource::setSoundAsociated(std::string name, DWORD mode, float min, float max){
	asociated_sound.init_sound(name, mode, min, max);
}

void TCompAudioSource::setSoundAsociated(std::string name){
	asociated_sound.init_sound(name);
}

void TCompAudioSource::set3DAttributes(DWORD mode, float min, float max){
	m_mode = mode;
	m_min = min;
	m_max = max;
}

