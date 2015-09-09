#include "mcv_platform.h"
#include "comp_trigger.h"
#include "entity_manager.h"
#include "comp_audio_listener.h"
#include "ai\logic_manager.h"
#include "comp_transform.h"
#include "audio\sound_manager.h"

TCompAudioListener::~TCompAudioListener() {
}

void TCompAudioListener::loadFromAtts(const std::string& elem, MKeyValue &atts) {
	h_transform = assertRequiredComponent<TCompTransform>(this);
}

void TCompAudioListener::init() {
}

void TCompAudioListener::update(float elapsed){

	TCompTransform* transform = h_transform;
	CSoundManager::get().setListenerTransform(*transform);
}