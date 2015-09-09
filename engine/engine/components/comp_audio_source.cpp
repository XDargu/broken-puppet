#include "mcv_platform.h"
#include "comp_trigger.h"
#include "entity_manager.h"
#include "../audio/sound_manager.h"
#include "comp_audio_source.h"
#include "comp_transform.h"
#include "comp_hfx_zone.h"
#include "comp_name.h"
#include "ai\logic_manager.h"

TCompAudioSource::TCompAudioSource() {
	played = false;
	autoPlay = false;
}

TCompAudioSource::~TCompAudioSource() {
	//CLogicManager::get().unregisterTrigger(CHandle(this));
	CSoundManager::get().ERRCHECK(asociated_sound->stop(FMOD_STUDIO_STOP_IMMEDIATE));
	CSoundManager::get().ERRCHECK(asociated_sound->release());
	CLogicManager::get().unregisterAudioSource(this);
}

void TCompAudioSource::loadFromAtts(const std::string& elem, MKeyValue &atts) {

	if (elem == "audioSource"){
		name = assertRequiredComponent<TCompName>(this);
		autoPlay = atts.getBool("autoplay", "false");
	}

	if (elem == "event"){
		std::string sound_name = atts.getString("sound", "undefined");
		own_transform = assertRequiredComponent<TCompTransform>(this);

		//asociated_sound = CSoundManager::get().getInstance(sound_name);
		setInstance(sound_name);
	}

	if (elem == "param") {
		params_names.push_back(atts.getString("param_name", "undefined"));
		params_value.push_back(atts.getFloat("param_value", 0.f));
	}

	if (elem=="fin_param"){
		if (!params_value.empty()){
			const int size = sizeof(params_value.size()) / sizeof(float);
			CSoundManager::SoundParameter params[size];

			for (int j = 0; j < size; ++j){
				params[j].name = params_names[j];
				params[j].value = params_value[j];
			}

			bool success = CSoundManager::get().setInstanceParams(asociated_sound, params, size);
			XASSERT(success, "Error: invalid params in audio source");
		}
		CLogicManager::get().registerAudioSource(CHandle(this));
	}
}

void TCompAudioSource::init() {
	player = CEntityManager::get().getByName("Player");
	player_transform = ((CEntity*)player)->get<TCompTransform>();
	if (autoPlay)
		play();
}

void TCompAudioSource::update(float elapsed){

	if (asociated_sound != nullptr){
		TCompTransform* m_trans = own_transform;
		bool success = CSoundManager::get().setInstancePos(asociated_sound, *m_trans);
		if (success){

		}
	}
}

void TCompAudioSource::play(){
	CSoundManager::get().ERRCHECK(asociated_sound->start());
}

CHandle TCompAudioSource::getName(){
	return name;
}

void TCompAudioSource::setInstance(std::string event_desc_name){
	if (asociated_sound != nullptr){
		CSoundManager::get().ERRCHECK(asociated_sound->stop(FMOD_STUDIO_STOP_IMMEDIATE));
		CSoundManager::get().ERRCHECK(asociated_sound->release());
	}
	asociated_sound = CSoundManager::get().getInstance(event_desc_name);
	XASSERT(asociated_sound != nullptr, "Error: invalid event in audio source");
}


