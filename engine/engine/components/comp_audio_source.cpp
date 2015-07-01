#include "mcv_platform.h"
#include "comp_trigger.h"
#include "entity_manager.h"
#include "../audio/sound_manager.h"
#include "comp_audio_source.h"
#include "comp_transform.h"
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
			asociated_sound.setLoop(loop);
			autoPlaySound = true;
		}
	}
	
}

void TCompAudioSource::init() {

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
	if (autoPlaySound)
		if (!played){
			asociated_sound.playSound();
			played = true;
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

