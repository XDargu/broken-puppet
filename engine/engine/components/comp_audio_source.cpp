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
	std::string sound_name = atts.getString("sound", "undefined");
	own_transform = assertRequiredComponent<TCompTransform>(this);

	TCompTransform* m_trans = own_transform;

	CSoundManager::get().playEvent(sound_name, m_trans->position);

	/*
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
			asociated_sound.setSoundVolume(volume);
			asociated_sound.setLoop(loop);
			autoPlaySound = true/
		}
	}*/
}

void TCompAudioSource::init() {
	player = CEntityManager::get().getByName("Player");
	player_transform = ((CEntity*)player)->get<TCompTransform>();
}

void TCompAudioSource::update(float elapsed){
	//Hacer metodo que convierta XMVECTOR en BASS_3DVECTOR
	/*BASS_3DVECTOR pos_ref;
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
	setHFX(comp_hfx);
	//----------------------------------------------------------------------
	if (autoPlaySound){
		player_transform = ((CEntity*)player)->get<TCompTransform>();
		TCompTransform* p_transform = (TCompTransform*)player_transform;
		TCompTransform* m_transform = (TCompTransform*)own_transform;
		float distance = V3DISTANCE(transform->position, p_transform->position);
		if (!played){
			if (distance < distance_max){
				asociated_sound.playSound();
				played = true;
			}
		}
		else{
			if (loop){
				if (distance >= distance_max){
					asociated_sound.stopSound();
				}
			}
		}
	}*/
}

void TCompAudioSource::setHFX(CHandle comp_hfx){
	/*if (comp_hfx.isValid()){
		if (((TCompHfxZone*)comp_hfx)->getType() & TCompHfxZone::type::ECHO){
			int prueba = BASS_ChannelSetFX(asociated_sound.getChannel(), BASS_FX_DX8_ECHO, 0);
			BASS_DX8_ECHO* e = ((TCompHfxZone*)comp_hfx)->getEcho();
			if (e != nullptr){
				bool success = BASS_FXGetParameters(((TCompHfxZone*)comp_hfx)->getHFXZoneAtributtes(), e);
				if (success){
					BASS_FXSetParameters(prueba, e);
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
		}else if (((TCompHfxZone*)comp_hfx)->getType() & TCompHfxZone::type::REVERB){
			int prueba = BASS_ChannelSetFX(asociated_sound.getChannel(), BASS_FX_DX8_REVERB, 0);
			BASS_DX8_REVERB* r = ((TCompHfxZone*)comp_hfx)->getReverb();
			if (r != nullptr){
				bool success = BASS_FXGetParameters(((TCompHfxZone*)comp_hfx)->getHFXZoneAtributtes(), r);
				if (success){
					BASS_FXSetParameters(prueba, r);
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
	}*/
}

void TCompAudioSource::setSoundAsociated(std::string name, DWORD mode, float min, float max){
	//asociated_sound.init_sound(name, mode, min, max);
}

void TCompAudioSource::setSoundAsociated(std::string name){
	//asociated_sound.init_sound(name);
}

void TCompAudioSource::set3DAttributes(DWORD mode, float min, float max){
	m_mode = mode;
	m_min = min;
	m_max = max;
}

